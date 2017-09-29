#pragma once
#include <QTabWidget>
#include <QFrame>
#include <QPushButton>

#include "include/drawoverlay.h"
#include "include/splitter.h"

class QDrag;
class TabBar;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);
    virtual ~TabWidget();
    enum DropArea { TABBAR, TOP, RIGHT, BOTTOM, LEFT, INVALID };

protected:
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

	//degubbing
	void keyReleaseEvent(QKeyEvent *event) override;

private slots:
	void onAddNewTab();
    void onHasNoTabs();
	void closeTab(int index);
	void onTabDestroyed(QObject* object);

private:
    void updateIndicatorRect();
    void updateIndicatorArea(QPoint& p);
    void tabDragged(int index);
    int findTargetIndex(const Splitter* targetSplitter);
    void drawDragPixmap(QPixmap &pixmap, QRect tabRect, QString text);
    DropProperties createDropProperties(TabWidget* sourceTabWidget,
                                        Splitter* targetSplitter,
                                        Splitter* sourceSplitter);

private:
    const int mIndicatorMargin = 10;
    DropArea mIndicatorArea = DropArea::INVALID;
    DrawOverlay* mDrawOverlay;
	TabBar* mTabBar;
	QPushButton* mMenuButton;
};
