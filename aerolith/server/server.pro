TEMPLATE = app
HEADERS += mainserver.h table.h unscrambleGameData.h
SOURCES += main.cpp mainserver.cpp table.cpp
QT += network sql
CONFIG += console debug
CONFIG -= app_bundle
