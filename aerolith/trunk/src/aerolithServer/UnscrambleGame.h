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
    QStringList solutions;
    alphagramInfo()
    {
    }
    alphagramInfo(QString a, QStringList s)
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

    void initialize(quint8 cycleState, quint8 tableTimer, QString wordList, quint8 lexiconIndex);
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
    static void loadWordLists();
    static void sendLists(ClientSocket*);
    static QHash <QString, challengeInfo> challenges;

    static bool midnightSwitchoverToggle;

private:

    //  static QVector<QVector <QVector<alphagramInfo> > > alphagramData;

    enum WordListTypes
    {
        LIST_TYPE_DAILY_CHALLENGE, LIST_TYPE_REGULAR
    };


    struct WordList
    {
        QString name;
        WordListTypes type;
        QString lexiconName;
        WordList(QString _name, WordListTypes _type, QString _lexiconName)
        {
            name = _name; type = _type; lexiconName = _lexiconName;
        }
        WordList();
    };
    static QList <WordList> orderedWordLists;
    static QByteArray wordListDataToSend;

    void prepareTableAlphagrams();
    void sendUserCurrentAlphagrams(ClientSocket*);
    QString lexiconName;
    struct unscrambleGameQuestionData
    {
        // holds data for each word
        QString alphagram;
        quint8 numNotYetSolved;
        //    quint8 index;	// index
        QStringList solutions;
        quint16 probability;
    };

    //  QHash <QString, playerData> playerDataHash;
    QString wordList;
    bool neverStarted;
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
    QList <unscrambleGameQuestionData> unscrambleGameQuestions;
    /*QTextStream alphagramReader;
  QFile inFile;
  QFile outFile;
  QTextStream missedFileWriter;*/

    QVector <quint16> missedArray;
    QVector <quint16> quizArray;
    quint16 quizIndex;
    QVector <alphagramInfo> *alphaInfo;

    quint8 wordLength; // the word length of this list
    bool thisTableSwitchoverToggle;

    quint16 numTotalRacks;
    quint16 numRacksSeen;
    void generateQuizArray();
    void sendTimerValuePacket(quint16);
    void sendGiveUpPacket(QString);
    void sendGuessRightPacket(QString, QString, quint8);
    void sendNumQuestionsPacket();


private slots:
    void updateGameTimer();
    void updateCountdownTimer();

};

#endif
