QT += core
QT -= gui

TARGET = dots
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    DotsSimplifier.cpp \
    Helper.cpp \
    DotsException.cpp

HEADERS += \
    DotsSimplifier.h \
    Helper.h \
    DotsException.h

