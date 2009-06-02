# -------------------------------------------------
# Project created by QtCreator 2009-05-28T00:56:24
# -------------------------------------------------
QT += sql
macx {
    TARGET = wordgrids
}
win32 {
    TARGET = ../wordgrids
}
OBJECTS_DIR = build
MOC_DIR = moc

TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    tile.cpp
HEADERS += mainwindow.h \
    tile.h
FORMS += mainwindow.ui
RESOURCES += wordgrids.qrc
