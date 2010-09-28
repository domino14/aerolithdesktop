# -------------------------------------------------
# Project created by QtCreator 2009-05-28T00:56:24
# -------------------------------------------------
QT += sql network
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
    wordgridsgraphicsview.cpp \
    servercommunicator.cpp
HEADERS += mainwindow.h \
    tile.h \
    wordstructure.h \
    wordgridsgraphicsview.h \
    servercommunicator.h
FORMS += mainwindow.ui \
    preferences.ui \
    defpopup.ui \
    loginForm.ui
RESOURCES += wordgrids.qrc
OTHER_FILES += changes.txt
