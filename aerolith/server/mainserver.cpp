using namespace std;

#include "mainserver.h"
#include "serverthread.h"
#include <iostream>

MainServer::MainServer(QObject *parent) : QTcpServer(parent)
{
  qDebug("mainserver constructor");
  if (!listen(QHostAddress::Any, 1988)) // take me back to 1987
  {
    close();
    return;
  }

}

void MainServer::incomingConnection(int socketDescriptor)
{
  qDebug("incoming connection\n");
  ServerThread *thread = new ServerThread(socketDescriptor, this);
  connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater())); // important!!!
  thread->start();


}
