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

/* this is a table game, on the server side. it is a game that takes place in a table.
   not to be confused with GameTable, which is a graphical table for a generic game*/

#include "TableGame.h"

extern QByteArray block;
extern QDataStream out;


TableGame::~TableGame()
{
  qDebug() << "TableGame destructor";
}

TableGame::TableGame(Table* table)
{
  this->table = table;
  qDebug() << "tablegame constructor";
}

void TableGame::sendReadyBeginPacket(QString username)
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) table->tableNumber;
  out << (quint8)SERVER_TABLE_READY_BEGIN;
  out << username;
  fixHeaderLength();
  table->sendGenericPacket();

}

void TableGame::trySitting(ClientSocket* socket)
{
    if (!socket->connData.isSitting)
    {
        if (table->maxPlayers > table->sittingList.size())
        {
            table->sittingList.append(socket);
            socket->connData.isSitting = true;
            sendSuccessfulSitPacket(socket->connData.userName);
            performSpecificSitActions(socket);
        }


    }
}

void TableGame::tryStanding(ClientSocket* socket)
{
    if (socket->connData.isSitting)
    {
        table->sittingList.removeAll(socket);
        socket->connData.isSitting = false;
        sendSuccessfulStandPacket(socket->connData.userName);
        performSpecificStandActions(socket);

    }

}

void TableGame::sendSuccessfulStandPacket(QString username)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8)SERVER_TABLE_SUCCESSFUL_STAND;
    out << username;
    fixHeaderLength();
    table->sendGenericPacket();
}

void TableGame::sendSuccessfulSitPacket(QString username)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8)SERVER_TABLE_SUCCESSFUL_SIT;
    out << username;
    fixHeaderLength();
    table->sendGenericPacket();
}

void TableGame::sendGameStartPacket()
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) table->tableNumber;
  out << (quint8) SERVER_TABLE_GAME_START;
  fixHeaderLength();
  table->sendGenericPacket();
}

void TableGame::sendGameEndPacket()
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) table->tableNumber;
  out << (quint8) SERVER_TABLE_GAME_END;
  fixHeaderLength();
  table->sendGenericPacket();
}

