#pragma once

#include <QSplitter>

class Splitter : public QSplitter {
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
	void setAsRoot();
	bool hasTabWidgets();
	Splitter* findSplitter(QWidget* target, int& index);
	Splitter* root();
    void removeIfEmpty();

protected:
	static Splitter* mRoot;
};
