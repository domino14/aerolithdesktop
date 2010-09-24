#include "wordgridstable.h"

WordgridsTable::WordgridsTable(QObject* parent) : QObject(parent)
{
}

void WordgridsTable::initialize(ClientSocket *socket, quint16 tablenum, int boardSize)
{
    this->tableNum = tablenum;
    this->boardSize = boardSize;
}

void WordgridsTable::removePersonFromTable(ClientSocket* socket)
{
    peopleInTable.removeAll(socket);
}

void WordgridsTable::cleanupBeforeDelete()
{

}

void WordgridsTable::personJoined(ClientSocket * socket)
{
    peopleInTable.append(socket);
    socket->connectionData.tableNum = this->tableNum;
    socket->gameData.score = 0;
}
