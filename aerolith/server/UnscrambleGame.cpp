#include "UnscrambleGame.h"

const quint8 COUNTDOWN_TIMER_VAL = 3;

void UnscrambleGame::initialize(quint8 cycleState, quint8 tableTimer)
{

  tempFileExists = false;
  this->cycleState = cycleState;

  timer = new QTimer();
  timer->setProperty("tablenum", QVariant(tableNumber));  
  
  countdownTimer = new QTimer();
  countdownTimer->setProperty("tablenum", QVariant(tableNumber));
  
  gameStarted = false;
  countingDown = false;
  tableTimerVal = (quint16)tableTimer * (quint16)60;
  currentTimerVal = tableTimerVal;
  countdownTimerVal = COUNTDOWN_TIMER_VAL;

}

void UnscrambleGame::~UnscrambleGame()
{
  qDebug() << "UnscrambleGame destructor";
  timer->deleteLater();
  countdownTimer->deleteLater();


}
