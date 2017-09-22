#pragma once

#include <QSplitter>

class TabWidget;

class Splitter : public QSplitter {
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
	void setAsRoot();
	bool hasTabWidgets();
	Splitter* findSplitter(QWidget* target, int& index);
	Splitter* root();
	void removeIfEmpty(Splitter* splitter);
	void onRemoveAllEmptySplitters();
	void tabWidgetAboutToDelete(TabWidget* widget);

	//for debuggin
	QString indentation(int level);
	QString splitterBranch(Splitter* splitter = nullptr, int level = 0);
	QString printSplitterTree(Splitter* splitter = nullptr);

signals:
	void removeAllEmptySplitters();

protected:
	static Splitter* mRoot;
};
