message(generating makefile from jtclone.pro)
TEMPLATE = app
CONFIG += qt
QT += network sql

FORMS = tableCreateForm.ui solutionsForm.ui
SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h

TARGET = aerolith


