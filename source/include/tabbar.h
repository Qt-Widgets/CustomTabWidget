#pragma once
#include <QTabBar>
#include <QFrame>
#include <QPushButton>
#include <utils.h>

#include "include/drawoverlay.h"

class QDrag;

class TabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = 0);

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

signals:
	void mouseDragged(int index, int tabCount);

private:
	QPoint mDragStartPosition;
	int mDragIndex;
};
