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
QByteArray Table::initialize(ClientSocket* tableCreator, quint16 tableNumber,
                             DatabaseHandler* dbHandler)
{


    this->tableNumber = tableNumber;

    tableCreator->connData.in >> gameType >> maxPlayers;
    qDebug() << "Creating table" << tableNumber << "type:" << gameType << "with" << maxPlayers << "max players";
    host = tableCreator;
    canJoin = true;
    switch (gameType)
    {
        case GAME_TYPE_UNSCRAMBLE:
        {

            tableGame = new UnscrambleGame(this);

            tableInformationArray =
                    tableGame->initialize(dbHandler);


        }
        case GAME_TYPE_BONUS:
        {
//            in >> lexiconIndex;
//            tableGame = new BonusGame(this);    // TODO FIX lexiconname!
//            tableGame->initialize(0,0, tableName, "", dbHandler);
//
//            writeHeaderData();
//            out << (quint8) SERVER_NEW_TABLE;
//            out << (quint16) tableNumber;
//            out << (quint8) GAME_TYPE_BONUS;
//            out << (quint8) lexiconIndex;
//            out << tableName;
//            out << maxPlayers;
//            fixHeaderLength();
//
//            tableInformationArray = block;

        }
        break;



    }

    ////
    return tableInformationArray;


}

Table::~Table()
{
    qDebug() << "Table destructor";
    delete tableGame;
}

void Table::removePlayerFromTable(ClientSocket* socket)
{
    tableGame->playerLeftGame(socket);
    playerList.removeAll(socket);
    qDebug() << "players in table" << tableNumber << playerList;

}

void Table::sendGenericPacket()
{
    foreach (ClientSocket* thisSocket, playerList)
        thisSocket->write(block);
}

void Table::sendChatSentPacket(QString username, QString chat)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8) SERVER_TABLE_CHAT;
    out << username << chat;
    fixHeaderLength();
    sendGenericPacket();
}

void Table::sendTableMessage(QString message)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8) SERVER_TABLE_MESSAGE;
    out << message;
    fixHeaderLength();
    sendGenericPacket();
}
