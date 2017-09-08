#include "include/drawoverlay.h"
#include <QPainter>

DrawOverlay::DrawOverlay(QWidget *parent) : QWidget(parent) {
    setAttribute(Qt::WA_NoSystemBackground);
    //overlay is used only to draw on top of widgets, so we want to make this transparent for all mouse events.
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void DrawOverlay::setRect(QRect rect) {
    mRect = rect;
    update();
}

void DrawOverlay::paintEvent(QPaintEvent* /*event*/) {
    if (mRect != QRect()) {
        QPainter painter(this);
        painter.setPen(QPen(QBrush(QColor("#ffbd00")), 3));
        painter.drawRect(mRect);
    }
}
