
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
 void updateTimer();
 
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
   bool gameStarted;
   QTimer *timer;
   quint16 timerVal;
   quint16 totalNumberQuestions;
   quint8 cycleState; // holds value of cycle radio button
   // if 1, it will cycle at the end of the word list thru all missed words
   // if 0, it will just pick random words every time

   quint8 alphagramState; // holds value of alphagram radio button
   // if 1 it will alphagram automatically
   // if 0 it will shuffle automatically


   //   quint16 indexOfCurrentQuestion; // LAST PLACE in the file that was read from!
   // should be a multiple of 45 typically. i.e. read 45 lines, so the index of 
   // curq should be 45 so that it starts reading at line 45 next.
 };

 enum packetHeaderStatesEnum
   {
     S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT,
     GAME_STARTED, GAME_ENDED, CHAT_SENT, GUESS_RIGHT, TIMER_VALUE
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


 void processLogin(QTcpSocket*, connectionData*);
 void processGameGuess(QTcpSocket*, connectionData*);
 void processChat(QTcpSocket*, connectionData*);
 void processPrivateMessage(QTcpSocket*, connectionData*);
 void writeToClient(QTcpSocket*, QString, packetHeaderStatesEnum);
 void processNewTable(QTcpSocket*, connectionData*);
 void processJoinTable(QTcpSocket*, connectionData*);
 void processLeftTable(QTcpSocket*, connectionData*);
 void removePlayerFromTable(QTcpSocket* socket, connectionData* connData, quint16 tablenum);
 void processTableCommand(QTcpSocket*, connectionData*);
 bool isValidUsername(QString);
 QByteArray block;
 QDataStream out;


 void writeToTable(quint16, QList <QVariant>, packetHeaderStatesEnum);
};

#endif
