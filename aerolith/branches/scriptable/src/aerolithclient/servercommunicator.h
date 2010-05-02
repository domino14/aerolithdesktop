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

#ifndef SERVERCOMMUNICATOR_H
#define SERVERCOMMUNICATOR_H

#include <QObject>
#include <QByteArray>
#include <QDataStream>
#include <QTcpSocket>

#include "commonDefs.h"

class ServerCommunicator : public QObject
{
    Q_OBJECT
public:
    enum ConnectionModes { MODE_LOGIN, MODE_REGISTER};
    ServerCommunicator(QObject*);
    void sendChatAll(QString);

    bool isConnectedToServer();
    void connectToServer(QString server, int port, QString _username, QString _password, ConnectionModes);
    void disconnectFromServer();

private:

    QByteArray block;
    QDataStream out;
    QTcpSocket *commsSocket;
    quint16 blockSize;
    void handleTableCommand();
    void handleWordlistsMessage();
    void processHighScores();
    QDataStream in;

    QString username, password;




    ConnectionModes connectionMode;


signals:
    void badMagicNumber();
    void serverDisconnect();
    void serverConnect();
    void serverConnectionError(QString);

    void userLoggedIn(QString);
    void userLoggedOut(QString);
    void errorFromServer(QString);
    void chatReceived(QString, QString);
    void pmReceived(QString, QString);
    void newTableInfoReceived(quint16, quint8, QString, QString, quint8, bool);
    void playerJoinedTable(quint16, QString);
    void tablePrivacyChange(quint16, bool);
    void receivedTableInvite(quint16, QString);
    void bootedFromTable(quint16, QString);
    void playerLeftTable(quint16, QString);
    void tableDeleted(quint16);
    void gotServerMessage(QString);
    void clearAllUnscramblegameListData();
    void addUnscramblegameListData(QString, QStringList);
    void doneUnscramblegameListData();
    void clearUnscramblegameListData(QString, QString);
    void unscramblegameListSpaceUsage(quint32, quint32);
    void dontUnderstandPacket();

    /* in-table signals */
    void serverTableMessage(quint16 tablenum, QString message);
    void serverTableTimerValue(quint16 tablenum, quint16 timerval);
    void serverTableReadyBegin(quint16 tablenum, quint8 seat);
    void serverTableGameStart(quint16 tablenum);
    void serverTableAvatarChange(quint16 tablenum, quint8 seat, quint8 avatarID);
    void serverTableGameEnd(quint16 tablenum);
    void serverTableChat(quint16 tablenum, QString username, QString chat);
    void serverTableHost(quint16 tablenum, QString host);
    void serverTableSuccessfulStand(quint16 tablenum, QString username, quint8 seat);
    void serverTableSuccessfulSit(quint16 tablenum, QString username, quint8 seat);


    void specificTableCommand(QByteArray, quint16, quint8);


    /* other signals */
    void clearHighScoresTable();
    void newHighScore(int rank, QString username, double percentCorrect, int timeRemaining);
    void endHighScoresTable();

    void gotLexicon(QByteArray, int);
    void addWordList(int, QByteArray, char);
private slots:
    void readFromServer();
    void handleError(QAbstractSocket::SocketError socketError);
    void socketDisconnected();
    void socketConnected();

public slots:
    void chatTable(QString textToSend, quint16 tablenum);
    void changeMyAvatar(quint8 avatarID, quint16 tablenum);
    void sendPM(QString username, QString message);
    void joinTable(quint16 tablenum);
    void leaveTable(quint16 tablenum);
    void trySitting(quint8 seat, quint16 tablenum);
    void standUp(quint16 tablenum);
    void trySetTablePrivate(quint16 tablenum, bool priv);
    void sendReady(quint16 tablenum);
    void sendGiveup(quint16 tablenum);
    void sendClientVersion(QString version);
    void requestHighScores(QString challenge);
    void uploadWordList(QString lexicon, QList <quint32>& probIndices, QString listName);
    void requestSavedWordListInfo(QString lexicon);
    void invitePlayerToTable(quint16 tablenum, QString playerToInvite);
    void bootFromTable(quint16 tablenum, QString playerToBoot);
    void saveGame(quint16 tablenum);
    void deleteList(QString lexicon, QString listname);
    void sendSuggestionOrBugReport(QString suggestion);
    void sendPacket(QByteArray unprocessedPacket);

};


struct tempHighScoresStruct
{
    QString username;
    quint16 numCorrect;
    quint16 timeRemaining;
    tempHighScoresStruct(QString username, quint16 numCorrect, quint16 timeRemaining)
    {
        this->username = username;
        this->numCorrect = numCorrect;
        this->timeRemaining = timeRemaining;
    }

};

bool highScoresLessThan(const tempHighScoresStruct& a, const tempHighScoresStruct& b);


#endif // SERVERCOMMUNICATOR_H
