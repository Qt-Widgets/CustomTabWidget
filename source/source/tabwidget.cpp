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
    , mDrawOverlay(new DrawOverlay(this))
	, mTabBar(new TabBar(this))
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

	mMenuButton = new QPushButton("+", this);
	mMenuButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	mMenuButton->setStyleSheet(style);
	setCornerWidget(mMenuButton);
	QObject::connect(mMenuButton, &QPushButton::clicked, this, &TabWidget::onAddNewTab);
    QObject::connect(mTabBar, &TabBar::mouseDragged, this, &TabWidget::tabDragged);
    QObject::connect(mTabBar, &TabBar::hasNoTabs, this, &TabWidget::onHasNoTabs);
}

TabWidget::~TabWidget() {
    setAcceptDrops(false);

	Splitter* splitter = dynamic_cast<Splitter*>(parentWidget());
	if (splitter) {
		splitter->tabWidgetAboutToDelete(this);
	}
}

void TabWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (mDrawOverlay->getRect() == QRect()) {
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
    event->accept();
}

void TabWidget::dropEvent(QDropEvent *event) {
    const QMimeData *mime = event->mimeData();
    if (mime->hasText()) {
        if (mIndicatorArea == DropArea::INVALID) {
            qDebug() << "ERROR: drop area was invalid.";
            mDrawOverlay->setRect(QRect());
            event->ignore();
            return;
        }

        Splitter::RestoreSizeProperties sizeProperties;

        sizeProperties.sourceIndex = mime->data(sourceIndexMimeDataKey()).toInt();
        QString tabTitle = QString::fromStdString(mime->data(sourceTabTitleMimeDataKey()).toStdString());
        TabWidget* sourceTabWidget = static_cast<TabWidget*>(event->source());
		Q_ASSERT(sourceTabWidget);
        sizeProperties.sourceHasOnlyOneTab = sourceTabWidget->tabBar()->count() == 1;
        QWidget* sourceTab = sourceTabWidget->widget(sizeProperties.sourceIndex);
		Q_ASSERT(sourceTab);

        if (sourceTabWidget == this && sourceTabWidget->count() == 1) {
            event->acceptProposedAction();
            event->accept();
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
            Splitter* targetSplitter = dynamic_cast<Splitter*>(parentWidget());
            Q_ASSERT(targetSplitter);

            Splitter* sourceSplitter = dynamic_cast<Splitter*>(sourceTabWidget->parentWidget());
            Q_ASSERT(sourceSplitter);

            sizeProperties.targetIndex = findTargetIndex(targetSplitter);
            sizeProperties.thisIndex = targetSplitter->indexOf(this);
            sizeProperties.targetSizes = targetSplitter->sizes();

			bool vertical = (targetSplitter->orientation() == Qt::Vertical);
            sizeProperties.targetSplitterSize = vertical ? targetSplitter->size().height() : targetSplitter->size().width();
            bool dropVertically = (mIndicatorArea == BOTTOM || mIndicatorArea == TOP);
			bool createNewSplitter = vertical != dropVertically;

            Splitter* newSplitter = nullptr;
            TabWidget* newTabWidget = nullptr;

			if (createNewSplitter) {
                Qt::Orientation orientation = vertical ? Qt::Horizontal : Qt::Vertical;
                newSplitter = targetSplitter->insertChildSplitter(sizeProperties.thisIndex, orientation);
                newSplitter->insertWidget(0, this);
                newTabWidget = new TabWidget(newSplitter);
                newTabWidget->insertTab(0, sourceTab, tabTitle);
                newSplitter->insertWidget(sizeProperties.targetIndex, newTabWidget);
                newSplitter->restoreSizesAfterDrag(Splitter::newSplitter, sizeProperties);
            } else {
                newTabWidget = new TabWidget(targetSplitter);
                targetSplitter->insertWidget(sizeProperties.targetIndex, newTabWidget);
                newTabWidget->insertTab(0, sourceTab, tabTitle);

                sizeProperties.insertSize = vertical ? newTabWidget->minimumSizeHint().height() : newTabWidget->minimumSizeHint().width();
                sizeProperties.onlyMove = sourceSplitter == targetSplitter && sizeProperties.sourceHasOnlyOneTab;
                targetSplitter->restoreSizesAfterDrag(Splitter::targetSplitter, sizeProperties);
            }
        }

        event->acceptProposedAction();
        event->accept();
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

void TabWidget::updateIndicatorArea(QPoint& p) {
    //calculate what area the mouse cursor is in.
    int top = rect().top();
    int width = rect().width();
    int height = rect().height();
    int marginX = width * 0.5;
    int marginY = height * 0.3;

    bool tabBarArea = (p.y() < tabBar()->rect().height());

    bool topArea = (top + marginY > p.y()) && !tabBarArea;
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
            rect.setBottom(marginY);
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
