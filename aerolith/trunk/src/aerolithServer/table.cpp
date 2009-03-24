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

#include "table.h"

#include "UnscrambleGame.h"
#include "BonusGame.h"
#include "ClientWriter.h"

extern QByteArray block;
extern QDataStream out;
//  tmp->initialize(tablenum, wordListDescriptor, maxPlayers, connData->username);
QByteArray tableData::initialize(ClientSocket* tableCreator, quint16 tableNumber, QByteArray tableDescription)
{


    this->tableNumber = tableNumber;
    QDataStream in(&tableDescription, QIODevice::ReadOnly);
    // TODO possible exploit, if there aren't enough bytes in tableDescription to read all the below.
    // check for crash/instability.

    in >> gameType >> tableName >> maxPlayers;
    qDebug() << "Creating table" << tableNumber << tableName << gameType << maxPlayers << tableDescription.size();
    host = tableCreator;
    canJoin = true;
    switch (gameType)
    {
        case GAME_TYPE_UNSCRAMBLE:
        {
            quint8 unscrambleType, tableTimer;
            in >> lexiconIndex >> unscrambleType >> tableTimer;

            tableGame = new UnscrambleGame(this);
            // TODO FIX. initialize should only take the parameter (in) possibly
            tableGame->initialize(unscrambleType, tableTimer, tableName, lexiconIndex);

            // compute array to be sent out as table information array
            writeHeaderData();
            out << (quint8) SERVER_NEW_TABLE;
            out << (quint16) tableNumber;
            out << (quint8) GAME_TYPE_UNSCRAMBLE;
            out << (quint8) lexiconIndex;
            out << tableName;
            out << maxPlayers;
            fixHeaderLength();

            tableInformationArray = block;
        }
        case GAME_TYPE_BONUS:
        {
            in >> lexiconIndex;
            tableGame = new BonusGame(this);
            tableGame->initialize(0,0, tableName, lexiconIndex);

            writeHeaderData();
            out << (quint8) SERVER_NEW_TABLE;
            out << (quint16) tableNumber;
            out << (quint8) GAME_TYPE_BONUS;
            out << (quint8) lexiconIndex;
            out << tableName;
            out << maxPlayers;
            fixHeaderLength();

            tableInformationArray = block;

        }
        break;



    }

    ////
    return tableInformationArray;


}

tableData::~tableData()
{
    qDebug() << "tableData destructor";
    delete tableGame;
}

void tableData::removePlayerFromTable(ClientSocket* socket)
{
    tableGame->playerLeftGame(socket);
    playerList.removeAll(socket);
    qDebug() << "players in table" << tableNumber << playerList;

}

void tableData::sendGenericPacket()
{
    foreach (ClientSocket* thisSocket, playerList)
        thisSocket->write(block);
}

void tableData::sendChatSentPacket(QString username, QString chat)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8) SERVER_TABLE_CHAT;
    out << username << chat;
    fixHeaderLength();
    sendGenericPacket();
}

void tableData::sendTableMessage(QString message)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8) SERVER_TABLE_MESSAGE;
    out << message;
    fixHeaderLength();
    sendGenericPacket();
}
