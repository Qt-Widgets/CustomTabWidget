#-------------------------------------------------
#
# Project created by QtCreator 2017-09-08T14:42:18
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = source
TEMPLATE = app

QMAKE_MAC_SDK = macosx10.13

# Compiler warnings when using depricated qt features.
DEFINES += QT_DEPRECATED_WARNINGS

HEADERS  += mainwindow.h \
        include/drawoverlay.h \
        include/splitter.h \
        include/tabwidget.h \
        include/tabbar.h \
    test_forms/testinspector.h

SOURCES += main.cpp\
        mainwindow.cpp \
        source/splitter.cpp \
        source/drawoverlay.cpp \
        source/tabbar.cpp \
        source/tabwidget.cpp \
    test_forms/testinspector.cpp

FORMS    += mainwindow.ui \
    test_forms/test_inspector.ui
