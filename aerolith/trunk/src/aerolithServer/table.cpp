#include "table.h"

#include "UnscrambleGame.h"
#include "ClientWriter.h"

extern QByteArray block;
extern QDataStream out;
//  tmp->initialize(tablenum, wordListDescriptor, maxPlayers, connData->username);
void tableData::initialize(quint16 tableNumber, QString tableName, quint8 maxPlayers, 
	ClientSocket* tableCreator, quint8 cycleState, quint8 tableTimer, gameModes gameMode, 
	QString additionalDescriptor)
{
	qDebug() << "Creating a table.";
  this->tableNumber = tableNumber;
  this->tableName = tableName;
  this->maxPlayers = maxPlayers;
  //playerList << tableCreator;
	host = tableCreator;
	canJoin = true;
  this->gameMode = gameMode;

 

  // for NOW, default gameMode = unscramble

  if (gameMode == GAMEMODE_UNSCRAMBLE)
    {
      tableGame = new UnscrambleGame(this);
      tableGame->initialize(cycleState, tableTimer, additionalDescriptor);
    }
  
}

tableData::~tableData()
{
  qDebug() << "tableData destructor";
  delete tableGame;
}

void tableData::removePlayerFromTable(ClientSocket* socket)
{
	tableGame->playerLeftGame(socket);
	playerList.removeAll(socket);
	qDebug() << "players in table" << tableNumber << playerList;
	
}

void tableData::sendGenericPacket()
{
  foreach (ClientSocket* thisSocket, playerList)
    thisSocket->write(block);
}

void tableData::sendChatSentPacket(QString username, QString chat)
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) tableNumber;
  out << (quint8) SERVER_TABLE_CHAT;
  out << username << chat;
  fixHeaderLength();
  sendGenericPacket();
}

void tableData::sendTableMessage(QString message)
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) tableNumber;
  out << (quint8) SERVER_TABLE_MESSAGE;
  out << message;
  fixHeaderLength();
  sendGenericPacket();
}