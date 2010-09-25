#include "wordgridstable.h"

WordgridsTable::WordgridsTable(QObject* parent) : QObject(parent)
{
    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void WordgridsTable::initialize(ClientSocket *socket, quint16 tablenum, int boardSize)
{
    this->tableNum = tablenum;
    this->boardSize = boardSize;
    this->timerModeGame = MODE_BEGINNING;
    gameTimer->start(2000);

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
