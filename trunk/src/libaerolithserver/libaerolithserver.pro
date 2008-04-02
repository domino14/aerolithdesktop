message (generating makefile from libaerolithserver.pro)
TEMPLATE = lib
CONFIG += qt
QT += network sql
QT -= gui
CONFIG += console x86 ppc
TARGET = aerolithserver

ROOT = ../..
DESTDIR = $$ROOT/
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
INCLUDEPATH += build/moc
DEPENDPATH += build/moc

HEADERS += mainserver.h table.h UnscrambleGame.h TableGame.h ClientSocket.h ClientWriter.h serverthread.h
SOURCES += mainserver.cpp table.cpp TableGame.cpp UnscrambleGame.cpp ClientWriter.cpp serverthread.cpp
