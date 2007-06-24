#include "mainserver.h"
#include <QtDebug>

MainServer::MainServer()
{
  connect(this, SIGNAL(newConnection()), this, SLOT(addConnection()));
  qDebug("mainserver constructor");
  blockSize = 0;
}

void MainServer::addConnection()
{
  QTcpSocket* connection = nextPendingConnection();
  connections.append(connection);
  QBuffer* buffer = new QBuffer(this);
  buffer->open(QIODevice::ReadWrite);
  buffers.insert(connection, buffer);
  connect(connection, SIGNAL(disconnected()), this, SLOT(removeConnection()));
  connect(connection, SIGNAL(readyRead()), this, SLOT(receiveMessage()));


}

void MainServer::removeConnection()
{
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender()); // sender violates modularity
  // but many signals are connected to this slot
  QBuffer* buffer = buffers.take(socket);
  buffer->close();
  buffer->deleteLater();
  connections.removeAll(socket);
  socket->deleteLater();
}

void MainServer::receiveMessage()
{
  // this seems like it'll be balls slow, but it'll do for now
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender());
  QBuffer* buffer = buffers.value(socket);

  // missing some checks for returns values for the sake of simplicity                   
  qint64 bytes = buffer->write(socket->readAll());
  // go back as many bytes as we just wrote so that it can be read                       
  buffer->seek(buffer->pos() - bytes);
  // read only full lines, line by line                                                  
  while (buffer->canReadLine())
    {
      QByteArray line = buffer->readLine();
      foreach (QTcpSocket* connection, connections)
	{
	  connection->write(line);
	}
    }
  
}


