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

#include "serverthread.h"


ServerThread::ServerThread(QString version, DatabaseHandler* databaseHandler)
{
  //  shouldQuitThread = false;
  this->version = version;
  this->dbHandler = databaseHandler;
  
}

void ServerThread::startThread()
{
  //  shouldQuitThread = true;
  //while (isRunning());
  exit(); // terminate event loop
  while (isRunning()) ;
  start();
}

void ServerThread::stopThread()
{
  //  shouldQuitThread = true;
  exit(); // terminate event loop
  while (isRunning()) ;
  
}

void ServerThread::run()
{
  mainServer = new MainServer(version, dbHandler);
  
  shouldQuitThread = false;
  mainServer->listen(QHostAddress::Any, DEFAULT_PORT);
  emit readyToConnect();
  exec(); // enter event loop
	
  mainServer->close();
  mainServer->deleteLater();
}
