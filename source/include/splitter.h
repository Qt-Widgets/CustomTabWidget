#pragma once

#include <QSplitter>

class Splitter : public QSplitter {
	Q_OBJECT
public:
	explicit Splitter(QWidget *parent = 0);
};