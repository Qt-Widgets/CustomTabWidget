#pragma once

#include <QWidget>

class DrawOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit DrawOverlay(QWidget *parent = 0);
    void setRect(QRect rect);
    QRect getRect() { return mRect; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    //todo: this is not nice to copy the rectangle around. Replace with pointer.
    QRect mRect = QRect();
};
