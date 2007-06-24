TEMPLATE = app
CONFIG += qt console
HEADERS += mainserver.h serverthread.h
SOURCES += main.cpp mainserver.cpp serverthread.cpp
QT += network
QT -= gui
