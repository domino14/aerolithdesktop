TEMPLATE = app
CONFIG += qt
QT += network \
    sql \
    opengl
TARGET = aerolith
QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.5.sdk
CONFIG += x86 \
    ppc
ROOT = ../..
DESTDIR = $$ROOT/
MOC_DIR = build/moc
OBJECTS_DIR = build/obj
INCLUDEPATH += build/moc \
    $$ROOT/src/aerolithclient \
    $$ROOT/src/aerolithServer \
    $$ROOT/src/common
DEPENDPATH += build/moc \
    $$ROOT/src/aerolithclient \
    $$ROOT/src/aerolithServer \
    $$ROOT/src/common
ICON = $$ROOT/macosxicns.icns
SOURCES = main.cpp \
    GameTable.cpp \
    BonusGame.cpp \
    databasehandler.cpp \
    ../aerolithclient/wordfilter.cpp \
    ../common/commonDefs.cpp \
    ../aerolithclient/roomglwidget.cpp
FORMS = tableCreateForm.ui \
    solutionsForm.ui \
    scoresForm.ui \
    loginForm.ui \
    tableForm.ui \
    playerInfoForm.ui \
    tableCustomizationForm.ui \
    pmForm.ui \
    getProfileForm.ui \
    setProfileForm.ui \
    mainwindow.ui \
    databaseDialog.ui \
    ../aerolithclient/inviteForm.ui \
    ../aerolithclient/wordFilterForm.ui \
    ../aerolithclient/createTaxesTableForm.ui
SOURCES += mainwindow.cpp \
    UnscrambleGameTable.cpp \
    avatarLabel.cpp \
    tile.cpp \
    chip.cpp \
    wordRectangle.cpp
HEADERS += mainwindow.h \
    UnscrambleGameTable.h \
    avatarLabel.h \
    tile.h \
    chip.h \
    wordRectangle.h \
    playerlistwidget.h \
    GameTable.h \
    BonusGame.h \
    databasehandler.h \
    ../aerolithclient/wordfilter.h \
    ../aerolithclient/roomglwidget.h
RESOURCES = ../../client.qrc
RC_FILE = ../../client.rc
HEADERS += mainserver.h \
    table.h \
    UnscrambleGame.h \
    TableGame.h \
    ClientSocket.h \
    ClientWriter.h \
    serverthread.h \
    dawg.h \
    SavedUnscrambleGame.h
SOURCES += mainserver.cpp \
    table.cpp \
    TableGame.cpp \
    UnscrambleGame.cpp \
    ClientWriter.cpp \
    serverthread.cpp \
    dawg.cpp
HEADERS += commonDefs.h
OTHER_FILES += acknowledgments.txt \
    ../../codingIdeasAndProgress.txt
