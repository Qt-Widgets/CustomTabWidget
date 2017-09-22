#pragma once
#include <QTabWidget>
#include <QFrame>
#include <QPushButton>

#include "utils.h"
#include "include/drawoverlay.h"
#include "include/splitter.h"

class QDrag;
class TabBar;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0, Splitter* splitter = 0);
    virtual ~TabWidget();

protected:
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
	void onTabBarEmpty();
	void onAddNewTab();

private:
    void updateIndicatorRect();
    void updateIndicatorArea(QPoint& p);
	void tabDragged(int index, int tabCount);
    int findTargetIndex(const Splitter* targetSplitter);
    void drawDropWindow(QPixmap &pixmap, QRect tabRect, QString text);

private:
    const int mIndicatorMargin = 10;
    utils::DropArea mIndicatorArea = utils::DropArea::INVALID;
    DrawOverlay* mDrawOverlay;
	TabBar* mTabBar;
	QPushButton* mMenuButton;
	static QMap<TabWidget*, Splitter*> mSplitters;
};
