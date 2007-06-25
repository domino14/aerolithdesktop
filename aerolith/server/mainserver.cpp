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
  connectionData *connData = new connectionData;
  buffer->open(QIODevice::ReadWrite);
  connData->buffer = buffer;
  
  connectionParameters.insert(connection, connData);
  connect(connection, SIGNAL(disconnected()), this, SLOT(removeConnection()));
  connect(connection, SIGNAL(readyRead()), this, SLOT(receiveMessage()));


}

void MainServer::removeConnection()
{
  qDebug("remove connection");
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender()); // sender violates modularity
  // but many signals are connected to this slot
  QString username = usernamesHash.key(socket); // linear time
  if (username != "")
    {
      foreach (QTcpSocket* connection, connections)
	{
	  connection->write("LOGOUT " + username.toAscii() + "\n");
	  qDebug() << "wrote logout " + username.toAscii();
	}
      usernamesHash.take(username);
    }
  connectionData *connData = connectionParameters.take(socket);
  connData->buffer->close();
  connData->buffer->deleteLater();
  connections.removeAll(socket);
  socket->deleteLater();
  delete connData;
  qDebug("connection removed");
}

void MainServer::receiveMessage()
{
  // this seems like it'll be balls slow, but it'll do for now
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender());
  QBuffer* buffer = connectionParameters.value(socket)->buffer;

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
      // am i already logged in? this message should be impossible, unless the client is faulty
      // or like telnet, etc.
     


      QString username = message.mid(6, message.length()).simplified();
      // username is trying to log in
      // first check to see if the username is in the right format
      if (!isValidUsername(username))
	{
	  socket->write("ERROR That username is invalid. Please try another one.\n");
	  socket->disconnectFromHost();
	  return;
	}
      //
      
      if (usernamesHash.find(username) != usernamesHash.end())
	{
	  socket->write("ERROR That username is already in use. Please select another one.\n");
	  socket->disconnectFromHost();
	  return;
	}
      
      // got here with no error
      usernamesHash.insert(username, socket);
      
      foreach (QTcpSocket* connection, connections)
	connection->write("LOGIN " + username.toAscii() + "\n");
      
      foreach (QTcpSocket* connection, connections)
	if (connection != socket) socket->write("LOGIN " + usernamesHash.key(connection).toAscii() + "\n");
    }
  
  if (message.indexOf("CHAT ") == 0)
    {

    }

  if (message.indexOf("PM ") == 0)
    {

    }

  if (message.indexOf(""));


}

bool MainServer::isValidUsername(QString username)
{
  if (username.length() > 16) return false;
  if (!username.at(0).isLetter()) return false;
  for (int i = 1; i < username.length(); i++)
    if (!username.at(i).isLetterOrNumber()) return false;
  return true;
}
