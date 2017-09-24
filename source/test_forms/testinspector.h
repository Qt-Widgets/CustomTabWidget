#pragma once

#include <QWidget>

namespace Ui {
class Inspector;
}

class TestInspector : public QWidget
{
public:
    explicit TestInspector(QWidget* parent = 0);
    virtual ~TestInspector();

private:
    Ui::Inspector* mUi;
};
