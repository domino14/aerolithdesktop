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

#include "BonusGame.h"
#include "databasehandler.h"

const quint8 COUNTDOWN_TIMER_VAL = 3;

void BonusGame::initialize(quint8, quint8, QString tableName, QString lexiconName)
{
    this->tableName = tableName;
    this->lexiconName = lexiconName;

    gameStarted = false;
    countingDown = false;
    startEnabled = true;
    int tableTimer = 5;
    tableTimerVal = (quint16)tableTimer * (quint16)60;
    currentTimerVal = tableTimerVal;
    countdownTimerVal = COUNTDOWN_TIMER_VAL;

}

BonusGame::BonusGame(Table* table) : TableGame(table)
{
    qDebug() << "BonusGame constructor";
}

BonusGame::~BonusGame()
{
    qDebug() << "UnscrambleGame destructor";

    //	gameTimer->deleteLater();
    //	countdownTimer->deleteLater();

}
