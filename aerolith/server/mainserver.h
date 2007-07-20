#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include <QtNetwork>
#include <QBuffer>
#include <QHash>
#include <QList>
#include <QtSql>

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

 struct gameData
 {
   // holds data for each word
   QString alphagram;
   quint8 numNotYetSolved;
   quint8 i, j; // indices in table
   QStringList solutions;
 };

 struct playerData
 {
   QString username;
   bool readyToPlay;
   bool gaveUp;
   quint16 score;
   quint16 tablenum;
 };

 struct tableData
 {
   quint16 tableNumber;
   QString wordListDescriptor;
   QStringList playerList;
   quint8 maxPlayers;
   bool canJoin; 
   bool gameStarted;
   bool countingDown;
   QTimer *timer;
   QTimer *countdownTimer;
   quint16 currentTimerVal;
   quint16 tableTimerVal;
   quint8 countdownTimerVal;
   quint16 totalNumberQuestions;
   
   quint8 cycleState; // holds value of cycle radio button
   // if 1, it will cycle at the end of the word list thru all missed words
   // if 0, it will just pick random words every time
   
   quint8 alphagramState; // holds value of alphagram radio button
   // if 1 it will alphagram automatically
   // if 0 it will shuffle automatically
   QHash <QString, QString> gameSolutions; // the KEY (first value) is a solution
   // the VALUE (second) is an alphagram
   QList <gameData> alphagrams;
   //   quint16 indexOfCurrentQuestion; // LAST PLACE in the file that was read from!
   // should be a multiple of 45 typically. i.e. read 45 lines, so the index of 
   // curq should be 45 so that it starts reading at line 45 next.
   QTextStream alphagramReader;
   QFile inFile;

   QFile outFile;
   QTextStream missedFileWriter;
   
   bool tempFileExists;
 };

 enum packetHeaderStatesEnum
   {
     S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT
     
   };
 enum tablePacketHeaderStatesEnum
   {
     GAME_STARTED, GAME_ENDED, CHAT_SENT, GUESS_RIGHT, TIMER_VALUE, READY_TO_BEGIN, GAVE_UP
   };
 QList <QTcpSocket*> connections;
 QHash <QTcpSocket*, connectionData*> connectionParameters;
 QHash <QString, QTcpSocket*> usernamesHash;
 QHash <quint16, tableData*> tables;
 QHash <QString, playerData> playerDataHash;
 QHash <QString, QString> wordLists;
 QStringList orderedWordLists;
 quint16 blockSize;
 quint16 highestTableNumber;

 void writeHeaderData();
 void fixHeaderLength();


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

 bool isValidUsername(QString);
 QByteArray block;
 QDataStream out;
 void endGame(tableData*);
 void startGame(tableData*);
 void writeToTable(quint16, QList <QVariant>, tablePacketHeaderStatesEnum);
};

#endif
