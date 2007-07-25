#ifndef _TABLE_H_
#define _TABLE_H_

#define GAMEMODE_UNSCRAMBLE = 0;
#define GAMEMODE_ANAGRAMS = 1;
#define GAMEMODE_TRIVIA = 2; // and so on

class tableData
{
 public:
  ~tableData();
  void initialize(quint16 tableNumber, QString tableName, quint8 maxPlayers, QString tableCreator, quint8 cycleState, quint8 tableTimer);
  quint16 tableNumber;
  //QString wordListDescriptor; // todo: change this to table name
  QString tableName;
  QStringList playerList;
  quint8 maxPlayers;
  bool canJoin; 
  // bool gameStarted;
  //bool countingDown;
  //QTimer *timer;
  //QTimer *countdownTimer;
  //quint16 currentTimerVal;
  //quint16 tableTimerVal;
  //quint8 countdownTimerVal;
  //quint8 cycleState; // holds value of cycle radio button
  // if 1, it will cycle at the end of the word list thru all missed words
  // if 0, it will just pick random words every time
  // if 2 it's "endless mode"
  TableGame* tableGame;

  // unscrambleGame should include:
  // QString wordListDescriptor 
  // bool gameStarted
  // bool countingDown
  // QTimer *timer
  // QTimer *countdownTimer
  // quint16 currentTimerVal
  // quint16 tableTimerVal
  // quint8 countdownTimerVal
  // quint16 totalNumberQuestions
  // quint8 cycleState
  // QHash <QString, QString> gameSolutions
  // QHash <QString, quint8> alphagramIndices
  // QList <unscrambleGameData> unscrambleGameQuestions
  // QTextStream alphagramReader
  // QFile inFile
  // QTextStream missedFileWriter
  // bool tempFileExists
  





  //  QHash <QString, QString> gameSolutions; // the KEY (first value) is a solution
  // the VALUE (second) is an alphagram

  //QHash <QString, quint8> alphagramIndices; // the KEY is the alphagram, VALUE is the index in 'unscrambleGameQuestions

  //  QList <unscrambleGameData> unscrambleGameQuestions;
  
  //  QTextStream alphagramReader;
  // QFile inFile;
  
  // QFile outFile;
  //QTextStream missedFileWriter;
  
  //  bool tempFileExists;

  // placeholder for anagrams mode
  

  quint8 gameMode; // jumble = 0, anagrams = 1, etc etc
  QByteArray letters; // the letters that are showing on the board

};

#endif
