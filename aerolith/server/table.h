#ifndef _TABLE_H_
#define _TABLE_H_

#include "unscrambleGameData.h"

class tableData
{
 public:
  
  void initialize(quint16 tableNumber, QString wordListDescriptor, quint8 maxPlayers, QString tableCreator, quint8 cycleState, quint8 tableTimer);
  quint16 tableNumber;
  QString wordListDescriptor;
  QStringList playerList;
  quint8 maxPlayers;
  bool canJoin; 
  bool gameStarted;
  bool countingDown;
  QTimer *timer;
  QTimer *countdownTimer;
  quint16 currentTimerVal;
  quint16 tableTimerVal;
  quint8 countdownTimerVal;
  quint16 totalNumberQuestions;
  
  quint8 cycleState; // holds value of cycle radio button
  // if 1, it will cycle at the end of the word list thru all missed words
  // if 0, it will just pick random words every time
  // if 2 it's "endless mode"

  QHash <QString, QString> gameSolutions; // the KEY (first value) is a solution
  // the VALUE (second) is an alphagram

  QHash <QString, quint8> alphagramIndices; // the KEY is the alphagram, VALUE is the index in 'unscrambleGameQuestions

  QList <unscrambleGameData> unscrambleGameQuestions;
  
  QTextStream alphagramReader;
  QFile inFile;
  
  QFile outFile;
  QTextStream missedFileWriter;
  
  bool tempFileExists;

  // placeholder for anagrams mode
  
  quint8 gameMode; // jumble = 0, anagrams = 1, etc etc
  QByteArray letters; // the letters that are showing on the board

};

#endif
