#include "serverthread.h"

ServerThread::ServerThread(QString version)
{
  //  shouldQuitThread = false;
  this->version = version;
  
}

void ServerThread::startThread()
{
  //  shouldQuitThread = true;
  //while (isRunning());
  exit(); // terminate event loop
  while (isRunning());
  start();
}

void ServerThread::stopThread()
{
  //  shouldQuitThread = true;
  exit(); // terminate event loop
  while (isRunning());
  
}

void ServerThread::run()
{
  mainServer = new MainServer(version);
  
  shouldQuitThread = false;
  mainServer->listen(QHostAddress::Any, 1988);
  emit readyToConnect();
  exec(); // enter event loop
	
  mainServer->close();
  mainServer->deleteLater();
}
