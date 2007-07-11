message(generating makefile from jtclone.pro)
TEMPLATE = app
CONFIG += qt
QT += network

FORMS = tableCreateForm.ui
SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h

TARGET = jtclone


