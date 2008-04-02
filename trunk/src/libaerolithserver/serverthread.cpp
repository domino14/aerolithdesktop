#include "serverthread.h"

ServerThread::ServerThread(QString version) : mainServer(version)
{
  shouldQuitThread = false;
}

void ServerThread::startThread()
{
  shouldQuitThread = true;
  while (isRunning());

  start();
}

void ServerThread::stopThread()
{
  shouldQuitThread = true;
  while (isRunning());
  
}

void ServerThread::run()
{
  mainServer.listen(QHostAddress::Any, 1988);
  while (!shouldQuitThread);

  mainServer.close();
}
