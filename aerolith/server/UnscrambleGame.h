#ifndef _UNSCRAMBLE_GAME_H_
#define _UNSCRAMBLE_GAME_H_

#include <QtCore>




class UnscrambleGame : public TableGame
{
 public:
  void initialize(quint8 cycleState, quint8 tableTimer);
  ~UnscrambleGame();
 private:
  
  struct unscrambleGameData
  {
  // holds data for each word
    QString alphagram;
    quint8 numNotYetSolved;
    quint8 i, j; // indices in table
    QStringList solutions;
  };

  struct playerData
  {
    QString username;
    bool readyToPlay;
    bool gaveUp;
    quint16 score;
    quint16 tablenum;
  };
  
  QHash <QString, playerData> playerDataHash;  
  QString wordListDescriptor;
  bool gameStarted;
  bool countingDown;
  QTimer *timer;
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
  QTextStream missedFileWriter;
  bool tempFileExists;


};

#endif
