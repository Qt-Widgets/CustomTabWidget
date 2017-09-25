#pragma once

#include <QSplitter>

class TabWidget;

struct DropProperties {
    int insertSize = -1, index = -1, dropToIndex = -1, sourceWidgetIndex = -1, targetSplitterSize = -1, dropWidgetIndex = -1;
    bool onlyMove = false, removeSourceWidget = false, createNewSplitter = false, droppedOnSelf = false, insertAfter = false;
    int* dragLocation = nullptr;
    int* dropLocation = nullptr;
    QVector<int*> sourceSizes, targetSizes; //yes pointers because we have to remember original drag and drop locations.

    virtual ~DropProperties();
    QVector<int*> intToPointers(QVector<int> input);
    QVector<int> pointersToInt(QVector<int*> input);
    void deleteIntPointers(QVector<int*> input);
};

class Splitter : public QSplitter {
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
    virtual ~Splitter();

	void setAsRoot();
	Splitter* findSplitter(QWidget* target, int& index);
	Splitter* root();
	void tabWidgetAboutToDelete(TabWidget* widget);
    Splitter* insertChildSplitter(int toIndex, Qt::Orientation orientation);

	//for debuggin
	QString indentation(int level);
	QString splitterBranch(Splitter* splitter = nullptr, int level = 0);
    QString printSplitterTree();

    //restoring splitter sizes after drag operation.
    enum SplitterType { sourceSplitter, targetSplitter, newSplitter };
    void restoreSizesAfterDrag(SplitterType splitterType, DropProperties &p);
    QVector<int> splitIndexSizes(DropProperties &p);

public slots:
    void cleanupSplitterTree();

protected:
    void cleanSplitter(Splitter* splitter);

	static Splitter* mRoot;
};
