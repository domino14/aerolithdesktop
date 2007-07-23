#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtGui>
#include <QtNetwork>
#include <QtSql>
#include "ui_tableCreateForm.h"
#include "ui_solutionsForm.h"

#include "wordsTableWidget.h"
#include "playerInfoWidget.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT



public:
	MainWindow();
private:
	QLineEdit *username;
	QLineEdit *serverAddress;
	QLineEdit *solutionLE; // this will be submitted (to answers) when pressing enter
	QLineEdit *chatLE; // as will this (to chat)
	QTextEdit *chatText; // this is the chat box
	QTcpSocket *commsSocket;
	QListWidget *peopleConnected;
	quint16 blockSize; // used for socket
	QLabel *connectStatusLabel;
	void processServerString(QString);
	QString currentUsername;
	QPushButton *toggleConnection;
	QPushButton *exitTable;
	QLabel *timerDial;
	QDataStream in;
	QStackedWidget* gameStackedWidget;
	QTableWidget* roomTable;
	wordsTableWidget* wordsWidget;
	quint16 currentTablenum;
	//	QBrush colorBrushes[9];
	QBrush missedColorBrush;
	void handleCreateTable(quint16 tablenum, QString wordListDescriptor, quint8 maxPlayers);
	void handleDeleteTable(quint16 tablenum);
	void handleAddToTable(quint16 tablenum, QString player);
	void handleLeaveTable(quint16 tablenum, QString player);
	void handleTableCommand(quint16 tablenum, quint8 commandByte);
	int findRoomTableRow(quint16 tablenum);

	playerInfoWidget* PlayerInfoWidget;

	const int PLAYERLIST_ROLE;
	void writeHeaderData();
	void fixHeaderLength();
	QByteArray block;
	QDataStream out;
	void modifyPlayerLists(quint16 tablenum, QString player, int modification);
	QGroupBox *gameBoardGroupBox;
	bool gameStarted;
	QDialog *createTableDialog;
	QDialog *solutionsDialog;
	QDialog *helpDialog;

	Ui::tableCreateForm uiTable;
	Ui::solutionsForm uiSolutions;
	QString alphagrammizeString(QString);
	QString shuffleString(QString);
	QSet <QString> rightAnswers;

	QSqlDatabase wordDb;
	public slots:
		void submitSolutionLEContents();
		void submitChatLEContents();
		void readFromServer();
		void displayError(QAbstractSocket::SocketError);
		void serverDisconnection();
		void toggleConnectToServer();
		void writeUsernameToServer();
		void sendPM(QListWidgetItem*);
		void createNewRoom();
		void leaveThisTable();
		void joinTable();
		void alphagrammizeWords();
		void shuffleWords();
		void giveUpOnThisGame();
		void wordsWidgetItemClicked(QTableWidgetItem*);
		void submitReady();
		void aerolithHelpDialog();
};



#endif
