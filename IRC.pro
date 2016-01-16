#-------------------------------------------------
#
# Project created by QtCreator 2015-12-22T17:58:19
#
#-------------------------------------------------

QT       += core gui
CONFIG += network c++11
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IRC
TEMPLATE = app

LIBS += -lwsock32

SOURCES += main.cpp\
        mainwindow.cpp \
    signin.cpp \
    serverlistener.cpp \
    privatechat.cpp

HEADERS  += mainwindow.h \
    signin.h \
    serverlistener.h \
    privatechat.h

FORMS    += mainwindow.ui \
    signin.ui \
    privatechat.ui
