//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtGui>
#include <QtNetwork>
#include <QtSql>
#include "ui_tableCreateForm.h"

#include "ui_scoresForm.h"
#include "ui_loginForm.h"
#include "ui_pmForm.h"
#include "ui_mainwindow.h"
#include "ui_databaseDialog.h"

#include "ui_getProfileForm.h"
#include "ui_setProfileForm.h"

#include "wordfilter.h"
#include "databasehandler.h"
#include "UnscrambleGameTable.h"

class PMWidget : public QWidget
{
    Q_OBJECT
public:
    PMWidget (QWidget *parent, QString localUsername, QString remoteUsername);
    void appendText(QString);
    QString getRemoteUsername()
    {
        return remoteUsername;
    }
private:
    Ui::pmForm uiPm;
    QString localUsername;
    QString remoteUsername;
    void closeEvent(QCloseEvent*);
signals:
    void sendPM(QString user, QString text);
    void shouldDelete();
        private slots:
    void readAndSendLEContents();
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString, DatabaseHandler*);
private:
    QStringList peopleLoggedIn;
    QString aerolithVersion;
    QString WindowTitle;
    QString currentLexicon;
    QTcpSocket *commsSocket;

    quint16 blockSize; // used for socket

    WordFilter* wordFilter;
    void processServerString(QString);

    QString currentUsername;
    QDataStream in;
    quint16 currentTablenum;
    DatabaseHandler *dbHandler;

    struct LexiconLists
    {
        QString lexicon;
        QStringList regularWordLists;
        QStringList dailyWordLists;
        LexiconLists()
        {
        }
    };

    QList <LexiconLists> lexiconLists;


    void handleCreateTable(quint16 tablenum, quint8 gameType, QString lexiconName, QString wordListDescriptor,
                           quint8 maxPlayers, bool isPrivate);
    void handleDeleteTable(quint16 tablenum);
    void handleAddToTable(quint16 tablenum, QString player);
    void handleLeaveTable(quint16 tablenum, QString player);
    void handleTableCommand(quint16 tablenum, quint8 commandByte);
    void handleWordlistsMessage();



    const int PLAYERLIST_ROLE;
    void writeHeaderData();
    void fixHeaderLength();
    QByteArray block;
    QDataStream out;
    void modifyPlayerLists(quint16 tablenum, QString player, int modification);
    UnscrambleGameTable *gameBoardWidget;

    bool gameStarted;
    QDialog *createTableDialog;
    QDialog *helpDialog;
    QDialog *scoresDialog;
    QDialog *loginDialog;
    QDialog *databaseDialog;

    QMenu *challengesMenu;

    Ui::MainWindow uiMainWindow;
    Ui::tableCreateForm uiTable;
    Ui::scoresForm uiScores;
    Ui::loginForm uiLogin;
    Ui::dialogCreateDatabases uiDatabase;



    QWidget *setProfileWidget;
    QWidget *getProfileWidget;

    Ui::setProfileForm uiSetProfile;
    Ui::getProfileForm uiGetProfile;

    void sendClientVersion();
    void displayHighScores();

    QTimer* gameTimer;

    enum connectionModes { MODE_LOGIN, MODE_REGISTER};

    connectionModes connectionMode;

    void closeEvent(QCloseEvent*);
    void writeWindowSettings();
    void readWindowSettings();

    QHash <QString, PMWidget*> pmWindows;
    QIcon unscrambleGameIcon;
    struct tableRepresenter
    {
        QTableWidgetItem* tableNumItem;
        QTableWidgetItem* descriptorItem;
        QTableWidgetItem* playersItem;
        QTableWidgetItem* typeIcon;
        QPushButton* buttonItem;
        quint16 tableNum;
        QStringList playerList;
        quint8 gameType;    // see commonDefs.h for game types.
        quint8 maxPlayers;
        bool isPrivate;

    };

    QHash <quint16, tableRepresenter*> tables;

    bool savingGameAllowable;
signals:
    void startServerThread();
    void stopServerThread();
        public slots:
    void submitCorrectAnswer(quint8, quint8);
    void chatTable(QString);
    void submitChatLEContents();
    void readFromServer();
    void displayError(QAbstractSocket::SocketError);
    void serverDisconnection();
    void toggleConnectToServer();
    void connectedToServer();

    void sendPM(QString);
    void sendPM(QString, QString);
    void viewProfile(QString);
    void receivedPM(QString, QString);

    void createNewRoom();
    void leaveThisTable();
    void joinTable();
    void giveUpOnThisGame();
    void submitReady();
    void aerolithHelpDialog();
    void updateGameTimer();
    void changeMyAvatar(quint8);
    void dailyChallengeSelected(QAction*);
    void getScores();
    void registerName();

    void showRegisterPage();
    void showLoginPage();
    void startOwnServer();

    void aerolithAcknowledgementsDialog();
    void showAboutQt();

    void serverThreadHasStarted();
    void serverThreadHasFinished();
    void databaseCreated(QString);
    void setCheckbox(QString);
    void repopulateMyListsTable();
 private slots:
    void createDatabasesOKClicked();
    void rebuildDatabaseAction(QAction*);
    void createUnscrambleGameTable();
    void createBonusGameTable();
    void lexiconComboBoxIndexChanged(int);
    void dbDialogEnableClose(bool);
    void spinBoxWordLengthChange(int);
    void standUp();
    void trySitting(quint8 seatNumber);
    void trySetTablePrivate(bool);
    void showInviteDialog();
    void shouldDeletePMWidget();


    void saveGameBA(QByteArray, QString, QString);

    // uiTable auto-slots
    void on_radioButtonProbability_clicked();
    void on_radioButtonOtherLists_clicked();
    void on_radioButtonMyLists_clicked();
    void on_pushButtonImportList_clicked();
    void on_pushButtonDeleteList_clicked();
};


struct tempHighScoresStruct
{
    QString username;
    quint16 numCorrect;
    quint16 timeRemaining;
    tempHighScoresStruct(QString username, quint16 numCorrect, quint16 timeRemaining)
    {
        this->username = username;
        this->numCorrect = numCorrect;
        this->timeRemaining = timeRemaining;
    }

};

bool highScoresLessThan(const tempHighScoresStruct& a, const tempHighScoresStruct& b);
#endif
