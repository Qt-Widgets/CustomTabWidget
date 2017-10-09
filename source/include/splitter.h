#pragma once

#include <QSplitter>
#include <memory>

class TabWidget;
class QXmlStreamWriter;

struct DropProperties {
    int widgetMinSize = -1, targetSplitterSize = -1, sizeRemoved = 0;
    int sourceSplitterHeight = -1;
    bool onlyMove = false, removeSourceWidget = false, createNewSplitter = false;
    bool droppedOnSelf = false, insertAfter = false, dropOnTabBar = false, orientationsAreSame = false;
    std::shared_ptr<int> dragLocation = nullptr;
    std::shared_ptr<int> dropLocation = nullptr;
    //vector of pointers because we have to remember original drag and drop locations even after
    //items has been removed or inserted to the vector.
    QVector<std::shared_ptr<int>> sourceSizes, targetSizes;

    QVector<std::shared_ptr<int>> intToPointers(QVector<int> input);
    QVector<int> pointersToInt(QVector<std::shared_ptr<int> > input);
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
    enum SplitterType { targetSplitter, newSplitter };
    void restoreSizesAfterDrag(SplitterType splitterType, DropProperties &p);
    QVector<int> splitIndexSizes(DropProperties &p);
    void insertWidget(int index, QWidget* widget);

    QByteArray saveState() const = delete; //removes the default functionality
    void saveStateRecursive();
    void restoreStateRecursive();
    void branchState(Splitter* splitter, QXmlStreamWriter &xmlWriter);

public slots:
    void cleanupSplitterTree(QObject *object);

protected:
    void cleanSplitter(Splitter* splitter);
	static Splitter* mRoot;

private:
    QString mFilename = "/Users/petrij/CustomTabWidget/test.xml";
};
