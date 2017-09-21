#include "include/splitter.h"
#include "include/tabwidget.h"

Splitter* Splitter::mRoot = nullptr;

Splitter::Splitter(QWidget *parent) : QSplitter(parent) {

}

void Splitter::setAsRoot() {
	Q_ASSERT(!mRoot); //todo: handle case where you want to set a new root splitter.

	mRoot = this;
}

bool Splitter::hasTabWidgets() {
    QList<TabWidget*> widgets = findChildren<TabWidget*>("*", Qt::FindDirectChildrenOnly);
    return !widgets.isEmpty();
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

void Splitter::removeIfEmpty() {
    if (children().isEmpty()) {
        deleteLater();
    }
}
