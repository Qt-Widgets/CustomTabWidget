#include "include/splitter.h"
#include "include/tabwidget.h"
#include "utils.h"
#include <qDebug>

Splitter* Splitter::mRoot = nullptr;

Splitter::Splitter(QWidget *parent) : QSplitter(parent) {
	QObject::connect(this, &Splitter::removeAllEmptySplitters, this, &Splitter::onRemoveAllEmptySplitters, Qt::QueuedConnection);
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

void Splitter::onRemoveAllEmptySplitters() {
	if (!mRoot) {
		return;
	}

	QList<TabWidget*> widgets = mRoot->findChildren<TabWidget*>();
	QList<Splitter*> splitters = mRoot->findChildren<Splitter*>();
	splitters.append(mRoot);
	for (auto splitter : splitters) {
		removeIfEmpty(splitter);
	}

	qDebug() << "tabWidgetAboutToDelete";

	qDebug() << " ";
	qDebug().noquote() << printSplitterTree(mRoot);
	qDebug() << " ";
}

void Splitter::tabWidgetAboutToDelete(TabWidget* widget) {
	QList<TabWidget*> widgets = findChildren<TabWidget*>(QString(), Qt::FindDirectChildrenOnly);
	QList<Splitter*> splitters = findChildren<Splitter*>(QString(), Qt::FindDirectChildrenOnly);

	if (widgets.count() + splitters.count() == 0) {
		deleteLater();
	}

	emit removeAllEmptySplitters();
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

QString Splitter::printSplitterTree(Splitter* splitter /*= nullptr*/) {
	QString text = QString("root\n");
	text.append(splitterBranch(mRoot, 1));
	return text;
}
