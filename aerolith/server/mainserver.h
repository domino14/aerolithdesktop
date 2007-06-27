
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
   //   quint16 numBytesReceivedSoFar;
   quint16 tablenum; // currently in this table!

 };

 struct tableData
 {
   quint16 tableNumber;
   QString wordListDescriptor;
   QStringList playerList;
   quint8 maxPlayers;
   bool canJoin; 

   quint16 totalNumberQuestions;
   //   quint16 indexOfCurrentQuestion; // LAST PLACE in the file that was read from!
   // should be a multiple of 45 typically. i.e. read 45 lines, so the index of 
   // curq should be 45 so that it starts reading at line 45 next.
 };

 enum packetHeaderStatesEnum
   {
     S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT
   };
 
 QList <QTcpSocket*> connections;
 QHash <QTcpSocket*, connectionData*> connectionParameters;
 QHash <QString, QTcpSocket*> usernamesHash;
 QHash <quint16, tableData*> tables;
 quint16 blockSize;
 quint16 highestTableNumber;

 void processLogin(QTcpSocket*, connectionData*);
 void processGameGuess(QTcpSocket*, connectionData*);
 void processChat(QTcpSocket*, connectionData*);
 void processPrivateMessage(QTcpSocket*, connectionData*);
 void writeToClient(QTcpSocket*, QString, packetHeaderStatesEnum);
 void processNewTable(QTcpSocket*, connectionData*);
 void processJoinTable(QTcpSocket*, connectionData*);
 void processLeftTable(QTcpSocket*, connectionData*);

 void removePlayerFromTable(QTcpSocket*, quint16 tablenum, QString username);
 bool isValidUsername(QString);
};

#endif
