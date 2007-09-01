#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtGui>
#include <QtNetwork>
#include <QtSql>
#include "ui_tableCreateForm.h"
#include "ui_solutionsForm.h"
#include "ui_scoresForm.h"
#include "ui_loginForm.h"
#include "ui_tableJoiner.h"
#include "wordsTableWidget.h"
#include "playerInfoWidget.h"

#include "UnscrambleGameTable.h"

class RoomSelectorWidget : public QWidget, public Ui::tableJoinerWidget
{

Q_OBJECT

 public:
  RoomSelectorWidget(QWidget* parent = 0, quint16 tableNum = 0, QString wList = "", quint8 maxPlayers = 0);
 QStringList peopleList; // all people in the table. 
 quint16 tableNum;
 QString wordListDescriptor;
 quint8 maxPlayers;

 public slots: 

};


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
private:
	QWidget* centralWidget;
	QLineEdit* chatLE;
	QTextEdit* chatText;
	QTcpSocket *commsSocket;
	QListWidget *peopleConnected;
	quint16 blockSize; // used for socket

	void processServerString(QString);

	QString currentUsername;
	QDataStream in;
	//	QStackedWidget* gameStackedWidget;
	QTableWidget* roomTable;
	quint16 currentTablenum;
	//	QBrush colorBrushes[9];
	QBrush missedColorBrush;
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

	QList <RoomSelectorWidget*> roomSelectors;
	QHash <quint16, RoomSelectorWidget*> tables;

	bool gameStarted;
	QDialog *createTableDialog;
	QDialog *solutionsDialog;
	QDialog *helpDialog;
	QDialog *scoresDialog;
	QDialog *loginDialog;

	Ui::tableCreateForm uiTable;
	Ui::solutionsForm uiSolutions;
	Ui::scoresForm uiScores;
	Ui::loginForm uiLogin;

	QSet <QString> rightAnswers;

	QSqlDatabase wordDb;
	void populateSolutionsTable();
	void sendClientVersion();
	void displayHighScores();

	QTimer* gameTimer;

	enum connectionModes { MODE_LOGIN, MODE_REGISTER};

	connectionModes connectionMode;
	public slots:
	  void submitGuess(QString);
	void chatTable(QString);
	void submitChatLEContents();
		void readFromServer();
		void displayError(QAbstractSocket::SocketError);
		void serverDisconnection();
		void toggleConnectToServer();
		void connectedToServer();
		void sendPM(QListWidgetItem*);
		void createNewRoom();
		void leaveThisTable();
		void joinTable();
		void giveUpOnThisGame();
		void submitReady();
		void aerolithHelpDialog();
		void updateGameTimer();
		void changeMyAvatar(quint8 avatarID);
		void dailyChallengeSelected(QAction*);
		void getScores();
		void registerName();

		void showRegisterPage();
		void showLoginPage();
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
