//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

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
#include "databasehandler.h"

class MainServer : public QTcpServer
{
    Q_OBJECT

public: 
    MainServer(QString);

private:
    QTimer* oneMinuteTimer;
    QTimer* midnightTimer;
    void incomingConnection(int socketDescriptor); // inherited from QTcpServer
    QSqlDatabase userDb;
    QString aerolithVersion;

    enum packetHeaderStatesEnum
    {
        S_USERLOGGEDIN, S_ERROR, S_USERLOGGEDOUT, S_SERVERMESSAGE

            };

    QList <ClientSocket*> connections;

    // these two hashes are probably needed
    QHash <QString, ClientSocket*> usernamesHash;
    QHash <quint16, Table*> tables;

    QSet <QString> todaysBlacklist;
    QHash <QString, quint32> todaysBandwidthByUser;

    // QHash <QString, QString> wordLists;

    quint64 currentTableID;

    void sendHighScores(ClientSocket*);
    void processLogin(ClientSocket*);
    void processChat(ClientSocket*);
    void processPrivateMessage(ClientSocket*);
    void writeToClient(ClientSocket*, QString, packetHeaderStatesEnum);
    void processNewTable(ClientSocket*);
    void processJoinTable(ClientSocket*);
    void doJoinTable(ClientSocket*, quint16);
    void processLeftTable(ClientSocket*);
    void removePersonFromTable(ClientSocket* socket, quint16 tablenum);
    void processTableCommand(ClientSocket*);
    void processChatAction(ClientSocket*);
    void processVersionNumber(ClientSocket*);
    void registerNewName(ClientSocket*);
    void saveRemoteList(ClientSocket*);
    bool isValidUsername(QString);
    bool isValidPassword(QString);
    void listInfoRequest(ClientSocket*);
    void listDeleteRequest(ClientSocket* socket);
    void suggestionOrBugReport(ClientSocket* socket);
signals:
    void readyToConnect();
    void saveWordList(QString, QString, QString, QList<quint32>);
    void requestListInfo(QString lexicon, QString username);
    void requestListDelete(QString lexicon, QString listname, QString username);
    void otherDatabaseRequest(QByteArray);
public slots:
    void init();
    void deactivate();
    void gotListInfo(QString username, QString lex, QList <QStringList> myListsTableLabels);
    void doneSavingWordList(QString lexicon, QString listName, quint32 listSize, QString username);
    void saveWordListFailed(QString username);
    void deletedList(QString lex, QString list, QString username);
    void deleteListFailed(QString username);
    void requestedListExists(bool exists, quint16 table, quint64 tableid);
    void gotUnscrambleGameQuizArray(QList<quint32>,QList<quint32>,QByteArray,quint16,quint64);
private slots:
    void removeConnection();
    void receiveMessage();
    void checkEveryone();
    void midnightUpkeep();

    // void updateTimer();
    // void updateCountdownTimer();
};

#endif
