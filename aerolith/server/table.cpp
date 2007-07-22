#include "table.h"

const quint8 COUNTDOWN_TIMER_VAL = 3;

//  tmp->initialize(tablenum, wordListDescriptor, maxPlayers, connData->username);
void tableData::initialize(quint16 tableNumber, QString wordListDescriptor, quint8 maxPlayers, QString tableCreator, 
			   quint8 cycleState, quint8 tableTimer)
{
  this->tableNumber = tableNumber;
  this->wordListDescriptor = wordListDescriptor;
  this->maxPlayers = maxPlayers;
  playerList << tableCreator;

  if (maxPlayers == 1) canJoin = false;
  else canJoin = true;
  
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

}
