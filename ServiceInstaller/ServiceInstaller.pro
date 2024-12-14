#-------------------------------------------------
#
# Project created by QtCreator 2024-04-29T08:44:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ServiceInstaller
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui

include(qtservice/src/qtservice.pri)
