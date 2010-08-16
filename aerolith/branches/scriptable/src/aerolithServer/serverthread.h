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

#ifndef _SERVERTHREAD_H_
#define _SERVERTHREAD_H_

#include <QThread>
#include "mainserver.h"

class ServerThread : public QThread
{
Q_OBJECT
 public:
  ServerThread(QString version);
  MainServer* mainServer;
 protected:
  void run();
  
 private:

  volatile bool shouldQuitThread;
  QString version;
  public slots:
  void stopThread();
  void startThread();
 signals:
  void readyToConnect();
};

#endif
