
#include "mainserver.h"
#include <QtDebug>
#include <QtSql>
QList <QVariant> dummyList;

const quint16 MAGIC_NUMBER = 25345; 
const QString incompatibleVersionString = 
"You are using an outdated version of the Aerolith client.<BR>\
Please check <a href=""http://www.aerolith.org/aerolith"">http://www.aerolith.org/aerolith</a> for the new client.";
const QString compatibleButOutdatedVersionString = 
"You are using an outdated version of the Aerolith client. However, this version will work with the current server, but you will be missing new features. If you would like to upgrade, please check <a href=""http://www.aerolith.org/aerolith"">http://www.aerolith.org/aerolith</a> for the new client.";
const QString thisVersion = "0.1.2";

MainServer::MainServer() : out(&block, QIODevice::WriteOnly)
{
  connect(this, SIGNAL(newConnection()), this, SLOT(addConnection()));
  qDebug("mainserver constructor");
  blockSize = 0;
  highestTableNumber = 0;

  // load lists
  loadWordLists();
 
  QTime midnight(0, 0, 0);
  qsrand(midnight.msecsTo(QTime::currentTime()));

  //  wordDb = QSqlDatabase::addDatabase("QSQLITE");
  //wordDb.setDatabaseName(QDir::homePath() + "/.zyzzyva/lexicons/OWL2+LWL.db");
  //wordDb.open();

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

void MainServer::writeHeaderData()
{
  out.device()->seek(0);
  block.clear();
  out << (quint16)MAGIC_NUMBER;
  out << (quint16)0; // length    
}

void MainServer::fixHeaderLength()
{
  out.device()->seek(sizeof(quint16));
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));

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
  connectionParameters.insert(connection, connData);
  connect(connection, SIGNAL(disconnected()), this, SLOT(removeConnection()));
  connect(connection, SIGNAL(readyRead()), this, SLOT(receiveMessage()));


}


void MainServer::removeConnection()
{
  qDebug("remove connection");
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender()); // sender violates modularity
  // but many signals are connected to this slot
  connectionData* connData = connectionParameters.value(socket);
  QString username = connData->username;
  if (username != "")
    {
      quint16 tablenum = playerDataHash.value(username).tablenum;
      if (tablenum != 0)
      	{
	  qDebug() << username << " is at table " << tablenum << " so we have to remove him.";
	  removePlayerFromTable(socket, connData, tablenum);
	}
      else
	qDebug() << username << " is not at any tables.";

      foreach (QTcpSocket* connection, connections)
	{
	  writeToClient(connection, username, S_USERLOGGEDOUT);
	}
      

      usernamesHash.remove(username);
    }
  
  connections.removeAll(socket);
  socket->deleteLater();

  connectionParameters.remove(socket);

  playerDataHash.remove(username);

  qDebug("connection removed");
  qDebug() << " Number of connections: " << connections.size();
  qDebug() << " Number of player data structures: " << playerDataHash.size();
  qDebug() << " Number of connection data structures: " << connectionParameters.size();
  qDebug() << " Number of tables: " << tables.size();
}

void MainServer::receiveMessage()
{
 
  // this seems like it'll be slow, but it'll do for now
  QTcpSocket* socket = static_cast<QTcpSocket*> (sender());
  connectionData* connData = connectionParameters.value(socket);
  // is this the way it's normally done? hashing? maybe.
  if (socket->bytesAvailable() > 15000)
    {

      socket->disconnectFromHost();
      return;
    }
  
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
	  // THIS IS OPEN TO ABUSE.  what if the user sends a packet thats > 2^16 bytes? packetlength will wrap around,
	  // and reader won't read all bytes
	  // or if user sends 20 bytes e.g. and packetlength = 10
	  connData->numBytesInPacket = packetlength;
	  if (header != (quint16)MAGIC_NUMBER) // magic number
	    {
	      qDebug() << " wrong magic number: " << header << " packlength " << packetlength;
	      writeToClient(socket, incompatibleVersionString, S_ERROR);
	      socket->disconnectFromHost();
	      return;
	    }  
	}
      
    
      if (socket->bytesAvailable() < connData->numBytesInPacket)
	return;
      
      // ok, we can now read the WHOLE packet
      // ideally we read the 'case' byte right now, and then call a process function with
      // 'in' (the QDataStream) as a parameter!
      // the process function will set connData->numBytesInPacket to 0 at the end
      quint8 packetType;
      connData->in >> packetType; // this is the case byte!
      qDebug() << "Received from sender " << connData->username << " packet " << (char)packetType;      
      
      if (connData->numBytesInPacket > 10000) 
	{
	  socket->disconnectFromHost();
	  return;
	}
      switch(packetType)
	{
	case 'a':
	  processChatAction(socket, connData);
	  break;
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
	case 'v':
	  processVersionNumber(socket, connData);
	  break;
	case 'j':
	  // joined an existing table
	  processJoinTable(socket, connData);
	  break;
	case 'l':
	  processLeftTable(socket, connData);
	  break;
	case '=':
	  processTableCommand(socket, connData);
	  
	  break;
	default:
	  socket->disconnectFromHost(); // possibly a malicious packet
	  return;
	}
      
      connData->numBytesInPacket = 0;      
      
    }
}


void MainServer::processChatAction(QTcpSocket* socket, connectionData* connData)
{
  QString username, actionText;
  // i.e. cesar knocks bbstenniz over the head
  username = connData->username;
  connData->in >> actionText;
  if (actionText.length() > 400) 
    {
      socket->disconnectFromHost();
      return;
    }
  foreach (QTcpSocket* connection, connections)
    writeToClient(connection, "* " +username + " " + actionText, S_SERVERMESSAGE);
}

void MainServer::processVersionNumber(QTcpSocket* socket, connectionData* connData)
{
  QString version;
  connData->in >> version;

  if (version != thisVersion)
    {
      writeToClient(socket, compatibleButOutdatedVersionString , S_SERVERMESSAGE);
    }
}

void MainServer::processTableCommand(QTcpSocket* socket, connectionData* connData)
{
  quint16 tablenum;
  connData->in >> tablenum;

  quint8 subcommand;
  connData->in >> subcommand;
  
  tableData* table = tables.value(tablenum);

  switch (subcommand)
    {
    case 'b':
      if (table->gameStarted == false && table->countingDown == false)
	{

	  bool startTheGame = true;
	  
	  QList <QVariant> tempList;
	  tempList << QVariant(connData->username);
	  writeToTable(tablenum, tempList, READY_TO_BEGIN);
	  playerDataHash[connData->username].readyToPlay = true;
	  
	  foreach (QString player, table->playerList)
	    if (playerDataHash.value(player).readyToPlay == false)
	      {
		startTheGame = false;
		break;
	      }
	  
	  
	  if (startTheGame == true)
	    {
	      table->countingDown = true;
	      table->countdownTimerVal = 3;
	      table->countdownTimer->start(1000);
	      QList <QVariant> tempList;
	      tempList << QVariant(table->countdownTimerVal);
	      writeToTable(table->tableNumber, tempList, TIMER_VALUE);  
	    }
	}
      break;
    case 's':
      // guess from solution box
      {
	QString guess;
	connData->in >> guess;
	//if (guess == "")
	guess = guess.toUpper();
	if (guess == "")
	  {
	    

	  }
	else
	  {
	    if (table->gameStarted)
	      {
		if (table->gameSolutions.contains(guess))
		  {
		    QString alphagram = table->gameSolutions.value(guess);
		    table->gameSolutions.remove(guess);
		    quint8 rowIndex = 0, columnIndex = 0;
		    quint8 indexOfQuestion = table->alphagramIndices.value(alphagram);
		    table->unscrambleGameQuestions[indexOfQuestion].numNotYetSolved--;
		    rowIndex = table->unscrambleGameQuestions[indexOfQuestion].i;
		    columnIndex = table->unscrambleGameQuestions[indexOfQuestion].j;

		    // booo! use a hash table!
		    /*		    for (quint8 i = 0; i < 45; i++)
		      if (table->unscrambleGameQuestions.at(i).alphagram == alphagram)
			{
			  table->unscrambleGameQuestions[i].numNotYetSolved--;
			  rowIndex = table->unscrambleGameQuestions.at(i).i;
			  columnIndex = table->unscrambleGameQuestions.at(i).j;
			  break;
			}
		    */

		    QList <QVariant> tempList;
		    tempList << QVariant(connData->username);
		    tempList << QVariant(guess);
		    tempList << QVariant(rowIndex);
		    tempList << QVariant(columnIndex);
		    writeToTable(tablenum, tempList, GUESS_RIGHT);
		    
		    if (table->gameSolutions.isEmpty()) endGame(table);
		  }
	      }
	  }
	qDebug() << " ->GUESS" << connData->username << guess;
	
      }
      break;
    case 'c':
      // chat
      {
	QString chat;
	connData->in >> chat;
	if (chat == "/reload" && connData->username == "cesar") loadWordLists();
	
	if (chat.length() > 400)
	  {
	    socket->disconnectFromHost();
	    return;
	  }

	QList <QVariant> tempList;
	tempList << QVariant(connData->username);
	tempList << QVariant(chat);
	writeToTable(tablenum, tempList, CHAT_SENT);
	qDebug() << " ->" << tablenum << connData->username << chat;
      }
      break;
    case 'u':
      // uncle
      {
	// user gave up.
	if (table->gameStarted == true)
	  {
	    bool giveUp = true;
	    
	    QList <QVariant> tempList;
	    tempList << QVariant(connData->username);
	    writeToTable(tablenum, tempList, GAVE_UP);
	    playerDataHash[connData->username].gaveUp = true;
	    foreach (QString player, table->playerList)
	      if (playerDataHash.value(player).gaveUp == false)
		{
		  giveUp = false;
		  break;
		}
	    if (giveUp == true) endGame(table);
	    
	  }
	
      }
      break;
    default:
      socket->disconnectFromHost();
    }
}
void MainServer::prepareTableAlphagrams(tableData* table)
{
  // load alphagrams into the gamesolutions hash and the alphagrams list
  

  if (table->cycleState == 0) table->tempFileExists = false;

  if (table->tempFileExists == false)
    {
      table->inFile.close();
      table->outFile.close();
      // generate missed file
      table->outFile.setFileName(QString("missed%1").arg(table->tableNumber));
      table->outFile.open(QIODevice::WriteOnly | QIODevice::Text);
      table->missedFileWriter.setDevice(&(table->outFile));
      table->missedFileWriter.seek(0);


      qDebug() << "before generating temp file";
      QFile inFile(wordLists.value(table->wordListDescriptor));
      QFile outFile(QString("tmp%1").arg(table->tableNumber));
      QTextStream inStream(&inFile);
      QTextStream outStream(&outFile);
      inFile.open(QIODevice::ReadOnly | QIODevice::Text);
      outFile.open(QIODevice::WriteOnly | QIODevice::Text);
      QStringList fileContents;
      while (!inStream.atEnd())
	fileContents << inStream.readLine().trimmed();
      inFile.close();
      int index;
      while (fileContents.size() > 0)
	{
	  index = qrand() % fileContents.size();
	  outStream << fileContents.at(index) << "\n";
	  fileContents.removeAt(index);
	}
      outFile.close();
      table->tempFileExists = true;
      qDebug() << "after generating temp file";
      table->inFile.setFileName(QString("tmp%1").arg(table->tableNumber));
      table->alphagramReader.setDevice(&(table->inFile));
      table->inFile.open(QIODevice::ReadWrite | QIODevice::Text);
      table->alphagramReader.seek(0);
    }

  table->gameSolutions.clear();
  table->unscrambleGameQuestions.clear();
  table->alphagramIndices.clear();
  if (table->cycleState == 1 && table->alphagramReader.atEnd())
    {
      table->inFile.close();
      table->inFile.remove();
      table->outFile.close();
      table->outFile.copy(QString("tmp%1").arg(table->tableNumber));
      table->outFile.remove();

      table->inFile.setFileName(QString("tmp%1").arg(table->tableNumber));
      table->outFile.setFileName(QString("missed%1").arg(table->tableNumber));
      table->alphagramReader.setDevice(&(table->inFile));
      table->missedFileWriter.setDevice(&(table->outFile));
      table->inFile.open(QIODevice::ReadOnly | QIODevice::Text);
      table->outFile.open(QIODevice::WriteOnly | QIODevice::Text);
      table->missedFileWriter.seek(0);
      table->alphagramReader.seek(0);
      // CLOSE table->inFile
      // COPY missed%1 to tmp%1
      // repeat code above to load the file into memory, shuffle, etc

    }


  QStringList lineList;
  QString line;

  for (quint8 i = 0; i < 9; i++)
    for (quint8 j = 0; j < 5; j++)
      {
	unscrambleGameData thisGameData;
	thisGameData.i = i;
	thisGameData.j = j;
	if (table->alphagramReader.atEnd())
	  {
	    thisGameData.alphagram = "";
	    thisGameData.numNotYetSolved = 0;
	  }
	else
	  {
	    line = table->alphagramReader.readLine();
	    lineList = line.split(" ");
	    thisGameData.alphagram = lineList.at(0);
	    thisGameData.numNotYetSolved = (quint8)(lineList.size()-1);
	    for (int k = 1; k < lineList.size(); k++)
	      {
		table->gameSolutions.insert(lineList.at(k), lineList.at(0));
		thisGameData.solutions << lineList.at(k);
	      }
	  }

	table->unscrambleGameQuestions.append(thisGameData);
	table->alphagramIndices.insert(thisGameData.alphagram, i*5 + j);
      }
}

void MainServer::sendUserCurrentAlphagrams(tableData* table, QTcpSocket* socket)
{
  writeHeaderData();      
  out << (quint8) '+';
  out << table->tableNumber;
  out << (quint8) 'W';
 
  for (int i = 0; i < 45; i++)
    {
      out << table->unscrambleGameQuestions.at(i).alphagram;
      out << table->unscrambleGameQuestions.at(i).numNotYetSolved;
      out << table->unscrambleGameQuestions.at(i).solutions;
    }

  fixHeaderLength();  
  socket->write(block);        
}


void MainServer::processLeftTable(QTcpSocket* socket, connectionData* connData)
{
  quint16 tablenum;
  connData->in >> tablenum;
  
  // the table checking/deletion process must also be done on disconnection!!!

  removePlayerFromTable(socket, connData, tablenum);
  

}

void MainServer::removePlayerFromTable(QTcpSocket* socket, connectionData* connData, quint16 tablenum)
{
  QString username = connData->username;
  if (playerDataHash.value(username).tablenum != tablenum)
    {
      
      qDebug() << "A SERIOUS ERROR OCCURRED " << username << playerDataHash.value(username).tablenum << tablenum;

    }
  // this functions removes the player from the table
  // additionally, if the table is then empty as a result, it deletes the table!
  if (tables.contains(tablenum))
    {
      tableData *tmp = tables.value(tablenum);
      tmp->playerList.removeAll(username);

        // write to all connections that username has left table  
      writeHeaderData();      
      out << (quint8) 'L';
      out << tablenum;
      out << username;
      fixHeaderLength();
      
      foreach (QTcpSocket* connection, connections)
	connection->write(block);      
      
      qDebug() << "wrote " << username << " left " << tablenum;
      if (tmp->playerList.size() == 0)
	{
	  qDebug() << " need to kill table " << tablenum;
	  tables.remove(tablenum);
	  tmp->timer->deleteLater();
	  tmp->countdownTimer->deleteLater();
	  delete tmp; // delete this table data structure
	  
	  // write to all clients that table has ceased to exist!
	  writeHeaderData();
	  out << (quint8) 'K'; // kill table
	  out << tablenum;
	  fixHeaderLength();
	  
	  foreach (QTcpSocket* connection, connections)
	    {
	      connection->write(block);
	      qDebug() << "wrote that we Killed table " << tablenum << " to " << connectionParameters.value(connection)->username;
	    }
	  // now we must delete the temporary files!
	  // TODO temp file system
	  
	}
      else
	{
	  tmp->canJoin = true;
	  foreach (QString thisname, tmp->playerList)
	    playerDataHash[thisname].readyToPlay = false;
	}
    } 
  else
    {
      // an error that shouldn't happen.
      writeToClient(socket, "Error leaving table!", S_ERROR);
      return;    
    
    }

  playerDataHash[username].tablenum = 0;
  playerDataHash[username].readyToPlay = false;
  playerDataHash[username].gaveUp = false;
}

void MainServer::processNewTable(QTcpSocket* socket, connectionData* connData)
{
  // 

  quint16 tablenum = 0;  
  bool foundFreeNumber = false;

  
  QString tableName;
  quint8 maxPlayers;
  connData->in >> tableName;
  connData->in >> maxPlayers;
  
  quint8 cycleState;
  connData->in >> cycleState;
  quint8 tableTimer;
  connData->in >> tableTimer;
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
  
  if (connData->loggedIn == false)
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

  foreach (QTcpSocket* connection, connections)
    connection->write(block);  

  playerDataHash[connData->username].tablenum = tablenum;
  playerDataHash[connData->username].readyToPlay = false;
  playerDataHash[connData->username].gaveUp = false;

  tableData *tmp = new tableData;
  tmp->initialize(tablenum, tableName, maxPlayers, connData->username, cycleState, tableTimer, GAMEMODE_UNSCRAMBLE);


  connect(tmp->timer, SIGNAL(timeout()), this, SLOT(updateTimer()));
  connect(tmp->countdownTimer, SIGNAL(timeout()), this, SLOT(updateCountdownTimer()));
  tables.insert(tablenum, tmp);

  writeHeaderData();
  out << (quint8) 'J';
  out << (quint16) tablenum;
  out << connData->username;
  fixHeaderLength();
  
  foreach (QTcpSocket* connection, connections)
    connection->write(block);  

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

  if (connData->loggedIn == false)
    {
      qDebug() << "join? wait.. still logging in!";
      return;
    }

  // got here with no errors, join table!
  
  // does this work?
  tmp->playerList << username;
  if (tmp->playerList.size() == tmp->maxPlayers) tmp->canJoin = false;

  writeHeaderData();
  out << (quint8) 'J';
  out << (quint16) tablenum;
  out << username;
  fixHeaderLength();
  foreach (QTcpSocket* connection, connections)
    connection->write(block);
  
  playerDataHash[username].tablenum = tablenum;
  playerDataHash[username].readyToPlay = false;
  playerDataHash[username].gaveUp = false;
  if (tmp->gameStarted)
    {
      sendUserCurrentAlphagrams(tmp, socket);
      //sendUser
    }
}

void MainServer::processPrivateMessage(QTcpSocket* socket, connectionData* connData)
{
  QString username;
  connData->in >> username;
  QString message;
  connData->in >> message;
  if (username.length() + message.length() > 400)
    {
      socket->disconnectFromHost();
      return;
    }

  if (usernamesHash.find(username) != usernamesHash.end())
    {
      // the username exists
      QTcpSocket* connection = usernamesHash.value(username); // receiver
      writeHeaderData();      
      out << (quint8) 'P';
      out << connData->username; // sender
      out << message; 
      fixHeaderLength();
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
      socket->disconnectFromHost();
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

  // create a new playerData
  playerData tempPD;
  tempPD.username = username;
  tempPD.readyToPlay = false;
  tempPD.gaveUp = false;
  tempPD.score = 0;
  tempPD.tablenum = 0;

  playerDataHash.insert(username, tempPD);
  

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

      foreach(QString thisUsername, table->playerList)
	{
	  writeHeaderData();
	  out << (quint8) 'J';
	  out << (quint16) table->tableNumber;
	  out << thisUsername;
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

void MainServer::updateTimer()
{
  QTimer* timer = static_cast <QTimer*> (sender());
  // corresponds to a specific table
  // figure out which one
  quint16 tablenum = (quint16) timer->property("tablenum").toInt();
  tableData *tmp = tables.value(tablenum);
  tmp->currentTimerVal--;

  QList <QVariant> tempList;
  tempList << QVariant(tmp->currentTimerVal);
  writeToTable(tablenum, tempList, TIMER_VALUE);

  if (tmp->currentTimerVal == 0)
    {
      endGame(tmp);
    }

}

void MainServer::updateCountdownTimer()
{
  QTimer* timer = static_cast <QTimer*> (sender());
  // corresponds to a specific table
  // figure out which one
  quint16 tablenum = (quint16) timer->property("tablenum").toInt();
  tableData *tmp = tables.value(tablenum);
  tmp->countdownTimerVal--;

  QList <QVariant> tempList;
  tempList << QVariant(tmp->countdownTimerVal);
  writeToTable(tablenum, tempList, TIMER_VALUE);

  if (tmp->countdownTimerVal == 0)
    {
      startGame(tmp);
   }
}

void MainServer::startGame(tableData* table)
{
  table->countingDown = false;
  table->countdownTimer->stop();
  writeToTable(table->tableNumber, dummyList, GAME_STARTED);
  
  // this seems like horrible overkill. there has to be a better way...
  QList <QVariant> tempList;
  tempList << QVariant(table->tableTimerVal);
  writeToTable(table->tableNumber, tempList, TIMER_VALUE);
  
  
  // code for starting the game
  // steps:
  // 1. list should already be loaded when table was created
  // 2. send to everyone @ the table:
  //    - 45 alphagrams
  prepareTableAlphagrams(table);
  foreach (QString player, table->playerList)
    sendUserCurrentAlphagrams(table, usernamesHash.value(player));
  table->currentTimerVal = table->tableTimerVal;
  table->timer->start(1000);
  table->gameStarted = true;
}

void MainServer::endGame(tableData* tmp)
{
  tmp->timer->stop();
  tmp->gameStarted = false;
  // TODO here is code for the game ending
  // send game ended to everyone, etc.
  writeToTable(tmp->tableNumber, dummyList, GAME_ENDED);
  QList <QVariant> tempList;
  tempList << QVariant(0);
  writeToTable(tmp->tableNumber, tempList, TIMER_VALUE);

  foreach (QString username, tmp->playerList)
    {
      playerDataHash[username].readyToPlay = false;
      playerDataHash[username].gaveUp = false;
    }

  // if in cycle mode, update list
  if (tmp->cycleState == 1)
    {
      for (int i = 0; i < 45; i++)
	{
	  if (tmp->unscrambleGameQuestions.at(i).numNotYetSolved > 0)
	    {
	      // this one has not been solved!
	      
	      tmp->missedFileWriter << tmp->unscrambleGameQuestions.at(i).alphagram;
	      for (int j = 0; j < tmp->unscrambleGameQuestions.at(i).solutions.size(); j++)
		tmp->missedFileWriter << " " << tmp->unscrambleGameQuestions.at(i).solutions.at(j);
	      tmp->missedFileWriter << "\n";

	      qDebug() << "wrote" << tmp->unscrambleGameQuestions.at(i).alphagram << 
		tmp->unscrambleGameQuestions.at(i).solutions << " to missed file.";
	    }
	}

    }


}


void MainServer::processChat(QTcpSocket* socket, connectionData* connData)
{
  QString username = connData->username;
  QString chattext;
  connData->in >> chattext;
  

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
  foreach (QTcpSocket* connection, connections)
    connection->write(block);
  
  qDebug() <<" ->" << username << ":" << chattext;

}

void MainServer::writeToTable(quint16 tablenum, QList <QVariant> parameterList, tablePacketHeaderStatesEnum type)
{  
  tableData* table = tables.value(tablenum);

  writeHeaderData();
  out << (quint8) '+';
  out << (quint16) tablenum;

  switch (type)
    {
    case GAME_STARTED:
      out << (quint8) 'S';
      break;

    case GAME_ENDED:
      out << (quint8) 'E';
      break;

    case CHAT_SENT:
      {
	QString username = parameterList.at(0).toString();
	QString chat = parameterList.at(1).toString();
	out << (quint8) 'C';
	out << username;
	out << chat;

      }
      break;
      
    case GUESS_RIGHT:
      {
	QString username = parameterList.at(0).toString();
	QString guess = parameterList.at(1).toString();
	quint8 row = parameterList.at(2).toInt();
	quint8 column = parameterList.at(3).toInt();
	out << (quint8)'A' << username << guess << row << column;
      }
      

      break;
    case TIMER_VALUE:
      out << (quint8) 'T';
      out << (quint16) parameterList.at(0).toInt();

      break;
    case READY_TO_BEGIN:
      out << (quint8)'B';
      out << parameterList.at(0).toString(); 
      break;

    case GAVE_UP:
      out << (quint8)'U';
      out << parameterList.at(0).toString();
      break;
    }

  fixHeaderLength();
  
  foreach(QString userInTable, table->playerList)
    usernamesHash.value(userInTable)->write(block);
  



}

void MainServer::writeToClient(QTcpSocket* socket, QString parameter, packetHeaderStatesEnum type)
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
  if (username.length() > 16) return false;
  if (!username.at(0).isLetter()) return false;
  for (int i = 1; i < username.length(); i++)
    if (!username.at(i).isLetterOrNumber()) return false;
  return true;
}
