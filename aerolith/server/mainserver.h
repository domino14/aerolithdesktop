
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
   QDataStream in;
   bool loggedIn;
   QString username;
   quint16 numBytesInPacket;
   quint16 numBytesReceivedSoFar;
 };

 enum packetHeaderStatesEnum
   {
     S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT
   };
 
 QList <QTcpSocket*> connections;
 QHash <QTcpSocket*, connectionData*> connectionParameters;
 QHash <QString, QTcpSocket*> usernamesHash;
 quint16 blockSize;


 void processLogin(QTcpSocket*, connectionData*);
 void processGameGuess(QTcpSocket*, connectionData*);
 void processChat(QTcpSocket*, connectionData*);
 void writeToClient(QTcpSocket*, QString, packetHeaderStatesEnum);
 bool isValidUsername(QString);
};

#endif
