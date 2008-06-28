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

#include "ui_getProfileForm.h"
#include "ui_setProfileForm.h"

#include "UnscrambleGameTable.h"

class PMWidget : public QWidget
{
	Q_OBJECT
public:
	PMWidget (QWidget *parent, QString senderUsername, QString receiverUsername);
	void appendText(QString);

private:
	Ui::pmForm uiPm;
	QString senderUsername;
	QString receiverUsername;
signals:
	void sendPM(QString user, QString text);
	private slots:
		void readAndSendLEContents();
};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QString);
private:
	QString aerolithVersion;

	
	QTcpSocket *commsSocket;

	quint16 blockSize; // used for socket

	void processServerString(QString);

	QString currentUsername;
	QDataStream in;
	quint16 currentTablenum;

	void handleCreateTable(quint16 tablenum, QString wordListDescriptor, quint8 maxPlayers);
	void handleDeleteTable(quint16 tablenum);
	void handleAddToTable(quint16 tablenum, QString player);
	void handleLeaveTable(quint16 tablenum, QString player);
	void handleTableCommand(quint16 tablenum, quint8 commandByte);
	
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

	QMenu *challengesMenu;

	Ui::MainWindow uiMainWindow;
	Ui::tableCreateForm uiTable;
	Ui::scoresForm uiScores;
	Ui::loginForm uiLogin;

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

	struct tableRepresenter
	{
	  QTableWidgetItem* tableNumItem;
	  QTableWidgetItem* descriptorItem;
	  QTableWidgetItem* maxPlayersItem;
	  QTableWidgetItem* playersItem;
	  QTableWidgetItem* numPlayersItem;
	  QPushButton* buttonItem;
	  quint16 tableNum;
	  QStringList playerList;
	  
	};

	QHash <quint16, tableRepresenter*> tables;
 signals:
	void startServerThread();
	void stopServerThread();
	public slots:
		void submitGuess(QString);
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