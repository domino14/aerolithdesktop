#include "table.h"

#include "UnscrambleGame.h"
#include "ClientWriter.h"

extern QByteArray block;
extern QDataStream out;
//  tmp->initialize(tablenum, wordListDescriptor, maxPlayers, connData->username);
void tableData::initialize(quint16 tableNumber, QString tableName, quint8 maxPlayers, ClientSocket* tableCreator, quint8 cycleState, quint8 tableTimer, gameModes gameMode, QString additionalDescriptor)
{

  this->tableNumber = tableNumber;
  this->tableName = tableName;
  this->maxPlayers = maxPlayers;
  playerList << tableCreator;

  if (maxPlayers == 1) canJoin = false;
  else canJoin = true;
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
void tableData::sendGenericPacket()
{
  foreach (ClientSocket* thisSocket, playerList)
    thisSocket->write(block);
}

void tableData::sendChatSentPacket(QString username, QString chat)
{
  writeHeaderData();
  out << (quint8) '+';
  out << (quint16) tableNumber;
  out << (quint8) 'C';
  out << username << chat;
  fixHeaderLength();
  sendGenericPacket();
}

void tableData::sendTableMessage(QString message)
{
  writeHeaderData();
  out << (quint8) '+';
  out << (quint16) tableNumber;
  out << (quint8) 'M';
  out << message;
  fixHeaderLength();
  sendGenericPacket();
}
