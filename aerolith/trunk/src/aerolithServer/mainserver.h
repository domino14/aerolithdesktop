
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
    MainServer(QString);


private slots:
    void removeConnection();
    void receiveMessage();
    void checkEveryone();
    void newDailyChallenges();
    // void updateTimer();
    // void updateCountdownTimer();

private:
    QTimer* oneMinuteTimer;
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

    QString aerolithVersion;

    // QHash <QString, QString> wordLists;



    void sendAvatarChangePacket(ClientSocket *fromSocket, ClientSocket *toSocket, quint8 avatarID);

    void sendHighScores(ClientSocket*);
    void processLogin(ClientSocket*);
    void processChat(ClientSocket*);
    void processPrivateMessage(ClientSocket*);
    void writeToClient(ClientSocket*, QString, packetHeaderStatesEnum);
    void processNewTable(ClientSocket*);
    void processJoinTable(ClientSocket*);
    void doJoinTable(ClientSocket*, quint16);
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
