#pragma once

#include <QWidget>

class DrawOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit DrawOverlay(QWidget *parent = 0);
    void setRect(QRect rect);
    QRect getRect() { return mRect; }
    void setRectHidden(bool input) { mHidden = input; }
    bool isRectHidden() { return mHidden; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QRect mRect = QRect();
    bool mHidden = true;
};
