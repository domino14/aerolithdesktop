TEMPLATE = app
CONFIG += qt
QT += network sql
TARGET = aerolith
QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.5.sdk
CONFIG+= x86 ppc


ROOT = ../..
DESTDIR = $$ROOT/
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
INCLUDEPATH += build/moc $$ROOT/src/libaerolithclient $$ROOT/src/libaerolithserver
DEPENDPATH += build/moc

macx {
TARGET = Aerolith
ICON = $$ROOT/macosxicns.icns
}

unix {
LIBS = -laerolithclient -laerolithserver -L$$ROOT
}

win32 {
LIBS = -laerolithclient -laerolithserver -L$$ROOT
}

SOURCES = main.cpp

