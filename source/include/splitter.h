#pragma once

#include <QSplitter>

class TabWidget;

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
    QList<int> splitIndexSizes(int insertSize, int index, int targetIndex, QList<int> sizes, bool onlyMove, int sourceIndex);

	//for debuggin
	QString indentation(int level);
	QString splitterBranch(Splitter* splitter = nullptr, int level = 0);
    QString printSplitterTree();

    //SplitterType is used when restoring splitter handle sizes after a drop operation.
    enum SplitterType { sourceSplitter, targetSplitter, newSplitter };
    struct RestoreSizeProperties {
        int insertSize;
        int index;
        int targetIndex;
        QList<int> sizes;
        QList<int> targetSizes;
        bool onlyMove;
        int sourceIndex;
        int targetSplitterSize;
        int thisIndex;
        bool sourceHasOnlyOneTab;
    };
    void restoreSizesAfterDrag(SplitterType splitterType, RestoreSizeProperties input);

public slots:
    void cleanupSplitterTree();

protected:
    void cleanSplitter(Splitter* splitter);

	static Splitter* mRoot;
};
