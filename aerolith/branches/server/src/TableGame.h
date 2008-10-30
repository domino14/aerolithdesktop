#ifndef _TABLE_GAME_H_
#define _TABLE_GAME_H_

#include <QtCore>
#include "table.h"
#include "ClientWriter.h"
// this class should not be instantiated!
#include "ClientSocket.h"

class tableData;

class TableGame : public QObject
{

Q_OBJECT

 public:

  // change to initialize (QList <QVariant>)
  TableGame(tableData*);
  virtual ~TableGame();
  

  virtual void initialize(quint8, quint8, QString) = 0;
  virtual void gameStartRequest(ClientSocket*) = 0;
  virtual void guessSent(ClientSocket*, QString) = 0;
  virtual void gameEndRequest(ClientSocket*) = 0;
  virtual void playerJoined(ClientSocket*) = 0;

  // the most generic game packets. the more specific ones go in the specific game classes (like UnscrambleGame gets sendGuessRightPacket)

  void sendReadyBeginPacket(QString);
  void sendGameStartPacket();
  void sendGameEndPacket();
  

 protected:
  tableData* table;

};

#endif
