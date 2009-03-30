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

#ifndef _TABLE_GAME_H_
#define _TABLE_GAME_H_

#include <QtCore>
#include "table.h"
#include "ClientWriter.h"
// this class should not be instantiated!
#include "ClientSocket.h"

class Table;

class TableGame : public QObject
{

Q_OBJECT

 public:

  // change to initialize (QList <QVariant>)
  TableGame(Table*);
  virtual ~TableGame();
  

  virtual void initialize(quint8, quint8, QString, QString) = 0;
  virtual void gameStartRequest(ClientSocket*) = 0;
  virtual void guessSent(ClientSocket*, QString) = 0;
  virtual void gameEndRequest(ClientSocket*) = 0;
  virtual void playerJoined(ClientSocket*) = 0;

  // the most generic game packets. the more specific ones go in the specific game classes (like UnscrambleGame gets sendGuessRightPacket)

  void sendReadyBeginPacket(QString);
  void sendGameStartPacket();
  void sendGameEndPacket();
  
  virtual void playerLeftGame(ClientSocket*) = 0;
  

 protected:
  Table* table;

};

#endif
