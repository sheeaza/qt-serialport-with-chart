#-------------------------------------------------
#
# Project created by QtCreator 2016-09-19T09:01:52
#
#-------------------------------------------------

QT       += core gui serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = serial_port
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    chartwindow.cpp

HEADERS  += mainwindow.h \
    chartwindow.h

FORMS    += mainwindow.ui \
    chartwindow.ui

DISTFILES +=

