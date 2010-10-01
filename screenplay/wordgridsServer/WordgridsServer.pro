# -------------------------------------------------
# Project created by QtCreator 2010-09-20T23:01:25
# -------------------------------------------------
QT += network
QT -= gui
TARGET = WordgridsServer
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    mainserver.cpp \
    wordgridstable.cpp
HEADERS += mainserver.h \
    wordgridstable.h \
    ClientSocket.h
