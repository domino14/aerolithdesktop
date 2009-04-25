//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _UNSCRAMBLE_GAME_H_
#define _UNSCRAMBLE_GAME_H_

#include <QtCore>
#include <QtSql>
#include <QObject>
#include "TableGame.h"
#include "databasehandler.h"

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
    QVector <quint32> dbIndices;
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

void getUniqueRandomNumbers(QVector<quint32>&numbers, quint32 start, quint32 end, int numNums);

class UnscrambleGame : public TableGame
{

    Q_OBJECT

public:

    QByteArray initialize(DatabaseHandler* dbHandler);
    UnscrambleGame(Table*);
    ~UnscrambleGame();

    void gameStartRequest(ClientSocket*);
    void guessSent(ClientSocket*, QString);
    void gameEndRequest(ClientSocket*);
    void playerJoined(ClientSocket*);
    void playerLeftGame(ClientSocket*);

    void endGame();
    void startGame();

    static void generateDailyChallenges(DatabaseHandler* dbHandler);
    static void loadWordLists(DatabaseHandler* dbHandler);
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
    DatabaseHandler* dbHandler;
    struct unscrambleGameQuestionData
    {
        // holds data for each word
        QString alphagram;
        quint8 numNotYetSolved;
        //    quint8 index;	// index
        QStringList solutions;
        int probability;
    };

    //  QHash <QString, playerData> playerDataHash;
    QSqlQuery query;
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

    QVector <quint32> missedArray;
    QVector <quint32> quizArray;
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
