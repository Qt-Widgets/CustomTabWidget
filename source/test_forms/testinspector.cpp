#include "testinspector.h"
#include "ui_test_inspector.h"

TestInspector::TestInspector(QWidget* parent) : QWidget(parent) {
    mUi = new Ui::Inspector();
    mUi->setupUi(this);
}

TestInspector::~TestInspector() {
    delete mUi;
}
