#-------------------------------------------------
#
# Project created by QtCreator 2017-09-08T14:42:18
#
#-------------------------------------------------
INCLUDEPATH += $$PWD/editor-widgets/include
DEPENDPATH += $$PWD/editor-widgets/include

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = source
TEMPLATE = app

# Compiler warnings when using depricated qt features.
DEFINES += QT_DEPRECATED_WARNINGS

SUBDIRS = \
		editor-widgets

HEADERS  += mainwindow.h \
		include\drawoverlay.h
		include\splitter.h \

SOURCES += main.cpp\
        mainwindow.cpp
		source\splitter.cpp \
		source\drawoverlay.cpp

FORMS    += mainwindow.ui \
