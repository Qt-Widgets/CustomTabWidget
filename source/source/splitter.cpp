#include "include\splitter.h"
#include "include\tabwidget.h"

Splitter* Splitter::mRoot = nullptr;

Splitter::Splitter(QWidget *parent) : QSplitter(parent) {

}

void Splitter::setAsRoot() {
	Q_ASSERT(!mRoot); //todo: handle case where you want to set a new root splitter.

	mRoot = this;
}

QList<QWidget*> Splitter::getWidgets() {
	QList<QWidget*> widgets;
	for (int i = 0; i < count(); i++) {
		widgets.append(widget(i));
	}
	return widgets;
}

bool Splitter::hasTabWidgets() {
	QList<QWidget*> widgets = getWidgets();
	for (QWidget* item : widgets) {
		TabWidget* tabWidget = static_cast<TabWidget*>(item);
		if (tabWidget) {
			return true;
		}
	}
	return false;
}

Splitter* Splitter::findSplitter(QWidget* target, int& index) {
	Q_ASSERT(mRoot);
	QList<Splitter*> splitters = mRoot->findChildren<Splitter*>("*");
	for (Splitter* splitter : splitters) {
		int i = splitter->indexOf(static_cast<QWidget*>(target));
		if (i < 0) {
			//target not found in this splitter
			continue;
		} else {
			//container's splitter found
			index = i;
			return splitter;
		}
	}
	return nullptr;
}

Splitter* Splitter::root() {
	return mRoot;
}
