#include "wordgridstable.h"

int letterDist[26] =
{ 9, 2, 2, 4, 12, 2, 3, 2, 9, 1, 1, 4, 2, 6, 8, 2, 1, 6, 4, 6, 4, 2, 2, 1, 2, 1 };
const int letterDistSum = 98;


WordgridsTable::WordgridsTable(QObject* parent) : QObject(parent)
{
    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void WordgridsTable::initialize(quint16 tablenum, int boardSize,
                                int btTurnoff, GameType gameType)
{
    this->tableNum = tablenum;
    this->boardSize = boardSize;
    this->timerModeGame = MODE_BEGINNING;
    gameTimer->start(1000);
    curTimerValue = 20;
    this->btTurnoff = btTurnoff;
    this->gameType = gameType;

    if (gameType == GAME_STRUCK) gameTimerValue = boardSize*boardSize*2.5;
    else if (gameType == GAME_DASH) gameTimerValue = boardSize*boardSize*4;
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
            startGame();
        }
        else if (timerModeGame == MODE_BREAK)
        {
            timerModeGame = MODE_INGAME;
            curTimerValue = gameTimerValue;
            startGame();
        }
        else if (timerModeGame == MODE_INGAME)
        {
            sendMessageToTable("GAMEOVER " + QByteArray::number(tableNum) + "\n");
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

void WordgridsTable::startGame()
{
    foreach (ClientSocket* socket, peopleInTable)
    {
        socket->gameData.score = 0;
        writeScoreToAll(socket);
    }
    movesHash.clear();
    generateAndSendNewBoard();
}

void WordgridsTable::generateAndSendNewBoard()
{
    curBoard = "";
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            int letter = qrand()%letterDistSum;
            int accum = 0;
            int lettercounter;
            for (lettercounter = 0; lettercounter < 26; lettercounter++)
            {
                accum += letterDist[lettercounter];
                if (letter < accum) break;
            }
//            /* handle special Q case */
//            if ((char)lettercounter + 'A' == 'Q')
//            {
//                thisRoundLetters << "Qu";
//            }
//            else
//            {
//                tile->setTileLetter(QString((char)lettercounter + 'A'));
//                thisRoundLetters << QString((char)lettercounter + 'A');
//            }
            curBoard += ((char)lettercounter + 'A');
        }
    }
    sendMessageToTable("CURBOARD " + QByteArray::number(tableNum) + " " + curBoard + "\n");
}

void WordgridsTable::writeScoreToAll(ClientSocket* socket)
{
    foreach (ClientSocket* connection, peopleInTable)
    {
        connection->write("PLAYERSCORE " + QByteArray::number(tableNum) +
                          " " + QByteArray::number(socket->gameData.score) + " " +
                          socket->connectionData.userName.toAscii() + "\n");
    }
}



void WordgridsTable::personJoined(ClientSocket * socket)
{
    peopleInTable.append(socket);
    socket->connectionData.tableNum = this->tableNum;
    socket->gameData.score = 0;
    if (timerModeGame == MODE_INGAME)
    {
         socket->write("CURBOARD " + QByteArray::number(tableNum) + " " + curBoard + "\n");
         foreach (ClientSocket* connection, peopleInTable)
         {
             socket->write("PLAYERSCORE " + QByteArray::number(tableNum) +
                               " " + QByteArray::number(connection->gameData.score) + " " +
                               connection->connectionData.userName.toAscii() + "\n");
         }
    }
}

void WordgridsTable::processMove(ClientSocket* socket, QList<QByteArray> params)
{
    // acceptpos xl yl xh yh score bonustile
    movesHash[socket].append(params);
    // keep info about player moves?
    socket->gameData.score = params[5].toInt();
    writeScoreToAll(socket);
}
