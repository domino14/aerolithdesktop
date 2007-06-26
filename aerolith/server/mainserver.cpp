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
  connectionData *connData = new connectionData;
  connData->loggedIn = false;
  connData->numBytesInPacket = 0;
  connData->numBytesReceivedSoFar = 0;
  connData->in.setDevice(connection);
  connData->in.setVersion(QDataStream::Qt_4_2);
  
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
	  writeToClient(connection, username, S_USERLOGGEDOUT);
	}
      usernamesHash.take(username);
    }
  connectionData *connData = connectionParameters.take(socket);
  //  connData->buffer->close();
  // connData->buffer->deleteLater();
  connections.removeAll(socket);
  socket->deleteLater();
  delete connData;
  qDebug("connection removed");
  qDebug() << " Number of connections: " << connections.size();
}

void MainServer::receiveMessage()
{
  // this seems like it'll be slow, but it'll do for now
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender());
  connectionData* connData = connectionParameters.value(socket);
  // is this the way it's normally done? hashing? maybe.

  while (socket->bytesAvailable()> 0)
    {
      //QDataStream needs EVERY byte to be available for reading!
      if (connData->numBytesInPacket == 0)
	{
	  if (socket->bytesAvailable() < 4) 
	    return;
	  
	  quint16 header;
	  quint16 packetlength;
	  
	  // there are 4 bytes available. read them in!
	  connData->in >> header;
	  connData->in >> packetlength;
	  if (header != (quint16)25344) // magic number
	    {
	      qDebug() << " wrong magic number: " << header << " packlength " << packetlength;
	      socket->disconnectFromHost();
	      return;
	    }
	  connData->numBytesInPacket = packetlength;
	  
	}
      
      if (socket->bytesAvailable() < connData->numBytesInPacket)
	return;
      
      // ok, we can now read the WHOLE packet
      // ideally we read the 'case' byte right now, and then call a process function with
      // 'in' (the QDataStream) as a parameter!
      // the process function will set connData->numBytesInPacket to 0 at the end
      quint8 packetType;
      connData->in >> packetType; // this is the case byte!
      
      switch(packetType)
	{
	case 'e':
	  // entered
	  processLogin(socket, connData);
	  break;
	  /*    case 'l':
	  // left
	  processLogout(socket, connData);
	  break;*/ // there is no 'left' packet because server writes it.
	case 'c':
	  // chat
	  processChat(socket, connData);
	  break;
	case 'g':
	  // game guess
	  processGameGuess(socket, connData);
	  break;
	}
      
      connData->numBytesInPacket = 0;
    }
}
void MainServer::processLogin(QTcpSocket* socket, connectionData* connData)
{
  QString username;
  connData->in >> username;
  qDebug() << "Login: " << username; 
  
  if (connData->loggedIn == true)
    {
      writeToClient(socket, "You are already logged in!", S_ERROR);
      return;
    }

  if (!isValidUsername(username))
    {
      writeToClient(socket, "That is an invalid username.", S_ERROR);
      socket->disconnectFromHost();
      return;
    }

  if (usernamesHash.find(username) != usernamesHash.end())
    {
      writeToClient(socket, "That username is already in use! Please select another one.", S_ERROR);
      socket->disconnectFromHost();
      return;
    }
  // got here with no error
  usernamesHash.insert(username, socket);
  connData->loggedIn = true;
  connData->username = username;
  foreach (QTcpSocket* connection, connections)
    {
      // write to each socket that "username" has logged in
      writeToClient(connection, username, S_USERLOGGEDIN);


      if (connection != socket)
	{
	  // write to THIS socket that every other username has logged in.
	  writeToClient(socket, connectionParameters.value(connection)->username, S_USERLOGGEDIN);

	}
    }
}

void MainServer::processGameGuess(QTcpSocket* socket, connectionData* connData)
{
  // QString ;
  // connData->in >> username;
  //qDebug() << "Login: " << username;

}


void MainServer::processChat(QTcpSocket* socket, connectionData* connData)
{
  QString username = connData->username;
  QString chattext;
  connData->in >> chattext;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;// magic byte
  out << (quint16)0; // length
  
  out << (quint8) 'C';
  out << username;
  out << chattext;
  
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));

  foreach (QTcpSocket* connection, connections)
    connection->write(block);
  


}


void MainServer::writeToClient(QTcpSocket* socket, QString parameter, packetHeaderStatesEnum type)
{
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;// magic byte
  out << (quint16)0; // length 
  QString debugstring;
  switch (type)
    {
    case S_USERLOGGEDIN:

      out << (quint8) 'L';
      out << parameter;
      debugstring = "logged in.";
      break;
    case S_ERROR:
      out << (quint8) '!';
      out << parameter;
      debugstring = "error.";
      break;
    case S_USERLOGGEDOUT:
      out << (quint8) 'X';
      out << parameter;
      debugstring = "logged out.";
      break;

    }
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
  socket->write(block);

  qDebug() << " wrote to " << connectionParameters.value(socket)->username << " that " << parameter << debugstring;
} 



bool MainServer::isValidUsername(QString username)
{
  if (username.length() > 16) return false;
  if (!username.at(0).isLetter()) return false;
  for (int i = 1; i < username.length(); i++)
    if (!username.at(i).isLetterOrNumber()) return false;
  return true;
}
