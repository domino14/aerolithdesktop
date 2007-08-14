
#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include <QtNetwork>
#include <QBuffer>
#include <QHash>
#include <QList>
#include <QtSql>
#include "table.h"
#include "ClientSocket.h"
#include "ClientWriter.h"
#include "TableGame.h"

class MainServer : public QTcpServer
{
Q_OBJECT

public: 
 MainServer();
  

private slots:
 void removeConnection();
 void receiveMessage();
 void pingEveryone();
 void newDailyChallenges();
 // void updateTimer();
 // void updateCountdownTimer();

private:
 QTimer* oneMinutePingTimer;
 QTimer* midnightTimer;
 void incomingConnection(int socketDescriptor); // inherited from QTcpServer
 QSqlDatabase userDb;
 

 enum packetHeaderStatesEnum
   {
     S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT, S_SERVERMESSAGE
     
   };

 QList <ClientSocket*> connections;
 
 // these two hashes are probably needed
 QHash <QString, ClientSocket*> usernamesHash;
 QHash <quint16, tableData*> tables;


 QHash <QString, QString> wordLists;
 QStringList orderedWordLists;

 void sendAvatarChangePacket(ClientSocket *fromSocket, ClientSocket *toSocket, quint8 avatarID);

 void loadWordLists();
 void sendHighScores(ClientSocket*);
 void processLogin(ClientSocket*);
 void processChat(ClientSocket*);
 void processPrivateMessage(ClientSocket*);
 void writeToClient(ClientSocket*, QString, packetHeaderStatesEnum);
 void processNewTable(ClientSocket*);
 void processJoinTable(ClientSocket*);
 void processLeftTable(ClientSocket*);
 void removePlayerFromTable(ClientSocket* socket, quint16 tablenum);
 void processTableCommand(ClientSocket*);
 void processChatAction(ClientSocket*);
 void processVersionNumber(ClientSocket*);
 void processAvatarID(ClientSocket*);
 void registerNewName(ClientSocket*);
 bool isValidUsername(QString);
 bool isValidPassword(QString);

};

#endif
