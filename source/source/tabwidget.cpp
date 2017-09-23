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
        if (mIndicatorArea == utils::DropArea::INVALID) {
            qDebug() << "ERROR: drop area was invalid.";
            mDrawOverlay->setRect(QRect());
            event->ignore();
            return;
        }

        int sourceIndex = mime->data(sourceIndexMimeDataKey()).toInt();
        QString tabTitle = QString::fromStdString(mime->data(sourceTabTitleMimeDataKey()).toStdString());
        TabWidget* sourceTabWidget = static_cast<TabWidget*>(event->source());
		Q_ASSERT(sourceTabWidget);
        QWidget* sourceTab = sourceTabWidget->widget(sourceIndex);
		Q_ASSERT(sourceTab);
		
        if (mIndicatorArea == utils::DropArea::TABBAR) {
            //dropped on tabbar
            QPoint mousePos = tabBar()->mapFromGlobal(QCursor::pos());
            int targetIndex = tabBar()->tabAt(mousePos);
            insertTab(targetIndex, sourceTab, tabTitle);
        } else {
            //dropped on widget area
			Splitter* targetSplitter = dynamic_cast<Splitter*>(parentWidget());
            Q_ASSERT(targetSplitter);
			bool vertical = (targetSplitter->orientation() == Qt::Vertical);
			bool dropVertically = (mIndicatorArea == utils::BOTTOM || mIndicatorArea == utils::TOP);
			bool createNewSplitter = vertical != dropVertically;
			int targetIndex = findTargetIndex(targetSplitter);

			if (createNewSplitter) {
                Qt::Orientation orientation = vertical ? Qt::Horizontal : Qt::Vertical;
                targetSplitter = targetSplitter->create(targetSplitter, targetIndex, orientation);
                targetSplitter->insertWidget(0, this);
            }

            TabWidget* newTabWidget = new TabWidget(targetSplitter);
            newTabWidget->insertTab(0, sourceTab, tabTitle);
        }

        event->acceptProposedAction();
        event->accept();
        mDrawOverlay->setRect(QRect());
    }
}

int TabWidget::findTargetIndex(const Splitter* targetSplitter) {
    int targetIndex = targetSplitter->indexOf(this);
    bool insertAfterTarget = (mIndicatorArea == utils::BOTTOM || mIndicatorArea == utils::RIGHT);
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
	}
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
        mIndicatorArea = utils::DropArea::TOP;
    } else if (bottomArea) {
        mIndicatorArea = utils::DropArea::BOTTOM;
    } else if (rightArea) {
        mIndicatorArea = utils::DropArea::RIGHT;
    } else if (leftArea) {
        mIndicatorArea = utils::DropArea::LEFT;
    } else if (tabBarArea) {
        mIndicatorArea = utils::DropArea::TABBAR;
    }
}

void TabWidget::updateIndicatorRect() {
    if (mIndicatorArea != utils::DropArea::INVALID) {
        QRect rect = this->rect();
        rect.setTop(tabBar()->rect().height());
        int marginY = rect.height() * 0.4;
        int marginX = rect.width() * 0.4;

        switch (mIndicatorArea) {
        case utils::DropArea::TOP:
            rect.setBottom(marginY);
            break;
        case utils::DropArea::BOTTOM:
            rect.setTop(rect.bottom() - marginY);
            break;
        case utils::DropArea::RIGHT:
            rect.setLeft(rect.right() - marginX);
            break;
        case utils::DropArea::LEFT:
            rect.setRight(rect.left() + marginX);
            break;
        case utils::DropArea::TABBAR: {
            QPoint mousePos = tabBar()->mapFromGlobal(QCursor::pos());
            int index = tabBar()->tabAt(mousePos);
            rect = tabBar()->tabRect(index);
            rect.setRight(rect.left());
            break;
        }
        case utils::DropArea::INVALID:
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

    {
        drag->exec(Qt::MoveAction);
    }

    //clear widgets and splitters after drag finished.
    if (tabBar()->count() == 0) {
        deleteLater();
    }
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
