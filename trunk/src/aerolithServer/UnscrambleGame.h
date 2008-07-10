#ifndef _UNSCRAMBLE_GAME_H_
#define _UNSCRAMBLE_GAME_H_

#include <QtCore>
#include <QtSql>
#include <QObject>
#include "TableGame.h"

struct highScoreData
{
  QString userName;
  quint16 numCorrect;
  quint16 numSolutions;
  quint16 timeRemaining;
};

struct challengeInfo
{
	QHash <QString, highScoreData> *highScores;
	quint8 wordLength;
	QVector <quint16> dbIndices;
};

struct alphagramInfo
{
	QString alphagram;
	QString solutions;
	alphagramInfo()
	{
	}
	alphagramInfo(QString a, QString s)
	{
		alphagram = a;
		solutions = s;
	}
};

void getUniqueRandomNumbers(QVector<quint16>&numbers, quint16 start, quint16 end, quint16 numNums);

class UnscrambleGame : public TableGame
{

Q_OBJECT

public:

	void initialize(quint8 cycleState, quint8 tableTimer, QString wordList);
	UnscrambleGame(tableData*);
	~UnscrambleGame();
	
	void gameStartRequest(ClientSocket*);
	void guessSent(ClientSocket*, QString);
	void gameEndRequest(ClientSocket*);
	void playerJoined(ClientSocket*);
  void playerLeftGame(ClientSocket*);
	
	void endGame();
	void startGame();
	
	static void generateDailyChallenges();
	static void prepareWordDataStructure();
	static QHash <QString, challengeInfo> challenges;
	
	static bool midnightSwitchoverToggle;
 
private:
 
  static QVector <QVector<alphagramInfo> > alphagramData;
	
  void prepareTableAlphagrams();
  void sendUserCurrentAlphagrams(ClientSocket*);
  
  struct unscrambleGameData
  {
  // holds data for each word
    QString alphagram;
    quint8 numNotYetSolved;
    quint8 index;	// index
    QStringList solutions;
  };

  //  QHash <QString, playerData> playerDataHash;  
  QString wordList;
  bool listExhausted;
  bool wroteToMissedFileThisRound;
  bool gameStarted;
  bool countingDown;
  bool startEnabled;
  QTimer gameTimer;
  QTimer countdownTimer;
  quint16 currentTimerVal;
  quint16 tableTimerVal;
  quint8 countdownTimerVal;
  quint16 totalNumberQuestions;
  quint8 cycleState;
  quint16 numTotalSolutions;
  QHash <QString, QString> gameSolutions;
  QHash <QString, quint8> alphagramIndices;
  QList <unscrambleGameData> unscrambleGameQuestions;
  QTextStream alphagramReader;
  QFile inFile;
  QFile outFile;
  QTextStream missedFileWriter;
  bool tempFileExists;
  quint8 wordLengths; // the word length of this list (used only for daily challenges)
  bool thisTableSwitchoverToggle;

  quint16 numTotalRacks;
  quint16 numRacksSeen;
  quint16 numMissedRacks;
  void generateTempFile();
  void sendTimerValuePacket(quint16);
  void sendGiveUpPacket(QString);
  void sendGuessRightPacket(QString, QString, quint8);
  void sendNumQuestionsPacket();
  
  
  private slots:
    void updateGameTimer();
  void updateCountdownTimer();

};

#endif
