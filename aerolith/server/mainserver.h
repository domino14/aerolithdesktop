#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include <QtNetwork>
#include <QBuffer>
#include <QHash>
#include <QList>
#include <QtSql>
#include "table.h"


class MainServer : public QTcpServer
{
Q_OBJECT

public: 
 MainServer();
  

private slots:
 void addConnection();
 void removeConnection();
 void receiveMessage();
 void updateTimer();
 void updateCountdownTimer();
private:
 QSqlDatabase wordDb;
 
 struct connectionData
 {
   QDataStream in;
   bool loggedIn;
   QString username;
   quint16 numBytesInPacket;
   //   quint16 numBytesReceivedSoFar;
   //   quint16 tablenum; // currently in this table!  -- shouldn't be here. connectionData should only be used for actual connection
   // stuff such as packet count, data stream, logged in, etc. tablenum should be in playerdata.
};



 enum packetHeaderStatesEnum
   {
     S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT, S_SERVERMESSAGE
     
   };
 enum tablePacketHeaderStatesEnum
   {
     GAME_STARTED, GAME_ENDED, CHAT_SENT, GUESS_RIGHT, TIMER_VALUE, READY_TO_BEGIN, GAVE_UP
   };
 QList <QTcpSocket*> connections;
 QHash <QTcpSocket*, connectionData*> connectionParameters;
 QHash <QString, QTcpSocket*> usernamesHash;
 QHash <quint16, tableData*> tables;


 QHash <QString, QString> wordLists;
 QStringList orderedWordLists;
 quint16 blockSize;
 quint16 highestTableNumber;

 void writeHeaderData();
 void fixHeaderLength();
 void loadWordLists();

 void processLogin(QTcpSocket*, connectionData*);
 void processChat(QTcpSocket*, connectionData*);
 void processPrivateMessage(QTcpSocket*, connectionData*);
 void writeToClient(QTcpSocket*, QString, packetHeaderStatesEnum);
 void processNewTable(QTcpSocket*, connectionData*);
 void processJoinTable(QTcpSocket*, connectionData*);
 void processLeftTable(QTcpSocket*, connectionData*);
 void removePlayerFromTable(QTcpSocket* socket, connectionData* connData, quint16 tablenum);
 void processTableCommand(QTcpSocket*, connectionData*);
 void prepareTableAlphagrams(tableData*);
 void sendUserCurrentAlphagrams(tableData*, QTcpSocket*);
 void processChatAction(QTcpSocket*, connectionData*);
 void processVersionNumber(QTcpSocket*, connectionData*);
 bool isValidUsername(QString);
 QByteArray block;
 QDataStream out;
 void endGame(tableData*);
 void startGame(tableData*);
 void writeToTable(quint16, QList <QVariant>, tablePacketHeaderStatesEnum);
};

#endif
