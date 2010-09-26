#include "wordgridstable.h"

WordgridsTable::WordgridsTable(QObject* parent) : QObject(parent)
{
    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void WordgridsTable::initialize(ClientSocket *socket, quint16 tablenum, int boardSize, int gameTimerValue,
                                bool allowBonusTiles)
{
    this->tableNum = tablenum;
    this->boardSize = boardSize;
    this->timerModeGame = MODE_BEGINNING;
    gameTimer->start(1000);
    curTimerValue = 20;
    this->gameTimerValue = gameTimerValue;
    this->allowBonusTiles = allowBonusTiles;

}

void WordgridsTable::removePersonFromTable(ClientSocket* socket)
{
    peopleInTable.removeAll(socket);
}

void WordgridsTable::cleanupBeforeDelete()
{

}

void WordgridsTable::timeout()
{
    curTimerValue--;
    if (curTimerValue == 0)
    {
        if (timerModeGame == MODE_BEGINNING)
        {
            timerModeGame = MODE_INGAME;
            curTimerValue = gameTimerValue;
        }
        else if (timerModeGame == MODE_BREAK)
        {
            timerModeGame = MODE_INGAME;
            curTimerValue = gameTimerValue;
        }
        else if (timerModeGame == MODE_INGAME)
        {
            movesHash.clear();
            timerModeGame = MODE_BREAK;
            curTimerValue = 25;
        }
    }
    sendMessageToTable("TIMERVAL " + QByteArray::number(tableNum) + " " + QByteArray::number(curTimerValue) + "\n");
}

void WordgridsTable::sendMessageToTable(QByteArray ba)
{
    foreach (ClientSocket* socket, peopleInTable)
        socket->write(ba);
}



void WordgridsTable::personJoined(ClientSocket * socket)
{
    peopleInTable.append(socket);
    socket->connectionData.tableNum = this->tableNum;
    socket->gameData.score = 0;
}

int WordgridsTable::processMove(ClientSocket* socket, QList<QByteArray> params)
{
    // acceptpos xl yl xh yh score bonustile
    movesHash[socket].append(params);
    // keep info about player moves?
    return params[5].toInt();
}
