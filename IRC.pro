#-------------------------------------------------
#
# Project created by QtCreator 2015-12-22T17:58:19
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IRC
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    signin.cpp \
    serverlistener.cpp

HEADERS  += mainwindow.h \
    signin.h \
    serverlistener.h

FORMS    += mainwindow.ui \
    signin.ui
