#include <QTabBar>
#include <QDragMoveEvent>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QWindow>
#include <QApplication>
#include <assert.h>
#include <mainwindow.h>

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
    installEventFilter(this);
    setAcceptDrops(true);
    //setTabsClosable(true);

    QObject::connect(mTabBar, &TabBar::mouseDragged, this, &TabWidget::tabDragged);
}

TabWidget::~TabWidget() {
    setAcceptDrops(false);
	QObject::disconnect(mTabBar, &TabBar::mouseDragged, this, &TabWidget::tabDragged);
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
            //event->setDropAction(Qt::MoveAction);
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
        int sourceIndex = mime->data(sourceIndexMimeDataKey()).toInt();
        QString tabTitle = QString::fromStdString(mime->data(sourceTabTitleMimeDataKey()).toStdString());
        TabWidget* sourceTabWidget = static_cast<TabWidget*>(event->source());
        QWidget* sourceWidget = sourceTabWidget->widget(sourceIndex);
        QPoint mousePos = tabBar()->mapFromGlobal(QCursor::pos());
        int targetIndex = tabBar()->tabAt(mousePos);

        if (mIndicatorArea == utils::DropArea::TABBAR) {
            insertTab(targetIndex, sourceWidget, tabTitle);
        } else {
            //TabWidgetContainer* sourceContainer = static_cast<TabWidgetContainer*>(sourceTabWidget->parentWidget());
            //TabWidgetContainer* targetContainer = static_cast<TabWidgetContainer*>(this->parentWidget());
            //MainWindow::splitterManager()->splitTabWidget(sourceIndex, sourceContainer, targetContainer, mIndicatorArea, tabTitle);
        }

        event->acceptProposedAction();
        event->accept();
        mDrawOverlay->setRect(QRect());
    }
}

void TabWidget::resizeEvent(QResizeEvent* event) {
    QTabWidget::resizeEvent(event);
    if (mDrawOverlay) {
        mDrawOverlay->setGeometry(this->rect());
    }
    event->accept();
}

void TabWidget::updateIndicatorArea(QPoint& p) {
    //calculate what area the mouse cursor is in.
    int left = rect().left();
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
	QPixmap pixmap(tabBar()->tabRect(index).size() * dpr);
	pixmap.setDevicePixelRatio(dpr);
	render(&pixmap);
	
	QDrag* drag = new QDrag(this);
	drag->setMimeData(mimeData);
	drag->setPixmap(pixmap);
	Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
}
