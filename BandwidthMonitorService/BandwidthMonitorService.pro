QT += core network
QT -= gui

TARGET = BandwidthMonitorService
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    monitorservice.cpp \
    config.cpp \
    mainthread.cpp \
    monitorthread.cpp

include(qtservice/src/qtservice.pri)

HEADERS += \
    monitorservice.h \
    config.h \
    mainthread.h \
    monitorthread.h

LIBS += C:\npcap-sdk-1.13\Lib\wpcap.lib \
        -lws2_32

INCLUDEPATH += C:\npcap-sdk-1.13\Include

