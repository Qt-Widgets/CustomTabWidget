#include <QTabBar>
#include <QDragMoveEvent>
#include <QDebug>
#include <QDrag>
#include <QMap>
#include <QMimeData>
#include <QWindow>
#include <QApplication>
#include <assert.h>
#include <mainwindow.h>
#include <QTextOption>
#include <QPen>
#include <QPainter>

#include "include/tabwidget.h"
#include "include/tabbar.h"
#include "include/splitter.h"

static QString sourceIndexMimeDataKey() { return QStringLiteral("source/index"); }
static QString sourceTabTitleMimeDataKey() { return QStringLiteral("source/tabtitle"); }

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
    , mTabBar(new TabBar(this))
    , mDrawOverlay(new DrawOverlay(this))
{
    setTabBar(mTabBar);
    setAcceptDrops(true);
    setTabsClosable(true);

    //setting style for debugging purposes
	static QString style("QPushButton {"
		"   background-color: #757575;"
		"   padding-left: 4px;"
		"   padding-right: 4px;"
		"   padding-top: 2px;"
		"   padding-bottom: 2px;"
		"}");

    mDrawOverlay->raise();

	mMenuButton = new QPushButton("+", this);
	mMenuButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mMenuButton->setStyleSheet(style);
	setCornerWidget(mMenuButton);
	QObject::connect(mMenuButton, &QPushButton::clicked, this, &TabWidget::onAddNewTab);
    QObject::connect(mTabBar, &TabBar::mouseDragged, this, &TabWidget::tabDragged);
    QObject::connect(mTabBar, &TabBar::hasNoTabs, this, &TabWidget::onHasNoTabs);
	QObject::connect(mTabBar, &TabBar::tabCloseRequested, this, &TabWidget::closeTab);
}

TabWidget::~TabWidget() {
    setAcceptDrops(false);

	Splitter* splitter = dynamic_cast<Splitter*>(parentWidget());
	if (splitter) {
		splitter->tabWidgetAboutToDelete(this);
	}
}

void TabWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (mDrawOverlay->isRectHidden()) {
        return;
    }

    QPoint p = mapFromGlobal(QCursor::pos());
    if (!rect().contains(p)) {
        //Is this needed?
        //mouse not inside the widget
        return;
    }

    updateIndicatorArea(p);
    updateIndicatorRect();
    event->accept();
}

void TabWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasText()) {
        mDrawOverlay->setRect(this->rect());
        mDrawOverlay->setRectHidden(false);
        if (event->source() == this) {
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void TabWidget::dragLeaveEvent(QDragLeaveEvent* event) {
    mDrawOverlay->setRect(QRect());
    mDrawOverlay->setRectHidden(true);
    event->accept();
}

DropProperties TabWidget::createDropProperties(TabWidget* sourceTabWidget,
                                                         Splitter* targetSplitter,
                                                         Splitter* sourceSplitter) {
    //DropPropperties are used to figure out how to restore splitter sizes after drop,
    //and they are also used when inserting new splitters and widgets.
    DropProperties p;
    p.removeSourceWidget = sourceTabWidget->tabBar()->count() == 1;
    p.droppedOnSelf = sourceSplitter == targetSplitter;
    p.orientationsAreSame = sourceSplitter->orientation() == targetSplitter->orientation();
    bool sourceIsHorizontal = sourceSplitter->orientation() == Qt::Horizontal;
    p.sourceSplitterHeight = sourceIsHorizontal ? sourceSplitter->size().height() : -1;
    p.sourceSizes = p.intToPointers(sourceSplitter->sizes().toVector());
    int sourceWidgetIndex = sourceSplitter->indexOf(sourceTabWidget);
    p.dragLocation = p.sourceSizes.at(sourceWidgetIndex); //assigning of pointer

    p.targetSizes = p.intToPointers(targetSplitter->sizes().toVector());

    p.insertAfter = (mIndicatorArea == BOTTOM || mIndicatorArea == RIGHT);
    p.dropOnTabBar = mIndicatorArea == TABBAR;
    bool vertical = (targetSplitter->orientation() == Qt::Vertical);
    p.targetSplitterSize = vertical ? targetSplitter->size().height() : targetSplitter->size().width();
    bool dropVertically = (mIndicatorArea == BOTTOM || mIndicatorArea == TOP);
    p.createNewSplitter = vertical != dropVertically;
    int dropWidgetIndex = targetSplitter->indexOf(this);
    p.dropLocation = p.targetSizes.at(dropWidgetIndex); //assigning of pointer

    return p;
}

void TabWidget::dropEvent(QDropEvent *event) {
    const QMimeData *mime = event->mimeData();
    if (mime->hasText()) {
        if (mIndicatorArea == DropArea::INVALID) {
            qDebug() << "ERROR: drop area was invalid.";
            mDrawOverlay->setRectHidden(true);
            mDrawOverlay->setRect(QRect());
            event->ignore();
            return;
        }

        int sourceTabIndex = mime->data(sourceIndexMimeDataKey()).toInt();
        QString tabTitle = QString::fromStdString(mime->data(sourceTabTitleMimeDataKey()).toStdString());

        TabWidget* sourceTabWidget = static_cast<TabWidget*>(event->source());
        Q_ASSERT(sourceTabWidget);
        QWidget* sourceTab = sourceTabWidget->widget(sourceTabIndex);
		Q_ASSERT(sourceTab);
        Splitter* targetSplitter = dynamic_cast<Splitter*>(parentWidget());
        Q_ASSERT(targetSplitter);
        Splitter* sourceSplitter = dynamic_cast<Splitter*>(sourceTabWidget->parentWidget());
        Q_ASSERT(sourceSplitter);

        DropProperties p = createDropProperties(sourceTabWidget, targetSplitter, sourceSplitter);

        if (sourceTabWidget == this && sourceTabWidget->count() == 1) {
            event->acceptProposedAction();
            event->accept();
            mDrawOverlay->setRectHidden(true);
            mDrawOverlay->setRect(QRect());
            return;
        }
		
        if (mIndicatorArea == DropArea::TABBAR) {
            //dropped on tabbar
            QPoint mousePos = tabBar()->mapFromGlobal(QCursor::pos());
            int targetIndex = tabBar()->tabAt(mousePos);
            insertTab(targetIndex, sourceTab, tabTitle);
            targetIndex = (targetIndex == -1) ? tabBar()->count() -1 : targetIndex;
            setCurrentIndex(targetIndex);
        } else {
            //dropped on widget area
            Splitter* newSplitter = nullptr;
            TabWidget* newTabWidget = nullptr;
            bool vertical = (targetSplitter->orientation() == Qt::Vertical);
            int dropIndex = p.targetSizes.indexOf(p.dropLocation);
            dropIndex = p.insertAfter ? dropIndex + 1 : dropIndex;

            if (p.createNewSplitter) {
                Qt::Orientation orientation = vertical ? Qt::Horizontal : Qt::Vertical;
                int dropWidgetIndex = targetSplitter->indexOf(this);
                QList<int> targetSize = targetSplitter->sizes();
                newSplitter = targetSplitter->insertChildSplitter(dropWidgetIndex, orientation);
                newSplitter->insertWidget(0, this);
                newTabWidget = new TabWidget(newSplitter);
                newTabWidget->insertTab(0, sourceTab, tabTitle);
                newSplitter->insertWidget(dropIndex, newTabWidget);
//                p.widgetMinSize = vertical ? newTabWidget->sizeHint().height() : newTabWidget->sizeHint().width();
//                p.targetSizes.clear();
//                *p.dropLocation = vertical ? this->height() : this->width();
//                p.targetSizes.append(p.dropLocation);
                newSplitter->restoreSizesAfterDrag(Splitter::newSplitter, p);
                targetSplitter->setSizes(targetSize);
            } else {
                newTabWidget = new TabWidget(targetSplitter);
                targetSplitter->insertWidget(dropIndex, newTabWidget);
                newTabWidget->insertTab(0, sourceTab, tabTitle);
                p.widgetMinSize = vertical ? newTabWidget->sizeHint().height() : newTabWidget->sizeHint().width();
                targetSplitter->restoreSizesAfterDrag(Splitter::targetSplitter, p);
            }
        }

        event->acceptProposedAction();
        event->accept();
        mDrawOverlay->setRectHidden(true);
        mDrawOverlay->setRect(QRect());
    }
}

int TabWidget::findTargetIndex(const Splitter* targetSplitter) {
    int targetIndex = targetSplitter->indexOf(this);
    bool insertAfterTarget = (mIndicatorArea == BOTTOM || mIndicatorArea == RIGHT);
    return insertAfterTarget ? targetIndex + 1 : targetIndex;
}

void TabWidget::resizeEvent(QResizeEvent* event) {
    QTabWidget::resizeEvent(event);
    if (mDrawOverlay) {
        mDrawOverlay->setGeometry(this->rect());
    }
    event->accept();
}

void TabWidget::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_R) {
		Splitter* splitter = dynamic_cast<Splitter*>(parentWidget());
		Q_ASSERT(splitter);
		qDebug() << " ";
        qDebug().noquote() << splitter->printSplitterTree();
		qDebug() << " ";
        //event->accept();
	}
    QTabWidget::keyReleaseEvent(event);
}

void TabWidget::onAddNewTab() {
    addTab(new QWidget(this), "blaaNew");
}

void TabWidget::onHasNoTabs() {
    deleteLater();
}

void TabWidget::closeTab(int index) {
	QObject::connect(widget(index), &QWidget::destroyed, this, &TabWidget::onTabDestroyed);
	widget(index)->deleteLater();
}

void TabWidget::onTabDestroyed(QObject* object) {
	QWidget* destroyedWidget = dynamic_cast<QWidget*>(object);
	Q_ASSERT(destroyedWidget);
	QObject::disconnect(destroyedWidget, &QWidget::destroyed, this, &TabWidget::onTabDestroyed);
	int index = this->indexOf(destroyedWidget);
	if (tabBar()->count() == 1 && index != -1) {
		onHasNoTabs();
	}
}

void TabWidget::updateIndicatorArea(QPoint& p) {
    //calculate what area the mouse cursor is in.
    int top = rect().top();
    int width = rect().width();
    int height = rect().height();
    int marginX = width * 0.5;
    int marginY = height * 0.3;

    bool tabBarArea = (p.y() < tabBar()->rect().height());

    bool topArea = (top + marginY + tabBar()->rect().height() > p.y()) && !tabBarArea;
    bool bottomArea = (height - marginY < p.y()) && !tabBarArea;
    bool rightArea = (marginX < p.x()) && !tabBarArea;
    bool leftArea = (marginX > p.x()) && !tabBarArea;

    if (topArea) {
        mIndicatorArea = DropArea::TOP;
    } else if (bottomArea) {
        mIndicatorArea = DropArea::BOTTOM;
    } else if (rightArea) {
        mIndicatorArea = DropArea::RIGHT;
    } else if (leftArea) {
        mIndicatorArea = DropArea::LEFT;
    } else if (tabBarArea) {
        mIndicatorArea = DropArea::TABBAR;
    }
}

void TabWidget::updateIndicatorRect() {
    if (mIndicatorArea != DropArea::INVALID) {
        QRect rect = this->rect();
        rect.setTop(tabBar()->rect().height());
        int marginY = rect.height() * 0.4;
        int marginX = rect.width() * 0.4;

        switch (mIndicatorArea) {
        case DropArea::TOP:
            rect.setBottom(tabBar()->rect().height() + marginY);
            break;
        case DropArea::BOTTOM:
            rect.setTop(rect.bottom() - marginY);
            break;
        case DropArea::RIGHT:
            rect.setLeft(rect.right() - marginX);
            break;
        case DropArea::LEFT:
            rect.setRight(rect.left() + marginX);
            break;
        case DropArea::TABBAR: {
            QPoint mousePos = tabBar()->mapFromGlobal(QCursor::pos());
            int index = tabBar()->tabAt(mousePos);
            bool tabsUnderMouse = index != -1;

            if (tabsUnderMouse) {
                rect = tabBar()->tabRect(index);
                rect.setRight(rect.left());
            } else {
                bool hasTabs = tabBar()->count() > 0;
                if (hasTabs) {
                    index = tabBar()->count() - 1;
                    rect = tabBar()->tabRect(index);
                    rect.setLeft(rect.right());
                } else {
                    rect = tabBar()->rect();
                }
            }

            break;
        }
        case DropArea::INVALID:
            break;
        }

        mDrawOverlay->setRect(rect);
        mDrawOverlay->setRectHidden(false);
    }
}

void TabWidget::tabDragged(int index) {
	if (index == -1) {
		return;
	}
	if (!widget(index)) {
		return;
	}

	QMimeData *mimeData = new QMimeData;
	mimeData->setText(widget(index)->objectName());
	mimeData->setData(sourceTabTitleMimeDataKey(), QByteArray::fromStdString(tabText(index).toStdString()));
	mimeData->setData(sourceIndexMimeDataKey(), QByteArray::number(index));
	
	qreal dpr = window()->windowHandle()->devicePixelRatio();
    QRect tabRect = QRect(QPoint(), mTabBar->tabRect(index).size());
    QPixmap pixmap(tabRect.size() * dpr);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(dpr);
    drawDragPixmap(pixmap, tabRect, tabText(index));

//	QPixmap pixmap(tabBar()->tabRect(index).size() * dpr);
//	pixmap.setDevicePixelRatio(dpr);
//	render(&pixmap);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->exec(Qt::MoveAction);

    //clear widget after drag finished.
    if (tabBar()->count() == 0) {
        deleteLater();
    }

    //could not find any other way for doing this, but use invokeMethod, the problem is that
    //the cleanupSplitterTree must be called after all deleteLater events has been processed.
    QMetaObject::invokeMethod(parentWidget(), "cleanupSplitterTree", Qt::QueuedConnection);
}

void TabWidget::drawDragPixmap(QPixmap &pixmap, QRect tabRect, QString text) {
    QPen pen;
    pen.setColor(QColor(Qt::black));
    pen.setWidth(3);

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);

    QPainter painter(&pixmap);
    painter.setBrush(QBrush(QColor(Qt::white)));
    painter.setPen(pen);
    painter.drawRoundedRect(tabRect, 5, 5);
    painter.drawText(tabRect, text, option);
}
