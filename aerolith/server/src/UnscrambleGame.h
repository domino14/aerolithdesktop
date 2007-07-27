#ifndef _UNSCRAMBLE_GAME_H_
#define _UNSCRAMBLE_GAME_H_

#include <QtCore>
#include <QObject>
#include "TableGame.h"


class UnscrambleGame : public TableGame
{

Q_OBJECT

  public:
 void initialize(quint8 cycleState, quint8 tableTimer, QString wordListFileName);
 UnscrambleGame(tableData*);
 ~UnscrambleGame();
 
 void gameStartRequest(ClientSocket*);
 void guessSent(ClientSocket*, QString);
 void gameEndRequest(ClientSocket*);
 void playerJoined(ClientSocket*);
 
 void endGame();
 void startGame();

 private:

  void prepareTableAlphagrams();
  void sendUserCurrentAlphagrams(ClientSocket*);
  
  struct unscrambleGameData
  {
  // holds data for each word
    QString alphagram;
    quint8 numNotYetSolved;
    quint8 i, j; // indices in table
    QStringList solutions;
  };

  //  QHash <QString, playerData> playerDataHash;  
  QString wordListFileName;
  bool gameStarted;
  bool countingDown;
  QTimer *gameTimer;
  QTimer *countdownTimer;
  quint16 currentTimerVal;
  quint16 tableTimerVal;
  quint8 countdownTimerVal;
  quint16 totalNumberQuestions;
  quint8 cycleState;
  QHash <QString, QString> gameSolutions;
  QHash <QString, quint8> alphagramIndices;
  QList <unscrambleGameData> unscrambleGameQuestions;
  QTextStream alphagramReader;
  QFile inFile;
  QFile outFile;
  QTextStream missedFileWriter;
  bool tempFileExists;
  
  void generateTempFile();
  void sendTimerValuePacket(quint16);
  void sendGiveUpPacket(QString);
  void sendGuessRightPacket(QString, QString, quint8, quint8);

  private slots:
    void updateGameTimer();
  void updateCountdownTimer();

};

#endif
