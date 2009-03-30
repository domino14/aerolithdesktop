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

#ifndef _TABLE_H_
#define _TABLE_H_

#include <QtCore>
#include "ClientSocket.h"
#include "commonDefs.h"

class TableGame;


class Table
{
public:

    enum tablePacketHeaders
    {
        GAME_STARTED, GAME_ENDED, CHAT_SENT, GUESS_RIGHT, TIMER_VALUE, READY_TO_BEGIN, GAVE_UP
            };

    ~Table();
    QByteArray initialize(ClientSocket* tableCreator, quint16 tableNumber, QByteArray tableDescription);
    quint16 tableNumber;
    quint8 lexiconIndex;
    QByteArray tableInformationArray;
    QString tableName;
    QList <ClientSocket*> playerList;
    quint8 maxPlayers;
    bool canJoin;
    TableGame* tableGame;
    ClientSocket *host;

    void sendChatSentPacket(QString, QString);
    void sendGenericPacket();
    void sendTableMessage(QString);
    // anagrams mode?
    quint8 gameType;
    QByteArray letters; // the letters that are showing on the board
    void removePlayerFromTable(ClientSocket*);

};

#endif
