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
 // void updateTimer();
 // void updateCountdownTimer();

private:

 void incomingConnection(int socketDescriptor); // inherited from QTcpServer
 QSqlDatabase wordDb;
 

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


 void loadWordLists();

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
 bool isValidUsername(QString);

};

#endif
