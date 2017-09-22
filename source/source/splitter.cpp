#include "include/splitter.h"
#include "include/tabwidget.h"
#include <qDebug>

Splitter* Splitter::mRoot = nullptr;

Splitter::Splitter(QWidget *parent) : QSplitter(parent) {

}

void Splitter::setAsRoot() {
	Q_ASSERT(!mRoot); //todo: handle case where you want to set a new root splitter.

	mRoot = this;
}

bool Splitter::hasTabWidgets() {
    QList<TabWidget*> widgets = findChildren<TabWidget*>(QString(), Qt::FindDirectChildrenOnly);
    return !widgets.isEmpty();
}

Splitter* Splitter::findSplitter(QWidget* target, int& index) {
	Q_ASSERT(mRoot);
	QList<Splitter*> splitters = mRoot->findChildren<Splitter*>();
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

void Splitter::removeIfEmpty(Splitter* splitter) {
	QList<TabWidget*> widgets = splitter->findChildren<TabWidget*>(QString(), Qt::FindDirectChildrenOnly);
	QList<Splitter*> splitters = splitter->findChildren<Splitter*>(QString(), Qt::FindDirectChildrenOnly);

	if (widgets.count() == 0) {
		if (splitters.count() == 0) {
			splitter->deleteLater();
		}
	}
}

void Splitter::removeAllEmptySplitters() {
	if (!mRoot) {
		return;
	}

	QList<TabWidget*> widgets = mRoot->findChildren<TabWidget*>();
	QList<Splitter*> splitters = mRoot->findChildren<Splitter*>();
	splitters.append(mRoot);
	for (auto splitter : splitters) {
		removeIfEmpty(splitter);
	}
}

void Splitter::tabWidgetAboutToDelete(TabWidget* widget) {
	QList<TabWidget*> widgets = findChildren<TabWidget*>(QString(), Qt::FindDirectChildrenOnly);
	QList<Splitter*> splitters = findChildren<Splitter*>();
	if (widgets.count() == 1) {
		if (splitters.count() == 0) {
			deleteLater();
		}
	}
}
