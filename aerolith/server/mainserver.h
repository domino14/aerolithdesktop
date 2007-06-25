
#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include <QtNetwork>
#include <QBuffer>
#include <QHash>
#include <QList>

class MainServer : public QTcpServer
{
Q_OBJECT

public: 
 MainServer();
  

private slots:
 void addConnection();
 void removeConnection();
 void receiveMessage();
 
private:
 struct connectionData
 {
   QBuffer* buffer;
   bool loggedIn;
   QString username;

 };



 QList <QTcpSocket*> connections;
 QHash <QTcpSocket*, connectionData*> connectionParameters;
 QHash <QString, QTcpSocket*> usernamesHash;
 quint16 blockSize;
 void handleMessage(QTcpSocket*, QString);
 bool isValidUsername(QString);
};

#endif
