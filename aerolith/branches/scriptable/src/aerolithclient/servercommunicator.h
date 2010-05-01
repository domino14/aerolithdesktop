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
    ServerCommunicator(QObject*);
    void sendChatAll(QString);






    void sendPacket(QByteArray unprocessedPacket);
private:

    QByteArray block;
    QDataStream out;
    QTcpSocket *commsSocket;
    quint16 blockSize;
    void handleTableCommand();
    void processHighScores();
    QDataStream in;
signals:
    void badMagicNumber();
    void userLoggedIn(QString);
    void userLoggedOut(QString);
    void errorFromServer(QString);
    void chatReceived(QString, QString);
    void pmReceived(QString, QString);
    void newTableInfoReceived(quint16, quint8, QString, QString, quint8, bool);
    void playerJoinedTable(quint16, QString);
    void serverTablePrivacy(quint16, bool);
    void receivedTableInvite(quint16, QString);
    void bootedFromTable(quint16, QString);
    void playerLeftTable(quint16, QString);
    void tableDeleted(quint16);
    void gotServerMessage(QString);
    void clearAllUnscramblegameListData();
    void addUnscramblegameListData(QString, QStringList);
    void doneUnscramblegameListData();
    void clearUnscramblegameListData(QString, QStringList);
    void unscramblegameListSpaceUsage(quint32, quint32);
    void dontUnderstandPacket();

    void specificTableCommand(QByteArray, quint16, quint8);
private slots:
    void readFromServer();
    void displayError(QAbstractSocket::SocketError socketError);

};

#endif // SERVERCOMMUNICATOR_H
