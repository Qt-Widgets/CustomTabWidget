#pragma once
#include <QTabWidget>
#include <QFrame>
#include <QPushButton>
#include <utils.h>

#include "include/drawoverlay.h"
#include "include/splitter.h"

class QDrag;
class TabBar;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0, Splitter* splitter);
    virtual ~TabWidget();

protected:
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateIndicatorRect();
    void updateIndicatorArea(QPoint& p);
	void tabDragged(int index, int tabCount);

private:
    const int mIndicatorMargin = 10;
    utils::DropArea mIndicatorArea = utils::DropArea::INVALID;
    DrawOverlay* mDrawOverlay;
	TabBar* mTabBar;
	QPushButton* mMenuButton;
	static QMap<TabWidget*, Splitter*> mSplitters;
};