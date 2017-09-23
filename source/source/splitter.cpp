#include "include/splitter.h"
#include "include/tabwidget.h"
#include "utils.h"
#include <qDebug>

Splitter* Splitter::mRoot = nullptr;

Splitter::Splitter(QWidget *parent) : QSplitter(parent) {
}

void Splitter::setAsRoot() {
	Q_ASSERT(!mRoot); //todo: handle case where you want to set a new root splitter.

	mRoot = this;
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
	if (splitter == mRoot) {
		return;
	}
	bool hasTabWidgets = false;
	bool hasSplitters = false;
	int splitterCount = 0;
	Splitter* childSplitter = nullptr;

	for (auto child : splitter->children()) {
		TabWidget* tabWidget = dynamic_cast<TabWidget*>(child);
		hasTabWidgets |= tabWidget != nullptr;
		
		Splitter* splitter = dynamic_cast<Splitter*>(child);
		hasSplitters |= splitter != nullptr;

		if (splitter) {
			if (splitterCount == 0) {
				childSplitter = splitter;
			} else {
				childSplitter = nullptr;
			}
			splitterCount++;
		}
	}
	
	bool isEmpty = !hasSplitters && !hasTabWidgets;
	bool isLonely = childSplitter && !hasTabWidgets;

	if (isEmpty) {
		splitter->deleteLater();
	} else if (isLonely) {
		//in this case there is only one child splitter and no tabWidgets, so for this
		//we want to delete this splitter and move children directly to parent object.
        Q_ASSERT(splitter->parentWidget());
		childSplitter->setParent(splitter->parentWidget());
		splitter->deleteLater();
	}
}

void Splitter::onRemoveAllEmptySplitters() {
	if (!mRoot) {
		return;
	}

	QList<Splitter*> splitters = mRoot->findChildren<Splitter*>();
	//splitters.append(mRoot);
	for (auto splitter : splitters) {
		removeIfEmpty(splitter);
	}
}

void Splitter::tabWidgetAboutToDelete(TabWidget* widget) {
	if (!widget) {
		return;
	}
    onRemoveAllEmptySplitters();
}

Splitter *Splitter::create(Splitter *parent, int toIndex, Qt::Orientation orientation) {
    QList<TabWidget*> widgets = parent->findChildren<TabWidget*>(QString(), Qt::FindDirectChildrenOnly);
    QList<Splitter*> splitters = parent->findChildren<Splitter*>(QString(), Qt::FindDirectChildrenOnly);

    Splitter* newSplitter = nullptr;
    if (widgets.count() + splitters.count() >= 1) {
        newSplitter = new Splitter(parent);
        parent->insertWidget(toIndex, newSplitter);
    } else {
        //no need to create a new one, just change the orientation of the parent.
        newSplitter = parent;
    }

    newSplitter->setOrientation(orientation);
    return newSplitter;
}

QString Splitter::indentation(int level) {
	QString value = QString();
	for (int i = 0; i < level; i++) {
		value.append("  ");
	}
	return value;
}

QString Splitter::splitterBranch(Splitter* splitter, int level) {
	if (!splitter) {
		return QString();
	}

	QString text;
	for (auto child : splitter->children()) {
		TabWidget* tabWidget = dynamic_cast<TabWidget*>(child);
		Splitter* splitter = dynamic_cast<Splitter*>(child);
		if (tabWidget) {
			text.append(indentation(level));
			text.append(QString("tabWidget (%1)\n").arg(tabWidget->children().count()));
		} else if (splitter) {
			text.append(indentation(level));
			text.append(QString("splitter (%1)\n").arg(splitter->children().count()));
			if (splitter->children().count() != 0) {
				text.append(splitterBranch(splitter, level + 1));
			}
		} else {
			//text.append(QString("unknown object (%1)\n").arg(child->children().count()));
		}
	}
	return text;
}

QString Splitter::printSplitterTree() {
	QString text = QString("root\n");
	text.append(splitterBranch(mRoot, 1));
	return text;
}
