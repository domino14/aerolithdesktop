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
  QBuffer *buffer;
  quint16 blockSize; // used for socket
  QLabel *connectStatusLabel;

public slots:
  void submitSolutionLEContents();
  void submitChatLEContents();
  void readFromServer();
  void displayError(QAbstractSocket::SocketError);
  void connectToServer();
  void writeUsernameToServer();
};



#endif