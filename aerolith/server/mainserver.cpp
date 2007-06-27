#include "mainserver.h"
#include <QtDebug>

MainServer::MainServer()
{
  connect(this, SIGNAL(newConnection()), this, SLOT(addConnection()));
  qDebug("mainserver constructor");
  blockSize = 0;
  highestTableNumber = 0;
}

void MainServer::addConnection()
{
  qDebug("connection added");
  QTcpSocket* connection = nextPendingConnection();
  connections.append(connection);
  connectionData *connData = new connectionData;
  connData->loggedIn = false;
  connData->numBytesInPacket = 0;
  connData->in.setDevice(connection);
  connData->in.setVersion(QDataStream::Qt_4_2);
  connData->tablenum = 0; // bad table num
  connectionParameters.insert(connection, connData);
  connect(connection, SIGNAL(disconnected()), this, SLOT(removeConnection()));
  connect(connection, SIGNAL(readyRead()), this, SLOT(receiveMessage()));


}

void MainServer::removeConnection()
{
  qDebug("remove connection");
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender()); // sender violates modularity
  // but many signals are connected to this slot
  QString username = connectionParameters.value(socket)->username;
  if (username != "")
    {
      if (connectionParameters.value(socket)->tablenum != 0)
	removePlayerFromTable(socket, connectionParameters.value(socket)->tablenum, 
			      connectionParameters.value(socket)->username);
      
      foreach (QTcpSocket* connection, connections)
	{
	  writeToClient(connection, username, S_USERLOGGEDOUT);
	}
      

      usernamesHash.take(username);
    }
  connectionData *connData = connectionParameters.take(socket);

  
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
	case 'p':
	  // private message
	  processPrivateMessage(socket, connData);
	  break;
	case 't':
	  // created a new table
	  processNewTable(socket, connData);
	  break;
	case 'i':
	  //table information! 
	  //this packet should contain the entire word list as calculated by the client
	  // since this could be fairly large, put a limit of 5000 on it
	  // 5000 quint16s is 10000 bytes, which is pretty fast
	  // the quint16s will be indexes

	  // will write this into a temporary file, named
	  // #_qs.tmp
	  // will write errors into a temporary file #_es.tmp    # is the table number.
	  // these files will be deleted when the table is closed!!
	  break;
	case 'j':
	  // joined an existing table
	  processJoinTable(socket, connData);
	  break;
	case 'l':
	  processLeftTable(socket, connData);
	  break;
	}
      
      connData->numBytesInPacket = 0;
    }
}

void MainServer::processLeftTable(QTcpSocket* socket, connectionData* connData)
{
  quint16 tablenum;
  connData->in >> tablenum;
  
  // the table checking/deletion process must also be done on disconnection!!!

  removePlayerFromTable(socket, tablenum, connData->username);

  connData->tablenum = 0;      

}

void MainServer::removePlayerFromTable(QTcpSocket* socket, quint16 tablenum, QString username)
{
  // this functions removes the player from the table
  // additionally, if the table is then empty as a result, it deletes the table!
  if (tables.contains(tablenum))
    {
      tableData *tmp = tables.value(tablenum);
      tmp->playerList.removeAll(username);

        // write to all connections that username has left table  
      QByteArray block;
      QDataStream out(&block, QIODevice::WriteOnly);
      out.setVersion(QDataStream::Qt_4_2);
      out << (quint16)25344;// magic byte
      out << (quint16)0; // length
      
      out << (quint8) 'L';
      out << tablenum;
      out << username;
      
      out.device()->seek(2); // position of length
      out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
      
      foreach (QTcpSocket* connection, connections)
	connection->write(block);      
      

      if (tmp->playerList.size() == 0)
	{
	  tables.remove(tablenum);
	  delete tmp; // delete this table data structure
	  
	  // write to all clients that table has ceased to exist!
	    QByteArray block;
	    QDataStream out(&block, QIODevice::WriteOnly);
	    out.setVersion(QDataStream::Qt_4_2);
	    out << (quint16)25344;// magic byte
	    out << (quint16)0; // length
	    
	    out << (quint8) 'K'; // kill table
	    out << tablenum;
	    
	    out.device()->seek(2); // position of length
	    out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
	    
	    foreach (QTcpSocket* connection, connections)
	      connection->write(block);
	    // now we must delete the temporary files!
	    // TODO temp file system

	}
    } 
  else
    {
      // an error that shouldn't happen.
      writeToClient(socket, "Error leaving table!", S_ERROR);
      return;    
    
    }
}

void MainServer::processNewTable(QTcpSocket* socket, connectionData* connData)
{
  // 

  quint16 tablenum = 0;  
  bool foundFreeNumber = false;

  
  QString wordListDescriptor;
  quint8 maxPlayers;
  connData->in >> wordListDescriptor;
  connData->in >> maxPlayers;
  while (!foundFreeNumber && tablenum < 1000)
    {
      tablenum++;
      foundFreeNumber = tables.contains(tablenum);
    }

  if (!foundFreeNumber) 
    {
      writeToClient(socket, "You cannot create any more tables!", S_ERROR);
      return;
    }
  
  // TODO fix, bad code:
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;// magic byte
  out << (quint16)0; // length
  
  out << (quint8) 'T';
  out << (quint16) tablenum;
  out << wordListDescriptor;
  out << maxPlayers;
  
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
  
  connData->tablenum = tablenum;
  tableData *tmp = new tableData;
  tmp->tableNumber = tablenum;
  tmp->wordListDescriptor = wordListDescriptor;
  tmp->maxPlayers = maxPlayers;
  tmp->playerList << connData->username;
  if (maxPlayers == 1) tmp->canJoin = false;
  else tmp->canJoin = true;
  tables.insert(tablenum, tmp);
  
  QByteArray block2;
  QDataStream out2(&block2, QIODevice::WriteOnly);
  out2.setVersion(QDataStream::Qt_4_2);
  out2 << (quint16)25344;// magic byte
  out2 << (quint16)0; // length
  
  out2 << (quint8) 'J';
  out2 << (quint16) tablenum;
  out2 << connData->username;
  
  out2.device()->seek(2); // position of length
  out2 << (quint16)(block2.size() - sizeof(quint16) - sizeof(quint16));
  
  foreach (QTcpSocket* connection, connections)
  {
    connection->write(block);
    connection->write(block2);
  }
  

}

void MainServer::processJoinTable(QTcpSocket* socket, connectionData* connData)
{
  
  QString username = connData->username;

  quint16 tablenum;
  connData->in >> tablenum;
  

  // check if table exists, and if it does (which it should), if it's full
  if (!tables.contains(tablenum))
    {
      writeToClient(socket, "That table doesn't exist!", S_ERROR);
      return;
    }

  tableData *tmp = tables.value(tablenum);
  
  if (!tmp->canJoin)
    {
      writeToClient(socket, "That table is full! You can not join.", S_ERROR);
      return;
    }

  // got here with no errors, join table!
  
  // does this work?
  tmp->playerList << username;
  if (tmp->playerList.size() == tmp->maxPlayers) tmp->canJoin = false;

  QByteArray block2;
  QDataStream out2(&block2, QIODevice::WriteOnly);
  out2.setVersion(QDataStream::Qt_4_2);
  out2 << (quint16)25344;// magic byte
  out2 << (quint16)0; // length

  out2 << (quint8) 'J';
  out2 << (quint16) tablenum;
  out2 << connData->username;

  out2.device()->seek(2); // position of length
  out2 << (quint16)(block2.size() - sizeof(quint16) - sizeof(quint16));

  foreach (QTcpSocket* connection, connections)
    connection->write(block2);
  


}

void MainServer::processPrivateMessage(QTcpSocket* socket, connectionData* connData)
{
  QString username;
  connData->in >> username;
  QString message;
  connData->in >> message;
  
  if (usernamesHash.find(username) != usernamesHash.end())
    {
      // the username exists
      QTcpSocket* connection = usernamesHash.value(username); // receiver
      QByteArray block;
      QDataStream out(&block, QIODevice::WriteOnly);
      out.setVersion(QDataStream::Qt_4_2);
      out << (quint16)25344;// magic byte
      out << (quint16)0; // length
      
      out << (quint8) 'P';
      out << connData->username; // sender
      out << message; 
      
      out.device()->seek(2); // position of length
      out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
      connection->write(block); 
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

      out << (quint8) 'E';
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
