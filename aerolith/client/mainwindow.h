#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QtGui>
#include <QtNetwork>
class MainWindow : public QMainWindow
{
  Q_OBJECT


    
public:
  MainWindow();
private:
  QLineEdit *username;
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
  QDataStream in;
  QStackedWidget* gameStackedWidget;
  QTableWidget* roomTable;
  quint16 currentTablenum;
QBrush colorBrushes[9];
	void createTable(quint16 tablenum, QString wordListDescriptor, quint8 maxPlayers);
void deleteTable(quint16 tablenum);
void addToTable(quint16 tablenum, QString player);
	QHash <quint16, quint16> tablenums;
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

};



#endif
