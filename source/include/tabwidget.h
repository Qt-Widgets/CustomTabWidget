#pragma once
#include <QTabWidget>
#include <QFrame>
#include <QPushButton>

#include "include/drawoverlay.h"
#include "include/splitter.h"

class QDrag;
class TabBar;
class QMenu;

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);
    virtual ~TabWidget();
    enum DropArea { TABBAR, TOP, RIGHT, BOTTOM, LEFT, INVALID };
    void addMenuActions(QList<QAction*> actions);
    void clearMenuActions();
    QString getTabsAsType();

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
    void closeCurrentTab();
    void floatCurrentTab();
	void onTabDestroyed(QObject* object);
    void onCornerMenuClicked();

private:
    void addDefaultMenuActions();
    void updateIndicatorRect();
    void updateIndicatorArea(QPoint& p);
    void tabDragged(int index);
    int findTargetIndex(const Splitter* targetSplitter);
    void drawDragPixmap(QPixmap &pixmap, QRect tabRect, QString text);
    DropProperties createDropProperties(TabWidget* sourceTabWidget,
                                        Splitter* targetSplitter,
                                        Splitter* sourceSplitter);

private:
    DropArea mIndicatorArea = DropArea::INVALID;
    DrawOverlay* mDrawOverlay;
	TabBar* mTabBar;
	QPushButton* mMenuButton;
    QMenu* mCornerMenu;
};
