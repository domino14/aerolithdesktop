#include "table.h"



//  tmp->initialize(tablenum, wordListDescriptor, maxPlayers, connData->username);
void tableData::initialize(quint16 tableNumber, QString tableName, quint8 maxPlayers, QString tableCreator, quint8 cycleState, quint8 tableTimer, quint8 gameMode)
{
  this->tableNumber = tableNumber;
  this->tableName = tableName;
  this->maxPlayers = maxPlayers;
  playerList << tableCreator;

  if (maxPlayers == 1) canJoin = false;
  else canJoin = true;
  this->gameMode = gameMode;

 

  // for NOW, default gameMode = unscramble

  if (gameMode == GAMEMODE_UNSCRAMBLE)
    {
      tableGame = new UnscrambleGame();
      tableGame->initialize(cycleState, tableTimer);
    }


  /*
    tempFileExists = false;
    this->cycleState = cycleState;

  timer = new QTimer();
  countdownTimer = new QTimer();
  countdownTimer->setProperty("tablenum", QVariant(tableNumber));
  timer->setProperty("tablenum", QVariant(tableNumber));
  
  gameStarted = false;
  countingDown = false;
  tableTimerVal = (quint16)tableTimer * (quint16)60;
  currentTimerVal = tableTimerVal;
  countdownTimerVal = COUNTDOWN_TIMER_VAL;
  */


}

void tableData::~tableData()
{
  qDebug() << "tableData destructor";
  delete tableGame;
}
