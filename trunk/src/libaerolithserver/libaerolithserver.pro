message (generating makefile from server/src.pro)
TEMPLATE = lib
QT += network sql
QT -= gui
CONFIG += console debug
CONFIG -= app_bundle
TARGET = aerolithserver

ROOT = ../..
DESTDIR = $$ROOT/
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
INCLUDEPATH += build/moc
DEPENDPATH += build/moc

HEADERS += mainserver.h table.h UnscrambleGame.h TableGame.h ClientSocket.h ClientWriter.h
SOURCES += main.cpp mainserver.cpp table.cpp TableGame.cpp UnscrambleGame.cpp ClientWriter.cpp
