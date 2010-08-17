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
#include "ui_createTaxesTableForm.h"
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
#include "servercommunicator.h"


#include "ui_inviteForm.h"

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
    MainWindow(QString);
private:
    QStringList peopleLoggedIn;
    QString aerolithVersion;
    QString WindowTitle;
    QString currentLexicon;

    WordFilter* wordFilter;
    void processServerString(QString);

    QString currentUsername;
    quint16 currentTablenum;

    ServerCommunicator *serverCommunicator;

    struct LexiconLists
    {
        QString lexicon;
        QStringList regularWordLists;
        QStringList dailyWordLists;
        QList <QStringList> savedWordLists;
        LexiconLists()
        {
        }
    };

    QHash <QString, LexiconLists> lexiconListsHash;



    void handleTableCommand(quint16 tablenum, quint8 commandByte);
    void handleWordlistsMessage();

    void testFunction();

    void modifyPlayerLists(quint16 tablenum, QString player, int modification);
    UnscrambleGameTable *gameBoardWidget;

    bool gameStarted;
    QDialog *createScrambleTableDialog;
    QDialog *createTaxesTableDialog;
    QDialog *helpDialog;
    QDialog *scoresDialog;
    QDialog *loginDialog;
    QDialog *databaseDialog;

    QMenu *challengesMenu;

    Ui::MainWindow uiMainWindow;
    Ui::tableCreateForm uiCreateScrambleTable;
    Ui::scoresForm uiScores;
    Ui::loginForm uiLogin;
    Ui::dialogCreateDatabases uiDatabase;

    Ui::createTaxesTableForm uiCreateTaxesTable;


    QWidget *setProfileWidget;
    QWidget *getProfileWidget;

    Ui::setProfileForm uiSetProfile;
    Ui::getProfileForm uiGetProfile;

    void sendClientVersion();
    void displayHighScores();

    QTimer* gameTimer;

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

    QStringList existingLocalDBList;

    bool unscramblegameUserlistData_clearHash;  // a hackily-named variable that has a simple function (search in code)

signals:
    void startServer();
    void stopServer();

    void getQuestionData(QByteArray);
    void probIndicesRequest(QStringList, QString, QString);
    void reconnectToDatabases();
    void createLexiconDatabases(QStringList);
public slots:

    void submitChatLEContents();
    void toggleConnectToServer();

    void sendPM(QString);

    void viewProfile(QString);

    void createNewRoom();
    void leaveThisTable();
    void joinTable();
    void giveUpOnThisGame();
    void submitReady();
    void aerolithHelpDialog();
    void updateGameTimer();
    void dailyChallengeSelected(QAction*);
    void getScores();
    void registerName();

    void showRegisterPage();
    void showLoginPage();
    void startOwnServer();

    void aerolithAcknowledgementsDialog();
    void showAboutQt();

    void serverHasStarted();
    void serverHasFinished();
    void databaseCreated(QString);
    void dbDialogEnableClose(bool);

    void setCheckbox(QString);
    void repopulateMyListsTable();

    // from database
    void gotProbIndices(QList <quint32> indices, QString lexicon, QString listName);

    void setProgressMessage(QString);
    void setProgressValue(int);
    void setProgressRange(int, int);
 private slots:
    void sentLogin();

    void createDatabasesOKClicked();
    void rebuildDatabaseAction(QAction*);
    void createUnscrambleGameTable();
    void createBonusGameTable();
    void lexiconComboBoxIndexChanged(QString);

    void spinBoxWordLengthChange(int);
    void standUp();
    void trySitting(quint8 seatNumber);
    void trySetTablePrivate(bool);
    void showInviteDialog();
    void bootFromTable(QString);
    void shouldDeletePMWidget();
    void chatTable(QString);
    void changeMyAvatar(quint8);

    void acceptedInvite();
    void declinedInvite();

    void saveGame();
    void showDonationPage();

    void on_actionSubmitSuggestion_triggered();

    void on_comboBoxGameType_currentIndexChanged(QString);

    /* for server communciator*/

    void badMagicNumber();
    void userLoggedIn(QString);
    void userLoggedOut(QString);
    void chatReceived(QString, QString);
    void errorFromServer(QString);
    void serverConnectionError(QString);
    void serverDisconnected();
    void receivedPM(QString username, QString message);
    void handleCreateTable(quint16 tablenum, quint8 gameType, QString lexiconName, QString wordListDescriptor,
                           quint8 maxPlayers, bool isPrivate);

    void playerJoinedTable(quint16 tablenum, QString playerName);
    void tablePrivacyChange(quint16 tablenum, bool privacy);
    void tableInvite(quint16 tablenum, QString username);
    void bootedFromTable(quint16 tablenum, QString username);
    void playerLeftTable(quint16 tablenum, QString username);
    void handleDeleteTable(quint16 tablenum);

    void gotServerMessage(QString);

    void beginUnscramblegameListData();
    void addUnscramblegameListData(QString, QStringList);
    void doneUnscramblegameListData();
    void clearSingleUnscramblegameListData(QString, QString);
    void     unscramblegameListSpaceUsage(quint32 usage, quint32 max);

    void gettingLexiconAndListInfo();
    void gotLexicon(QByteArray);
    void doneGettingLexAndListInfo();
    void addWordList(QByteArray, QByteArray, char);

    void clearHighScoresTable();
    void newHighScore(int rank, QString username, double percCorrect, int timeRemaining);
    void endHighScores();

    void gotTableChat(quint16, QString, QString);
    void gotTableMessage(quint16, QString);
    void gotServerTableTimerValue(quint16, quint16);
    void gotServerTableReadyBegin(quint16, quint8);
    void gotServerTableGameStart(quint16);
    void gotServerTableAvatarChange(quint16, quint8, quint8);
    void gotServerTableGameEnd(quint16);
    void gotServerTableHost(quint16, QString);
    void gotServerTableSuccessfulStand(quint16, QString, quint8);
    void gotServerTableSuccessfulSit(quint16, QString, quint8);
    void gotServerTableGameEndRequest(quint16 tablenum, QString username);
    void gotSpecificTableCommand(quint16 tablenum, quint8 commandByte, QByteArray ba);
    // uiTable auto-slots
    void on_radioButtonProbability_clicked();
    void on_radioButtonOtherLists_clicked();
    void on_radioButtonMyLists_clicked();
    void on_pushButtonImportList_clicked();
    void on_pushButtonDeleteList_clicked();
};


#endif
