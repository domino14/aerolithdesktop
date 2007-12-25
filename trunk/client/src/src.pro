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

FORMS = tableCreateForm.ui solutionsForm.ui scoresForm.ui loginForm.ui tableForm.ui playerInfoForm.ui tableCustomizationForm.ui pmForm.ui getProfileForm.ui setProfileForm.ui mainwindow.ui
SOURCES += main.cpp mainwindow.cpp UnscrambleGameTable.cpp avatarLabel.cpp tile.cpp chip.cpp wordRectangle.cpp
HEADERS += mainwindow.h UnscrambleGameTable.h avatarLabel.h tile.h chip.h wordRectangle.h
RESOURCES = ../client.qrc
RC_FILE = ../client.rc

