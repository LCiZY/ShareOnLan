#-------------------------------------------------
#
# Project created by QtCreator 2020-10-25T12:48:21
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ShareOnLan
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
    fileserver.cpp \
    msgserver.cpp \
    globaldata.cpp \
    shareonlan.cpp \
    component_ui.cpp \
    user_setting.cpp

HEADERS += \
    fileserver.h \
    globaldata.h \
    msgserver.h \
    shareonlan.h \
    component_ui.h \
    user_setting.h

FORMS += \
    progressui.ui \
    ShareOnLan.ui \
    connect2ui.ui

DISTFILES += \
    defaultConfiguration.ini \
    logo.rc

RESOURCES += \
    config.qrc \
    icon.qrc \
    style.qrc \
    images.qrc

RC_FILE += logo.rc

