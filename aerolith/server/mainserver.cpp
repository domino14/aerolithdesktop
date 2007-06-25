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
  qDebug("connection added");
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
  qDebug("remove connection");
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender()); // sender violates modularity
  // but many signals are connected to this slot
  QString username = socket->property("Username").toString();
  foreach (QTcpSocket* connection, connections)
    connection->write("LOGOUT " + username.toAscii() + " has logged out!\n");

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
      handleMessage(socket, line);
    }
  
}

void MainServer::handleMessage(QTcpSocket *socket, QString message)
{
  // every message is a QString
  if (message.indexOf("LOGIN ") == 0)
    {
      QString username = message.mid(6, message.length()-7);
      // username is trying to log in
      // first check to see if the username is in the right format
      if (!isValidUsername(username))
	{
	  socket->write("ERROR That username is invalid. Please try another one.\n");
	  socket->disconnectFromHost();
	  return;
	}
      //
      foreach (QTcpSocket* connection, connections)
	{
	  QVariant possibleUsername = connection->property("Username");
	  if (possibleUsername.isValid())
	    if (possibleUsername.toString() == username)
	      {
		socket->write("ERROR That username is already in use. Please select another one.\n");
		socket->disconnectFromHost();
		return;
	      }
	}
      // got here with no error
      socket->setProperty("Username", QVariant(username));

      foreach (QTcpSocket* connection, connections)
	connection->write("LOGIN " + username.toAscii() + " has logged in!\n");
    }
}

bool MainServer::isValidUsername(QString username)
{
  if (username.length() > 16) return false;
  if (!username.at(0).isLetter()) return false;
  for (int i = 1; i < username.length(); i++)
    if (!username.at(i).isLetterOrNumber()) return false;
  return true;
}
