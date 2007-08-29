message(generating makefile from src.pro)
TEMPLATE = app
CONFIG += qt
QT += network sql
TARGET = aerolith
QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
CONFIG+= x86 ppc
ICON = macosxicns.icns

ROOT = ../
DESTDIR = $$ROOT/
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
INCLUDEPATH += build/moc
DEPENDPATH += build/moc

FORMS = tableCreateForm.ui solutionsForm.ui scoresForm.ui loginForm.ui tableJoiner.ui
SOURCES += main.cpp mainwindow.cpp wordsTableWidget.cpp playerInfoWidget.cpp UnscrambleGameTable.cpp
HEADERS += mainwindow.h wordsTableWidget.h playerInfoWidget.h UnscrambleGameTable.h
RESOURCES = ../client.qrc

