#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStyleFactory>
#include <QTabWidget>

#include "splitter.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	QApplication::setEffectEnabled(Qt::UI_AnimateMenu, false);
	QApplication::setEffectEnabled(Qt::UI_FadeMenu, false);
	QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);
	QApplication::setEffectEnabled(Qt::UI_AnimateTooltip, false);
	QApplication::setEffectEnabled(Qt::UI_FadeTooltip, false);
	QApplication::setEffectEnabled(Qt::UI_AnimateToolBox, false);
	QApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents);
	QApplication::setStyle(QStyleFactory::create("fusion"));

    ui->setupUi(this);
	setAnimated(false);
	createWidgets();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createWidgets() {
	Splitter* splitter = new Splitter(this);
	QTabWidget* tabWidget = new QTabWidget(this);
	tabWidget->addTab(new QWidget(this), "blaa1");
	tabWidget->addTab(new QWidget(this), "blaa2");
	tabWidget->addTab(new QWidget(this), "blaa3");
	splitter->addWidget(tabWidget);
	setCentralWidget(splitter);
}
