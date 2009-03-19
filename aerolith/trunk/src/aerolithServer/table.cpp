#include "table.h"

#include "UnscrambleGame.h"
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
    switch (gameType)
    {
        case GAME_TYPE_UNSCRAMBLE:
        {
            quint8 unscrambleType, tableTimer;
            in >> lexiconIndex >> unscrambleType >> tableTimer;
            host = tableCreator;
            canJoin = true;
            tableGame = new UnscrambleGame(this);
            tableGame->initialize(unscrambleType, tableTimer, tableName, lexiconIndex);

            // compute retarr
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
