#include <include/tabbar.h>
#include <QMouseEvent>
#include <QApplication>

TabBar::TabBar(QWidget *parent) : QTabBar(parent) {
    QObject::connect(this, &TabBar::tabCloseRequested, this, &TabBar::onCloseTab);
}

void TabBar::mousePressEvent(QMouseEvent* event) {
    //QTabBar::mousePressEvent(event);
	mDragStartPosition = event->pos();
	mDragIndex = tabAt(event->pos());
}

void TabBar::mouseMoveEvent(QMouseEvent* event) {
	if (!(event->buttons() & Qt::LeftButton)) {
		return;
	}
	if ((event->pos() - mDragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
		return;
	}

    emit mouseDragged(mDragIndex, count());
}

void TabBar::mouseReleaseEvent(QMouseEvent *event) {
    int index = tabAt(event->pos());
    setCurrentIndex(index);
}

void TabBar::onCloseTab(int index) {
    removeTab(index);
    if (count() == 0) {
        emit hasNoTabs();
    }
}
