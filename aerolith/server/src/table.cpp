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
  tableHost = tableCreator;
  peopleList << tableCreator;
  //sittingList.resize(maxPlayers);
  for (int i = 0; i < maxPlayers; i++)
    sittingList.push_back(NULL);
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

//void tableData::

void tableData::sendGenericPacket()
{
  foreach (ClientSocket* thisSocket, peopleList)
    thisSocket->write(block);
}
/*
void tableData::sendStoodUpPacket(quint8 vacantSeat)
{
  writeHeaderData();
  out << (quint8) '+' << (quint16) tableNumber << (quint8)'^';
  out << vacantSeat;
  fixHeaderLength();
  sendGenericPacket();
}

void tableData::sendSatPacket(quint8 seat, QString username)
{
  writeHeaderData();
  out << (quint8) '+' << (quint16) tableNumber << (quint8)'_';
  out << username << seat;
  fixHeaderLength();
  sendGenericPacket();
}
*/

bool tableData::tryToSit(ClientSocket* socket, int seat)
{
  if (peopleList.contains(socket) && !sittingList.contains(socket))
    {
      // can maybe sit
      if (seat > 0 && seat <= maxPlayers && socket->connData.seatNum == 0) // 
	{
	  if (sittingList.at(seat-1) == NULL)
	    {
	      sittingList[seat-1] = socket;
	      // sat!
	      socket->connData.seatNum = seat;
	      return true;
	    }
	}


    }
  return false;
}

void tableData::standUp(ClientSocket* socket, int seat)
{
  if (seat > 0 && seat <= maxPlayers)
    {
      if (sittingList.at(seat-1) == socket)
	{
	  sittingList[seat-1] = NULL;
	  socket->connData.seatNum = 0;
	}
      else
	{
	  qDebug() << "not sitting here?";
	}
    }

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
