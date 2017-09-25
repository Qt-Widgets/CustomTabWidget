#include "include/splitter.h"
#include "include/tabwidget.h"
#include "mainwindow.h"

#include <qDebug>

Splitter* Splitter::mRoot = nullptr;

Splitter::Splitter(QWidget *parent) : QSplitter(parent) {
    if (!mRoot) {
        setAsRoot();
    }
}

Splitter::~Splitter() {

}

void Splitter::setAsRoot() {
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

void Splitter::cleanSplitter(Splitter* splitter) {
	bool hasTabWidgets = false;
    bool hasSplitters = false;
    QList<QWidget*> childWidgets;
    Splitter* parentSplitter = dynamic_cast<Splitter*>(splitter->parentWidget());

	for (auto child : splitter->children()) {
		TabWidget* tabWidget = dynamic_cast<TabWidget*>(child);
		hasTabWidgets |= tabWidget != nullptr;

		Splitter* splitter = dynamic_cast<Splitter*>(child);
		hasSplitters |= splitter != nullptr;

        if (tabWidget) {
            childWidgets.append(tabWidget);
        } else if (splitter) {
            childWidgets.append(splitter);
        }
	}
	
    bool isEmpty = !hasSplitters && !hasTabWidgets;
    bool isLonely = childWidgets.count() < 2;
    bool sameOrientation = parentSplitter && (parentSplitter->orientation() == splitter->orientation());

    if (isEmpty && splitter != mRoot) {
		splitter->deleteLater();
    } else if (isLonely) {
        //in this case there is only one child, so for this we want to
        //delete this splitter and move the child directly to parent object.
        Q_ASSERT(splitter->parentWidget());
        Q_ASSERT(childWidgets.first());

        if (childWidgets.count() == 1) {
            //move child to parent.
            if (parentSplitter) {
                int index = parentSplitter->indexOf(splitter);
                parentSplitter->insertWidget(index, childWidgets.first());
            }

            Splitter* childSplitter = dynamic_cast<Splitter*>(childWidgets.first());
            if (splitter == mRoot && childSplitter) {
                MainWindow* parentWidget = dynamic_cast<MainWindow*>(mRoot->parentWidget());
                childSplitter->setParent(parentWidget);
                parentWidget->setCentralWidget(childSplitter);
                childSplitter->setAsRoot();
            }
        }

        if (splitter != mRoot) {
            splitter->deleteLater();
        }
    } else if (sameOrientation && childWidgets.count() != 0) {
        //splitter has the same orientation as the parent, so we can just move
        //children to the parent, and delete the splitter.
        int index = parentSplitter->indexOf(splitter);
        int i = 0;
        for (auto child : childWidgets) {
            parentSplitter->insertWidget(index + i, child);
            i++;
        }

        if (splitter != mRoot) {
            splitter->deleteLater();
        }
    }
}

void Splitter::cleanupSplitterTree() {
	if (!mRoot) {
		return;
	}

	QList<Splitter*> splitters = mRoot->findChildren<Splitter*>();
    splitters.append(mRoot);
	for (auto splitter : splitters) {
        cleanSplitter(splitter);
	}
}

void Splitter::tabWidgetAboutToDelete(TabWidget* widget) {
	if (!widget) {
		return;
	}
    cleanupSplitterTree();
}

Splitter *Splitter::insertChildSplitter(int toIndex, Qt::Orientation orientation) {
    QList<TabWidget*> widgets = findChildren<TabWidget*>(QString(), Qt::FindDirectChildrenOnly);
    QList<Splitter*> splitters = findChildren<Splitter*>(QString(), Qt::FindDirectChildrenOnly);

    Splitter* newSplitter = nullptr;
    if (widgets.count() + splitters.count() >= 1) {
        newSplitter = new Splitter();
        insertWidget(toIndex, newSplitter);
    } else {
        //no need to create a new one, just change the orientation of the parent.
        newSplitter = this;
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

void Splitter::restoreSizesAfterDrag(Splitter::SplitterType splitterType, DropProperties& p) {
    if (splitterType == sourceSplitter) {
        if (p.droppedOnSelf) {
            //
        } else {
            //
        }
    } else if (splitterType == targetSplitter) {
        setSizes(splitIndexSizes(p).toList());
    } else if (splitterType == newSplitter) {
        int halfSize = int(p.targetSplitterSize / 2);
        setSizes({ halfSize, halfSize });
        //int insertSize = vertical ? newTabWidget->minimumSizeHint().height() : newTabWidget->minimumSizeHint().width();
        //targetSizes = targetSplitter->splitIndexSizes(insertSize, thisIndex, dropToIndex, targetSizes, false, sourceIndex);
        //targetSplitter->setSizes(targetSizes);
    }
}

QVector<int> Splitter::splitIndexSizes(DropProperties& p) {
    if (p.targetSizes.isEmpty()) {
        return {};
    }

    int availableSpace = *p.dropLocation;

    if (availableSpace / 2 < p.insertSize || p.insertSize == 0) {
        //split in half when not enough space to do otherwise
        p.insertSize = availableSpace / 2;
        *p.dropLocation = p.insertSize;
    } else {
        *p.dropLocation = availableSpace - p.insertSize;
    }

    int dragIndex = p.targetSizes.indexOf(p.dragLocation);
    int dropIndex = p.targetSizes.indexOf(p.dropLocation);
    dropIndex = p.insertAfter ? dropIndex + 1 : dropIndex;
    bool onlyMove = p.droppedOnSelf && p.removeSourceWidget;
    if (onlyMove) {
        p.targetSizes.move(dragIndex, dropIndex);
        //sizes[dropToIndex] = splitSize;
    } else {
        p.targetSizes.insert(dropIndex, std::make_shared<int>(p.insertSize));
    }

    return p.pointersToInt(p.targetSizes);
}

QVector<std::shared_ptr<int> > DropProperties::intToPointers(QVector<int> input) {
    QVector<std::shared_ptr<int>> result;
    for (int item : input) {
        result.append(std::make_shared<int>(item));
    }
    return result;
}

QVector<int> DropProperties::pointersToInt(QVector<std::shared_ptr<int>> input) {
    QVector<int> result;
    for (auto item : input) {
        result.append(*item);
    }
    return result;
}
