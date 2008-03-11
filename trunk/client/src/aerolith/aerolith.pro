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
INCLUDEPATH += build/moc $$ROOT/src/libaerolith
DEPENDPATH += build/moc

macx {
TARGET = Aerolith
ICON = macosxicns.icns
}

unix {
LIBS = -laerolith -L$$ROOT
}

win32 {
LIBS = -laerolith -L$$ROOT
}

SOURCES = main.cpp

