#-------------------------------------------------
#
# Project created by QtCreator 2018-08-29T00:04:31
#
#-------------------------------------------------

QT       += core gui widgets serialport

TARGET = PedalVibration
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    serialthread.cpp \
    telemetryreader.cpp \
    assettocorsadata.cpp \
    settings.cpp \
    wheelslipconfiguration.cpp \
    windfanconfiguration.cpp \
    sender.cpp

HEADERS += \
        mainwindow.h \
    serialthread.h \
    telemetryreader.h \
    assettocorsadata.h \
    sharedfileout.h \
    settings.h \
    wheelslipconfiguration.h \
    windfanconfiguration.h \
    sender.h \
    globals.h

FORMS += \
        mainwindow.ui \
    wheelslipconfiguration.ui \
    windfanconfiguration.ui

DISTFILES +=

RESOURCES += \
    resources.qrc
