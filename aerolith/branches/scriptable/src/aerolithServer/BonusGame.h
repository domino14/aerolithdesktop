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

#ifndef BONUSGAME_H
#define BONUSGAME_H

#include <QObject>
#include "TableGame.h"

class BonusGame : public TableGame
{
Q_OBJECT

public:

    void initialize(quint8, quint8, QString tableName, QString lexiconName);
    BonusGame(Table*);
    ~BonusGame();

private:

    quint16 currentTimerVal;
    quint16 tableTimerVal;
    quint8 countdownTimerVal;
    QString lexiconName;
    QString tableName;
    bool gameStarted;
    bool countingDown;
    bool startEnabled;

    /* must implement these functions - virtual in TableGame*/
    void gameStartRequest(ClientSocket*){}
    void guessSent(ClientSocket*, QString){}
    void gameEndRequest(ClientSocket*){}
    void playerJoined(ClientSocket*){}
    void playerLeftGame(ClientSocket*){}



};

#endif // BONUSGAME_H
