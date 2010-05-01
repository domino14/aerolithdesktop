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
    originalHost = host;
    sittingList.resize(maxPlayers);
    inviteList.insert(host);
    for (int i = 0; i < maxPlayers; i++)
        sittingList[i] = NULL;

    if (maxPlayers == 1) isPrivate = true;
    else isPrivate = false;

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
//    trySitting(host, 0);    // always seat the host when he enters automatically.
    ////
    return tableInformationArray;


}

Table::~Table()
{
    qDebug() << "Table destructor";
    delete tableGame;
}

bool Table::canJoin(ClientSocket* joiner)
{
    if (isPrivate == false) return true;        // anyone can join a public table!
    else if (isPrivate == true)
    {
        if (inviteList.contains(joiner)) return true;
        else return false;
    }

}

void Table::removePersonFromTable(ClientSocket* socket)
{
    tryStanding(socket);    // always stand up before leaving.
    tableGame->playerLeftGame(socket);
    peopleInTable.removeAll(socket);
    inviteList.remove(socket);

    if (socket == host)
    {
        // the host has left. create a new host.
        if (peopleInTable.size() >= 1)
        {
            host = peopleInTable.at(0);
            sendHostChangePacket(host);
        }
    }
}

void Table::cleanupBeforeDelete()
{
    tableGame->cleanupBeforeDelete();
}

void Table::sendHostChangePacket(ClientSocket* host)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8) SERVER_TABLE_HOST;
    out << host->connData.userName;
    fixHeaderLength();
    sendGenericPacket();
}

void Table::sendGenericPacket()
{
    foreach (ClientSocket* thisSocket, peopleInTable)
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

void Table::personJoined(ClientSocket* socket)
{
    peopleInTable << socket;

    socket->connData.tableNum = tableNumber;
    socket->playerData.gaveUp = false;

    foreach (ClientSocket* thisConn, sittingList)
    {
        if (thisConn)
        {
            thisConn->playerData.readyToPlay = false;

            if (thisConn->connData.isSitting)
            {
                writeHeaderData();
                out << (quint8) SERVER_TABLE_COMMAND;
                out << (quint16) tableNumber;
                out << (quint8)SERVER_TABLE_SUCCESSFUL_SIT;
                out << thisConn->connData.userName;
                out << thisConn->connData.seatNumber;
                fixHeaderLength();
                socket->write(block);

                writeHeaderData();
                out << (quint8) SERVER_TABLE_COMMAND;
                out << (quint16)tableNumber;
                out << (quint8) SERVER_TABLE_AVATAR_CHANGE;
                out << thisConn->connData.seatNumber;
                out << thisConn->connData.avatarId;
                fixHeaderLength();
                socket->write(block);


            }
            else
                qDebug() << "THIS ERROR SHOULDN'T BE";
        }
    }

    tableGame->playerJoined(socket);

    if (host)
    {
        writeHeaderData();
        out << (quint8) SERVER_TABLE_COMMAND;
        out << (quint16) tableNumber;
        out << (quint8) SERVER_TABLE_HOST;
        out << host->connData.userName;
        fixHeaderLength();
        socket->write(block);
    }


}

void Table::processAvatarID(ClientSocket* socket, quint8 id)
{
    socket->connData.avatarId = id;
    sendAvatarChangePacket(socket);
}

void Table::sendAvatarChangePacket(ClientSocket *socket)
{
    if (socket->connData.isSitting)
    {
        writeHeaderData();
        out << (quint8) SERVER_TABLE_COMMAND;
        out << (quint16)tableNumber;
        out << (quint8) SERVER_TABLE_AVATAR_CHANGE;
        out << socket->connData.seatNumber;
        out << socket->connData.avatarId;
        fixHeaderLength();
        sendGenericPacket();
    }
}

bool Table::setTablePrivacy(ClientSocket* socket, bool p)
{
    if (socket == host)
    {
        isPrivate = p;
        return true;
    }
    return false;
}


void Table::trySitting(ClientSocket* socket, quint8 seatNumber)
{
    if (!socket->connData.isSitting)
    {
        if (seatNumber < maxPlayers && sittingList.at(seatNumber) == NULL)
        {
            sittingList[seatNumber] = socket;
            socket->connData.isSitting = true;
            socket->connData.seatNumber = seatNumber;
            sendSuccessfulSitPacket(socket->connData.userName, seatNumber);
            tableGame->performSpecificSitActions(socket);

            sendAvatarChangePacket(socket);
            qDebug() << "Sat!";
        }


    }
}

void Table::tryStanding(ClientSocket* socket)
{
    if (socket->connData.isSitting)
    {
        if (socket->connData.seatNumber < maxPlayers)
        {
            sittingList[socket->connData.seatNumber] = NULL;
            socket->connData.isSitting = false;
            sendSuccessfulStandPacket(socket->connData.userName, socket->connData.seatNumber);
            tableGame->performSpecificStandActions(socket);
        }
    }

}

void Table::sendSuccessfulStandPacket(QString username, quint8 previousSeatNumber)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8)SERVER_TABLE_SUCCESSFUL_STAND;
    out << username;
    out << previousSeatNumber;
    fixHeaderLength();
    sendGenericPacket();
}

void Table::sendSuccessfulSitPacket(QString username, quint8 seatNumber)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) tableNumber;
    out << (quint8)SERVER_TABLE_SUCCESSFUL_SIT;
    out << username;
    out << seatNumber;
    fixHeaderLength();
    sendGenericPacket();
}
