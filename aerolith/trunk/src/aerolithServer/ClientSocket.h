//    ClientSocket.h
//
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


#ifndef _CLIENT_SOCKET_H_
#define _CLIENT_SOCKET_H_

#include <QtCore>
#include <QtNetwork>

class ClientSocket : public QTcpSocket
{
  Q_OBJECT

public:
  struct ConnectionData
  {
    QDataStream in;
    bool loggedIn;
    QString userName;
    quint16 numBytesInPacket;
    quint16 tableNum; // adding here because table number is pretty essential to the workings of this particular server
    quint8 avatarId;
    bool isActive;
    quint16 minutesInactive;
  };

  // this struct will grow for other games. either we will have a separate struct for other games or all in this struct. this is still better than a hash table lookup and having to delete this struct.
  struct PlayerData
  {
    bool readyToPlay;
    bool gaveUp;
    quint16 score;
  };
  

  
  ConnectionData connData;
  PlayerData playerData;




};

#endif
