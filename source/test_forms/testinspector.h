#pragma once

#include <QWidget>

namespace Ui {
class Inspector;
}

class TestInspector : public QWidget
{
public:
    TestInspector(QWidget* parent = 0);
    ~TestInspector();

private:
    Ui::Inspector* mUi;
};
