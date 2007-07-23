message(generating makefile from src.pro)
TEMPLATE = app
CONFIG += qt
QT += network sql
TARGET = aerolith

ROOT = ../
DESTDIR = $$ROOT/
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
INCLUDEPATH += build/moc
DEPENDPATH += build/moc

FORMS = tableCreateForm.ui solutionsForm.ui
SOURCES += main.cpp mainwindow.cpp wordsTableWidget.cpp playerInfoWidget.cpp
HEADERS += mainwindow.h wordsTableWidget.h playerInfoWidget.h 


