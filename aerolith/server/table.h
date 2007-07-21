#ifndef _TABLE_H_
#define _TABLE_H_

#include "unscrambleGameData.h"

class tableData
{
 public:
  
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
  
  quint8 alphagramState; // holds value of alphagram radio button
  // if 1 it will alphagram automatically
  // if 0 it will shuffle automatically
  QHash <QString, QString> gameSolutions; // the KEY (first value) is a solution
  // the VALUE (second) is an alphagram
  QList <unscrambleGameData> alphagrams;
  //   quint16 indexOfCurrentQuestion; // LAST PLACE in the file that was read from!
  // should be a multiple of 45 typically. i.e. read 45 lines, so the index of 
  // curq should be 45 so that it starts reading at line 45 next.
  QTextStream alphagramReader;
  QFile inFile;
  
  QFile outFile;
  QTextStream missedFileWriter;
  
  bool tempFileExists;
};

#endif
