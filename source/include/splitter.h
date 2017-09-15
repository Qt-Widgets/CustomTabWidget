#pragma once

#include <QSplitter>

class Splitter : public QSplitter {
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
	void setAsRoot();
	QList<QWidget*> getWidgets();
	bool hasTabWidgets();
	Splitter* findSplitter(QWidget* target, int& index);
	Splitter* root();

protected:
	static Splitter* mRoot;
};