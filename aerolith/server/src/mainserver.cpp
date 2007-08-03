
#include "mainserver.h"
#include <QtDebug>
#include <QtSql>
#include "ClientWriter.h"
#include "UnscrambleGame.h"
//QList <QVariant> dummyList;

extern QByteArray block;
extern QDataStream out;
extern const quint16 MAGIC_NUMBER;

const QString incompatibleVersionString = 
"You are using an outdated version of the Aerolith client.<BR>\
Please check <a href=""http://www.aerolith.org/aerolith"">http://www.aerolith.org/aerolith</a> for the new client.";
const QString compatibleButOutdatedVersionString = 
"You are using an outdated version of the Aerolith client. However, this version will work with the current server, but you will be missing new features. If you would like to upgrade, please check <a href=""http://www.aerolith.org/aerolith"">http://www.aerolith.org/aerolith</a> for the new client.";
const QString thisVersion = "0.1.2";

MainServer::MainServer()
{

  // TODO:
  // the connectionData struct thing is probably slower than reimplementing
  // incomingConnection (int socketDescriptor)
  // get rid of this signal, and just reimplement incomingConnection
  // in incomingConnection, create a client* (which subclasses ClientSocket)
  // and has the connData structure WITHIN it
  // then sets the descriptor
  // this will avoid a hash table lookup everytime any client sends a message!!
  //  connect(this, SIGNAL(newConnection()), this, SLOT(addConnection()));
  qDebug("mainserver constructor");
  
  // load lists
  loadWordLists();
 
  QTime midnight(0, 0, 0);
  qsrand(midnight.msecsTo(QTime::currentTime()));
  initializeWriter();
  //  wordDb = QSqlDatabase::addDatabase("QSQLITE");
  //wordDb.setDatabaseName(QDir::homePath() + "/.zyzzyva/lexicons/OWL2+LWL.db");
  //wordDb.open();
  oneMinutePingTimer = new QTimer;
  connect(oneMinutePingTimer, SIGNAL(timeout()), this, SLOT(pingEveryone()));
  oneMinutePingTimer->start(60000);

  midnightTimer = new QTimer;
  connect(midnightTimer, SIGNAL(timeout()), this, SLOT(newDailyChallenges()));

  midnightTimer->start(86400000 + QTime::currentTime().msecsTo(midnight));
  qDebug() << "there are" << 86400000 + QTime::currentTime().msecsTo(midnight) << "msecs to midnight.";
  // but still generate daily challenges right now.
  UnscrambleGame::generateDailyChallenges();
  UnscrambleGame::midnightSwitchoverToggle = true;
}

void MainServer::newDailyChallenges()
{
  midnightTimer->stop();
  midnightTimer->start(86400000);
  UnscrambleGame::generateDailyChallenges();
  
}

void MainServer::loadWordLists()
{
  wordLists.clear();
  orderedWordLists.clear();
  QFile listFile("../listmaker/lists/LISTS");
  QTextStream thisIn;
  thisIn.setDevice(&listFile);

  listFile.open(QIODevice::ReadOnly | QIODevice::Text);

  while (!thisIn.atEnd())
    {
      QString line = thisIn.readLine();
      if (!line.contains("@")) break;
      QStringList tmp = line.split("@");
      wordLists.insert(tmp.at(1), "../listmaker/lists/" + tmp.at(0)); // key is the DESCRIPTIVE name
      // value is the FILENAME
      orderedWordLists << tmp.at(1);
    }

  listFile.close();

}

void MainServer::incomingConnection(int socketDescriptor)
{
  ClientSocket *client = new ClientSocket();
  if (client->setSocketDescriptor(socketDescriptor))
    {
      connections.append(client);
      client->connData.loggedIn = false;
      client->connData.numBytesInPacket = 0;
      client->connData.in.setDevice(client);
      client->connData.in.setVersion(QDataStream::Qt_4_2);
      client->connData.tableNum = 0;
      client->connData.avatarId = 1;
      client->connData.respondedToLastPing = true; // assume we responded to the last ping
      connect(client, SIGNAL(disconnected()), this, SLOT(removeConnection()));
      connect(client, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

    }
  qDebug() << "Incoming connection: " << client->peerAddress();

}

void MainServer::pingEveryone()
{
  if (connections.size() > 0)
    {
      writeHeaderData();      
      out << (quint8) '?'; // keep alive
      fixHeaderLength();

      foreach (ClientSocket* socket, connections)
	{
	  // disconnect if did not respond to last ping
	  if (socket->connData.respondedToLastPing == false)
	    {
	      socket->disconnectFromHost();
	      qDebug() << socket->connData.userName << "lagged out!";
	    }
	  else
	    {
	      socket->write(block);
	      socket->connData.respondedToLastPing = false;
	    }
	}
    }
}

void MainServer::removeConnection()
{
  qDebug("remove connection");
  ClientSocket* socket = static_cast<ClientSocket*> (sender()); // sender violates modularity
  // but many signals are connected to this slot
  //  connectionData* connData = connectionParameters.value(socket);
  QString username = socket->connData.userName;
  if (username != "")
    {
      //      quint16 tablenum = playerDataHash.value(username).tablenum;
      quint16 tableNum = socket->connData.tableNum;
      if (tableNum != 0)
      	{
	  qDebug() << username << " is at table " << tableNum << " so we have to remove him.";
	  removePlayerFromTable(socket, tableNum);
	}
      else
	qDebug() << username << " is not at any tables.";

      foreach (ClientSocket* connection, connections)
	writeToClient(connection, username, S_USERLOGGEDOUT);
      
      usernamesHash.remove(username);
    }
  
  connections.removeAll(socket);
  socket->deleteLater();

  qDebug("connection removed");
  qDebug() << " Number of connections: " << connections.size();
  qDebug() << " Number of tables: " << tables.size();
}

void MainServer::receiveMessage()
{
 
  // this seems like it'll be slow, but it'll do for now
  ClientSocket* socket = static_cast<ClientSocket*> (sender());

  if (socket->bytesAvailable() > 15000)
    {
      socket->disconnectFromHost();
      return;
    }
  
  while (socket->bytesAvailable()> 0)
    {
      //QDataStream needs EVERY byte to be available for reading!
      if (socket->connData.numBytesInPacket == 0)
	{
	  if (socket->bytesAvailable() < 4) 
	    return;
	  
	  quint16 header;
	  quint16 packetlength;
	  
	  // there are 4 bytes available. read them in!
	  socket->connData.in >> header;
	  socket->connData.in >> packetlength;
	  // THIS IS OPEN TO ABUSE.  what if the user sends a packet thats > 2^16 bytes? packetlength will wrap around,
	  // and reader won't read all bytes
	  // or if user sends 20 bytes e.g. and packetlength = 10
	  socket->connData.numBytesInPacket = packetlength;
	  if (header != (quint16)MAGIC_NUMBER) // magic number
	    {
	      qDebug() << " wrong magic number: " << header << " packlength " << packetlength;
	      writeToClient(socket, incompatibleVersionString, S_ERROR);
	      socket->disconnectFromHost();
	      return;
	    }  
	}
      
    
      if (socket->bytesAvailable() < socket->connData.numBytesInPacket)
	return;
      
      // ok, we can now read the WHOLE packet
      // ideally we read the 'case' byte right now, and then call a process function with
      // 'in' (the QDataStream) as a parameter!
      // the process function will set connData->numBytesInPacket to 0 at the end
      quint8 packetType;
      socket->connData.in >> packetType; // this is the case byte!
      qDebug() << "Received from sender " << socket->connData.userName << " packet " << (char)packetType;      
      
      if (socket->connData.numBytesInPacket > 10000) 
	{
	  socket->disconnectFromHost();
	  return;
	}
      switch(packetType)
	{
	case '?':
	  socket->connData.respondedToLastPing = true;
	  break;
	  
	case 'a':
	  processChatAction(socket);
	  break;
	case 'e':
	  // entered
	  processLogin(socket);
	  break;
	  /*    case 'l':
	  // left
	  processLogout(socket, connData);
	  break;*/ // there is no 'left' packet because server writes it.
	case 'c':
	  // chat
	  processChat(socket);
	  break;
	case 'p':
	  // private message
	  processPrivateMessage(socket);
	  break;
	case 't':
	  // created a new table
	  processNewTable(socket);
	  break;
	case 'i':
	  // avatar ID
	  processAvatarID(socket);
	  break;
	  

	  //	case 'i':
	  //table information! 
	  //this packet should contain the entire word list as calculated by the client
	  // since this could be fairly large, put a limit of 5000 on it
	  // 5000 quint16s is 10000 bytes, which is pretty fast
	  // the quint16s will be indexes
	  
	  // will write this into a temporary file, named
	  // #_qs.tmp
	  // will write errors into a temporary file #_es.tmp    # is the table number.
	  // these files will be deleted when the table is closed!!
	  //break;
	case 'v':
	  processVersionNumber(socket);
	  break;
	case 'j':
	  // joined an existing table
	  processJoinTable(socket);
	  break;
	case 'l':
	  processLeftTable(socket);
	  break;
	case '=':
	  processTableCommand(socket);	  
	  break;
	case 'h':
	  sendHighScores(socket);
	  break;

	default:
	  socket->disconnectFromHost(); // possibly a malicious packet
	  return;
	}
      
      socket->connData.numBytesInPacket = 0;      
      
    }
}


void MainServer::sendHighScores(ClientSocket* socket)
{
  // sends high scores for unscramble game
  quint8 wordLengthDesired;

  socket->connData.in >> wordLengthDesired;
  if (wordLengthDesired >= 4 && wordLengthDesired <= 9)
    {
      QList <highScoreData> *tmp = &UnscrambleGame::dailyHighScores[wordLengthDesired - 4];
      if (tmp->size() == 0)
	return;
      
      writeHeaderData();
      out << (quint8) 'H'; // high scores
      out << wordLengthDesired;
      out << tmp->at(0).numSolutions;
      out << (quint16) tmp->size();
      qDebug() << "WL: " << wordLengthDesired << "#Sol" << tmp->at(0).numSolutions << "#players" << tmp->size();
      for (int i = 0; i < tmp->size(); i++)
	{
	  out << tmp->at(i).userName;
	  out << tmp->at(i).numCorrect;
	  out << tmp->at(i).timeRemaining;
	  qDebug() << tmp->at(i).userName << tmp->at(i).numCorrect << tmp->at(i).timeRemaining;
	}

      // 40 bytes per score
      // max ppl allowed = ~1000 or more. 

      fixHeaderLength();
      socket->write(block);
    }
  
}

void MainServer::processAvatarID(ClientSocket* socket)
{
  QString username;
  quint8 avatarID;

  username = socket->connData.userName;
  socket->connData.in >> avatarID;
  socket->connData.avatarId = avatarID;

  // only send avatars to table and only if this user is in a table
  // when players join a table, send avatars as well

  if (socket->connData.tableNum != 0)
    {
      tableData* table = tables.value(socket->connData.tableNum);
      //      table->sendAvatarChangePacket(username, avatarID);
    
      foreach (ClientSocket *thisConn, table->playerList)
	sendAvatarChangePacket(socket, thisConn, socket->connData.avatarId);

    }

}


void MainServer::processChatAction(ClientSocket* socket)
{
  QString username, actionText;
  // i.e. cesar knocks bbstenniz over the head
  username = socket->connData.userName;
  socket->connData.in >> actionText;
  if (actionText.length() > 400) 
    {
      socket->disconnectFromHost();
      return;
    }
  foreach (ClientSocket* connection, connections)
    writeToClient(connection, "* " +username + " " + actionText, S_SERVERMESSAGE);
}

void MainServer::processVersionNumber(ClientSocket* socket)
{
  QString version;
  socket->connData.in >> version;

  if (version != thisVersion)
      writeToClient(socket, compatibleButOutdatedVersionString , S_SERVERMESSAGE);
}

void MainServer::processTableCommand(ClientSocket* socket)
{
  quint16 tablenum;
  socket->connData.in >> tablenum;
  quint8 subcommand;
  socket->connData.in >> subcommand;
  
  tableData* table = tables.value(tablenum);
  
  switch (subcommand)
    {
    case 'b':
      table->tableGame->gameStartRequest(socket);
      
      break;
    case 's':
      // guess from solution box
      {
	QString guess;
	socket->connData.in >> guess;
	table->tableGame->guessSent(socket, guess);
      }
      break;
    case 'c':
      // chat
      {
	QString chat;
	socket->connData.in >> chat;
	if (chat == "/reload" && socket->connData.userName == "cesar") 
	  loadWordLists();
	
	if (chat.length() > 400)
	  {
	    socket->disconnectFromHost();
	    return;
	  }

	qDebug() << " ->" << tablenum << socket->connData.userName << chat;
	table->sendChatSentPacket(socket->connData.userName, chat);
      }
      break;
    case 'u':
      // uncle
      
      // user gave up.
      table->tableGame->gameEndRequest(socket);
      
      break;
    default:
      socket->disconnectFromHost();
    }
  
}

void MainServer::processLeftTable(ClientSocket* socket)
{
  quint16 tablenum;
  socket->connData.in >> tablenum;
  
  // the table checking/deletion process must also be done on disconnection!!!

  removePlayerFromTable(socket,  tablenum);
  

}

void MainServer::removePlayerFromTable(ClientSocket* socket, quint16 tablenum)
{
  QString username = socket->connData.userName;
  if (socket->connData.tableNum != tablenum)
    {
      
      qDebug() << "A SERIOUS ERROR OCCURRED " << username << socket->connData.tableNum << tablenum;

    }
  // this functions removes the player from the table
  // additionally, if the table is then empty as a result, it deletes the table!
  if (tables.contains(tablenum))
    {
      tableData *tmp = tables.value(tablenum);
      tmp->playerList.removeAll(socket);

        // write to all connections that username has left table  
      writeHeaderData();      
      out << (quint8) 'L';
      out << tablenum;
      out << username;
      fixHeaderLength();
      
      foreach (ClientSocket* connection, connections)
	connection->write(block);      
      
      qDebug() << "wrote " << username << " left " << tablenum;
      if (tmp->playerList.size() == 0)
	{
	  qDebug() << " need to kill table " << tablenum;
	  tables.remove(tablenum);
	  delete tmp; // delete this table data structure -- this should also delete the tablegame
	  
	  // write to all clients that table has ceased to exist!
	  writeHeaderData();
	  out << (quint8) 'K'; // kill table
	  out << tablenum;
	  fixHeaderLength();
	  
	  foreach (ClientSocket* connection, connections)
	    {
	      connection->write(block);
	      qDebug() << "wrote that we Killed table " << tablenum << " to " << connection->connData.userName;
	    }
	  // now we must delete the temporary files!
	  // TODO temp file system
	  
	}
      else
	{
	  tmp->canJoin = true;
	  foreach (ClientSocket* thisConn, tmp->playerList)
	    thisConn->playerData.readyToPlay = false;
	  // TODO: actually send out readytoplay false
	}
    } 
  else
    {
      // an error that shouldn't happen.
      writeToClient(socket, "Error leaving table!", S_ERROR);
      return;    
    
    }
  
  socket->connData.tableNum = 0;
  socket->playerData.readyToPlay = false;
  socket->playerData.gaveUp = false;
}

void MainServer::processNewTable(ClientSocket* socket)
{
  // 

  quint16 tablenum = 0;  
  bool foundFreeNumber = false;

  
  QString tableName;
  quint8 maxPlayers;
  socket->connData.in >> tableName;
  socket->connData.in >> maxPlayers;
  
  quint8 cycleState;
  socket->connData.in >> cycleState;
  quint8 tableTimer;
  socket->connData.in >> tableTimer;

  // TODO: possibly more parameters such as game type, etc.
  while (!foundFreeNumber && tablenum < 1000)
    {
      tablenum++;
      foundFreeNumber = !tables.contains(tablenum);
    }

  if (!foundFreeNumber) 
    {
      writeToClient(socket, "You cannot create any more tables!", S_ERROR);
      return;
    }
  
  if (socket->connData.loggedIn == false)
    {
      qDebug() << "new table? wait.. still logging in!";
      return;
    }

  writeHeaderData();
  out << (quint8) 'T';
  out << (quint16) tablenum;
  out << tableName;
  out << maxPlayers;
  fixHeaderLength();

  foreach (ClientSocket* connection, connections)
    connection->write(block);  
  
  socket->connData.tableNum = tablenum;
  socket->playerData.readyToPlay = false;
  socket->playerData.gaveUp = false;

  tableData *tmp = new tableData;
  if (cycleState != 3)
    {
      tmp->initialize(tablenum, tableName, maxPlayers, socket, cycleState, tableTimer, 
		      tableData::GAMEMODE_UNSCRAMBLE, wordLists.value(tableName));
    }
  else
    {
      tmp->initialize(tablenum, tableName, maxPlayers, socket, cycleState, tableTimer, 
		      tableData::GAMEMODE_UNSCRAMBLE, tableName);
    }
  tables.insert(tablenum, tmp);

  writeHeaderData();
  out << (quint8) 'J';
  out << (quint16) tablenum;
  out << socket->connData.userName;
  fixHeaderLength();
  
  foreach (ClientSocket* connection, connections)
    connection->write(block);  
  
  // send avatar change packet to self
  sendAvatarChangePacket(socket, socket, socket->connData.avatarId);

}

void MainServer::processJoinTable(ClientSocket* socket)
{
  
  QString username = socket->connData.userName;

  quint16 tablenum;
  socket->connData.in >> tablenum;
  

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

  if (socket->connData.tableNum != 0)
    {
      writeToClient(socket, "You are already in a table!", S_ERROR);
      return;
    }

  if (socket->connData.loggedIn == false)
    {
      qDebug() << "join? wait.. still logging in!";
      return;
    }

  // got here with no errors, join table!
  
  // does this work?
  tmp->playerList << socket;
  if (tmp->playerList.size() == tmp->maxPlayers) tmp->canJoin = false;

  writeHeaderData();
  out << (quint8) 'J';
  out << (quint16) tablenum;
  out << username;
  fixHeaderLength();
  foreach (ClientSocket* connection, connections)
    connection->write(block);

  // TODO consider writing functions to update playerData and connData appropriately
  socket->connData.tableNum = tablenum;
  //  socket->playerData.readyToPlay = false;
  socket->playerData.gaveUp = false;

  foreach (ClientSocket* thisConn, tmp->playerList)
  {
    thisConn->playerData.readyToPlay = false;
    
   
    // send everyone's avatar TO this socket
    sendAvatarChangePacket(thisConn, socket, thisConn->connData.avatarId);
    // send this guy's avatar to every socket
    if (socket != thisConn) 
      sendAvatarChangePacket(socket, thisConn, socket->connData.avatarId);
  }
  
  tmp->tableGame->playerJoined(socket);
}

void MainServer::sendAvatarChangePacket(ClientSocket *fromSocket, ClientSocket *toSocket, quint8 avatarID)
{
  // this should not be a table command because there are other situations in which avatars can be changed, i.e. chatroom icons, etc. in the future
  writeHeaderData();      
  out << (quint8) 'I';
  out << fromSocket->connData.userName; // sender of message
  out << avatarID; 
  fixHeaderLength();
  toSocket->write(block); 
  qDebug() << "told " << toSocket->connData.userName << "that" << fromSocket->connData.userName << "now has avatar id " << avatarID;
}

void MainServer::processPrivateMessage(ClientSocket* socket)
{
  QString username, message;
  socket->connData.in >> username >> message;
  if (username.length() + message.length() > 400)
    {
      socket->disconnectFromHost();
      return;
    }

  if (usernamesHash.find(username) != usernamesHash.end())
    {
      // the username exists
      ClientSocket* connection = usernamesHash.value(username); // receiver
      writeHeaderData();      
      out << (quint8) 'P';
      out << socket->connData.userName; // sender of message
      out << message; 
      fixHeaderLength();
      connection->write(block); 
    }

  
}

void MainServer::processLogin(ClientSocket* socket)
{
  QString username, password;
  socket->connData.in >> username; // >> password
  qDebug() << "Login: " << username; 
  
  if (socket->connData.loggedIn == true)
    {
      writeToClient(socket, "You are already logged in!", S_ERROR);
      socket->disconnectFromHost();
      return;
    }

  if (!isValidUsername(username))
    {
      writeToClient(socket, "That is an invalid username.", S_ERROR);
      socket->disconnectFromHost();
      return;
    }

  // TODO REGISTRATION SYSTEM LOL
  if (usernamesHash.contains(username))
    {

      writeToClient(socket, "It appears that you were already logged in... Your previous connection has been logged out! Please try again.", S_ERROR);
      usernamesHash.value(username)->disconnectFromHost();
      socket->disconnectFromHost();
      return;
    }
  // got here with no error
  usernamesHash.insert(username, socket);
  socket->connData.loggedIn = true;
  socket->connData.userName = username;
  foreach (ClientSocket* connection, connections)
    {
      // write to each socket that "username" has logged in
      writeToClient(connection, username, S_USERLOGGEDIN);
      if (connection != socket)
	{
	  // write to THIS socket that every other username has logged in.
	  writeToClient(socket, connection->connData.userName, S_USERLOGGEDIN);
	}
    }
  socket->playerData.readyToPlay = false;
  socket->playerData.gaveUp = false;
  socket->playerData.score = 0;
  QList <tableData*> tableList= tables.values();

  foreach(tableData* table, tableList)
    {
      // write new table for every existing table!
      writeHeaderData();      
      out << (quint8) 'T';
      out << table->tableNumber;
      out << table->tableName;
      out << table->maxPlayers;

      // TODO: write more -- time limit, game type?
      fixHeaderLength();
      
      socket->write(block);

      foreach(ClientSocket* thisSocket, table->playerList)
	{
	  writeHeaderData();
	  out << (quint8) 'J';
	  out << (quint16) table->tableNumber;
	  out << thisSocket->connData.userName;
	  fixHeaderLength();	  
	  socket->write(block);
	}

    }
  // send existing lists

  foreach (QString listDescriptor, orderedWordLists)
    {
      writeHeaderData();
      out << (quint8) 'W';
      out << listDescriptor;
      fixHeaderLength();
      socket->write(block);
    }
}

void MainServer::processChat(ClientSocket* socket)
{
  QString username = socket->connData.userName;
  QString chattext;
  socket->connData.in >> chattext;
  

  if (chattext.length() > 400) 
    {
      socket->disconnectFromHost();
      return;
    }
  
  writeHeaderData();  
  out << (quint8) 'C';
  out << username;
  out << chattext;
  fixHeaderLength();
  foreach (ClientSocket* connection, connections)
    connection->write(block);
  
  qDebug() <<" ->" << username << ":" << chattext;

}

void MainServer::writeToClient(ClientSocket* socket, QString parameter, packetHeaderStatesEnum type)
{
  writeHeaderData();
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
    case S_SERVERMESSAGE:
      out << (quint8) 'S';
      out << parameter;
      break;
    }
  fixHeaderLength();
  socket->write(block);

  //  qDebug() << " wrote to " << connectionParameters.value(socket)->username << " that " << parameter << debugstring;
} 



bool MainServer::isValidUsername(QString username)
{
  if (username.length() > 16 || username.length() < 1) return false;
  if (!username.at(0).isLetter()) return false;
  for (int i = 1; i < username.length(); i++)
    if (!username.at(i).isLetterOrNumber()) return false;
  return true;
}
