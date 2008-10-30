#include "TableGame.h"

extern QByteArray block;
extern QDataStream out;


TableGame::~TableGame()
{
  qDebug() << "TableGame destructor";
}

TableGame::TableGame(tableData* table)
{
  this->table = table;
  qDebug() << "tablegame constructor";
}

void TableGame::sendReadyBeginPacket(QString username)
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) table->tableNumber;
  out << (quint8)SERVER_TABLE_READY_BEGIN;
  out << username;
  fixHeaderLength();
  table->sendGenericPacket();

}

void TableGame::sendGameStartPacket()
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) table->tableNumber;
  out << (quint8) SERVER_TABLE_GAME_START;
  fixHeaderLength();
  table->sendGenericPacket();
}

void TableGame::sendGameEndPacket()
{
  writeHeaderData();
  out << (quint8) SERVER_TABLE_COMMAND;
  out << (quint16) table->tableNumber;
  out << (quint8) SERVER_TABLE_GAME_END;
  fixHeaderLength();
  table->sendGenericPacket();
}

