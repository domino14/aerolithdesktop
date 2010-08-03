# -------------------------------------------------
# Project created by QtCreator 2009-05-28T00:56:24
# -------------------------------------------------
QT += sql
macx { 
    TARGET = wordgrids
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
    CONFIG += x86 \
        ppc
}
win32:TARGET = ../wordgrids
OBJECTS_DIR = build
MOC_DIR = moc
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    tile.cpp \
    wordstructure.cpp \
    wordgridsgraphicsview.cpp
HEADERS += mainwindow.h \
    tile.h \
    wordstructure.h \
    wordgridsgraphicsview.h
FORMS += mainwindow.ui \
    preferences.ui
RESOURCES += wordgrids.qrc
OTHER_FILES += changes.txt
