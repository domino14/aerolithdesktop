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

#include "UnscrambleGame.h"
#include "commonDefs.h"
#include "listmaker.h"
extern QByteArray block;
extern QDataStream out;

const quint8 COUNTDOWN_TIMER_VAL = 3;
const quint8 maxRacks = 50;
QHash <QString, challengeInfo> UnscrambleGame::challenges;
bool UnscrambleGame::midnightSwitchoverToggle;
const QString WORD_DATABASE_NAME;  /* TODO FIX. there are many databases */
QByteArray UnscrambleGame::wordListDataToSend;
//QVector<QVector <QVector<alphagramInfo> > > UnscrambleGame::alphagramData;

void UnscrambleGame::initialize(quint8 cycleState, quint8 tableTimer, QString wordList, QString lexiconName)
{
    wroteToMissedFileThisRound = false;
    listExhausted = false;
    this->wordList = wordList;
    this->lexiconName = lexiconName;
    this->cycleState = cycleState;

    connect(&gameTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));
    connect(&countdownTimer, SIGNAL(timeout()), this, SLOT(updateCountdownTimer()));

    gameStarted = false;
    countingDown = false;
    startEnabled = true;
    tableTimerVal = (quint16)tableTimer * (quint16)60;
    currentTimerVal = tableTimerVal;
    countdownTimerVal = COUNTDOWN_TIMER_VAL;


    /*

        */
    numRacksSeen = 0;
    numTotalRacks = 0;

    generateQuizArray();

    if (cycleState == TABLE_TYPE_DAILY_CHALLENGES)
    {
        if (wordLength <= 3) tableTimerVal = 100;
        if (wordLength == 4) tableTimerVal = 150;
        if (wordLength == 5) tableTimerVal = 220;
        if (wordLength == 6) tableTimerVal = 240;
        if (wordLength >= 7 && wordLength <= 8) tableTimerVal = 270;
        if (wordLength > 8) tableTimerVal = 300;
    }


}

UnscrambleGame::UnscrambleGame(Table* table) : TableGame(table)
{
    qDebug() << "UnscrambleGame constructor";
}

UnscrambleGame::~UnscrambleGame()
{
    qDebug() << "UnscrambleGame destructor";

    //	gameTimer->deleteLater();
    //	countdownTimer->deleteLater();

}

void UnscrambleGame::playerJoined(ClientSocket* client)
{
    if (gameStarted)
        sendUserCurrentAlphagrams(client);

    // possibly send scores, solved solutions, etc. in the future.
    if (table->playerList.size() == 1 && table->maxPlayers == 1 && cycleState != TABLE_TYPE_DAILY_CHALLENGES)
    {
        QSqlQuery userQuery(QSqlDatabase::database("usersDB"));
        userQuery.prepare("SELECT saveData from users where username = :username");
        userQuery.bindValue(":username", table->playerList.at(0)->connData.userName.toLower());
        userQuery.exec();
        while (userQuery.next())
        {
            QByteArray ba = userQuery.value(0).toByteArray();
            QDataStream stream(ba);
            if (ba.size() == 0)
            {
                qDebug() << "there was no save data";
                /* do nothing. this is a new table, and there was no save data. */
            }
            else
            {
                QString savedWordList;
                QVector <quint32> savedMissedArray;
                QVector <quint32> savedQuizArray;
                stream >> savedWordList >> savedMissedArray >> savedQuizArray;
                if (savedQuizArray.size() == 0 && savedMissedArray.size() == 0)
                {
                    qDebug() << "nothing needs to be loaded";
                    /* this data was saved when the user left the table, but after completely exhausting the quiz.
                                                so do nothing, nothing needs to be loaded*/
                }
                else
                {
                    if (savedWordList != wordList)
                    {
                        table->sendTableMessage("You currently have save data for the following word list: " + savedWordList + ". Please "
                                                "exit the table if you don't wish to lose your save data, and select the above list instead. If you click Start, "
                                                "the saved data will be deleted!");
                    }
                    else
                    {
                        quizArray = savedQuizArray;
                        missedArray = savedMissedArray;
                        table->sendTableMessage("You had saved data for this word list, and it has been loaded!");

                        numRacksSeen = 0;
                        numTotalRacks = savedQuizArray.size();

                    }

                }
            }
        }	//while



    }


}

void UnscrambleGame::playerLeftGame(ClientSocket* socket)
{
    // if this is the ONLY player, stop game.
    if (table->playerList.size() == 1 && table->maxPlayers == 1)
    {
        gameEndRequest(socket);
        if (cycleState != TABLE_TYPE_DAILY_CHALLENGES && !listExhausted && !neverStarted)
        {
            // generate blob for contents of missed file, temp file
            // missedFileWriter has written all the necessary words to missedFile, so read the entire file.
            QTime timer;
            timer.start();

            QByteArray ba;
            QDataStream stream(&ba, QIODevice::WriteOnly);

            stream << wordList;
            stream << missedArray;
            stream << quizArray.mid(quizIndex);
            // now write data to database
            QSqlQuery userQuery(QSqlDatabase::database("usersDB"));
            userQuery.prepare("UPDATE users SET saveData = :saveData where username = :username");
            userQuery.bindValue(":username", socket->connData.userName.toLower());
            userQuery.bindValue(":saveData", ba);
            userQuery.exec();

            qDebug() << "Saving lists took milliseconds: " << timer.elapsed();

        }
        else
        {

        }
    }
}

void UnscrambleGame::gameStartRequest(ClientSocket* client)
{
    if (startEnabled == true && gameStarted == false && countingDown == false)
    {
        bool startTheGame = true;

        sendReadyBeginPacket(client->connData.userName);
        client->playerData.readyToPlay = true;

        foreach (ClientSocket* thisClient, table->playerList)
            if (thisClient->playerData.readyToPlay == false)
            {
            startTheGame = false;
            break;
        }


        if (startTheGame == true)
        {
            countingDown = true;
            countdownTimerVal = 3;
            countdownTimer.start(1000);
            sendTimerValuePacket(countdownTimerVal);
            if (cycleState == TABLE_TYPE_DAILY_CHALLENGES)
            {
                if (challenges.contains(wordList))
                {
                    if (challenges.value(wordList).
                        highScores->contains(table->playerList.at(0)->connData.userName.toLower()))
                        table->sendTableMessage("You've already played this challenge. You can play again, but only the first game's results count toward today's high scores!");

                }
            }

            if (neverStarted)
            {
                // this will only be true the very FIRST time this function is called.
                // overwrite the save array for this user.

                if (table->playerList.size() == 1 && cycleState != TABLE_TYPE_DAILY_CHALLENGES && table->maxPlayers == 1)
                {

                    // only overwrite the list if this is a single player playing in a non-daily-challenge table.
                    QByteArray ba;
                    QSqlQuery userQuery(QSqlDatabase::database("usersDB"));
                    userQuery.prepare("UPDATE users SET saveData = :saveData where username = :username");
                    userQuery.bindValue(":username", table->playerList.at(0)->connData.userName.toLower());
                    userQuery.bindValue(":saveData", ba);
                    userQuery.exec();
                    table->sendTableMessage("If you quit Aerolith or leave this table, "
                                            "your progress on this word list -" + wordList + "- will automatically be saved.");
                }
            }

            neverStarted = false;

        }
    }

}

void UnscrambleGame::guessSent(ClientSocket* socket, QString guess)
{

    guess = guess.toUpper().trimmed();
    if (guess == "")
    {


    }
    else
    {
        if (gameStarted)
        {
            if (gameSolutions.contains(guess))
            {
                QString alphagram = gameSolutions.value(guess);
                gameSolutions.remove(guess);
                quint8 indexOfQuestion = alphagramIndices.value(alphagram);
                unscrambleGameQuestions[indexOfQuestion].numNotYetSolved--;

                sendGuessRightPacket(socket->connData.userName, guess, indexOfQuestion);
                if (gameSolutions.isEmpty()) endGame();
            }
        }
    }
    qDebug() << " ->GUESS" << socket->connData.userName << guess;
}

void UnscrambleGame::gameEndRequest(ClientSocket* socket)
{
    if (gameStarted == true)
    {
        bool giveUp = true;

        sendGiveUpPacket(socket->connData.userName);
        socket->playerData.gaveUp = true;

        foreach (ClientSocket* thisSocket, table->playerList)
        {
            if (thisSocket->playerData.gaveUp == false)
            {
                giveUp = false;
                break;
            }
        }
        if (giveUp == true)
        {
            currentTimerVal = 0;
            endGame();
        }
    }

}

void UnscrambleGame::startGame()
{

    countingDown = false;
    countdownTimer.stop();
    // code for starting the game
    // steps:
    //1. list should already be loaded when table was created
    //2. send to everyone @ the table:
    //    - maxRacks alphagrams
    gameStarted = true;
    wroteToMissedFileThisRound = false;
    prepareTableAlphagrams();
    foreach (ClientSocket* socket, table->playerList)
        sendUserCurrentAlphagrams(socket);

    sendGameStartPacket();
    sendTimerValuePacket(tableTimerVal);
    sendNumQuestionsPacket();
    currentTimerVal = tableTimerVal;
    gameTimer.start(1000);
    thisTableSwitchoverToggle = midnightSwitchoverToggle;


}

void UnscrambleGame::endGame()
{
    gameTimer.stop();
    gameStarted = false;

    sendGameEndPacket();
    sendTimerValuePacket((quint16)0);

    foreach (ClientSocket* client, table->playerList)
    {
        client->playerData.readyToPlay = false;
        client->playerData.gaveUp = false;
    }
    // if in cycle mode, update list
    if (cycleState == TABLE_TYPE_CYCLE_MODE)
    {
        if (!wroteToMissedFileThisRound)
        {
            wroteToMissedFileThisRound = true;
            for (int i = 0; i < maxRacks; i++)
            {
                if (unscrambleGameQuestions.at(i).numNotYetSolved > 0)
                {
                    missedArray << unscrambleGameQuestions.at(i).probability;
                }

            }
        }
        qDebug() << "Missed to date:" << missedArray.size() << "racks.";
    }
    else if (cycleState == TABLE_TYPE_DAILY_CHALLENGES) // daily challenges
    {
        startEnabled = false;
        table->sendTableMessage("This challenge is over! To see scores or to try another challenge, exit the table and make the appropriate selections with the Challenges button.");
        if (table->playerList.size() != 1)
            qDebug() << table->playerList.size() << "More or less than 1 player in a challenge table!? WTF";
        else
        {
            // search for player.
            if (challenges.contains(wordList))
            {
                if (challenges.value(wordList).highScores->contains(table->playerList.at(0)->connData.userName.toLower()))
                    table->sendTableMessage("You've already played this challenge. These results will not count towards this day's high scores.");
                else
                {
                    if (midnightSwitchoverToggle == thisTableSwitchoverToggle)
                    {
                        highScoreData tmp;
                        tmp.userName = table->playerList.at(0)->connData.userName;
                        tmp.numSolutions = numTotalSolutions;
                        tmp.numCorrect = numTotalSolutions - gameSolutions.size();
                        tmp.timeRemaining = currentTimerVal;
                        challenges.value(wordList).highScores->insert(table->playerList.at(0)->connData.userName.toLower(), tmp);

                    }
                    else
                        table->sendTableMessage("The daily lists have changed while you were playing. Please try again with the new list!");
                }

            }
        }
    }
}


void UnscrambleGame::updateCountdownTimer()
{
    countdownTimerVal--;
    sendTimerValuePacket(countdownTimerVal);
    if (countdownTimerVal == 0)
        startGame();
}

void UnscrambleGame::updateGameTimer()
{
    currentTimerVal--;
    sendTimerValuePacket(currentTimerVal);
    if (currentTimerVal == 0)
        endGame();
}


void UnscrambleGame::generateQuizArray()
{
    if (cycleState != TABLE_TYPE_DAILY_CHALLENGES)
    {

        QTime timer;
        timer.start();

        QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
        query.setForwardOnly(true);

        query.prepare("SELECT wordlength, probindices from wordlists where listname = ? and lexiconName = ?");
        query.bindValue(0, wordList);
        query.bindValue(1, lexiconName);
        query.exec();

        QByteArray indices;
        while (query.next())
        {
            wordLength = query.value(0).toInt();
            indices = query.value(1).toByteArray();
        }

        qDebug() << "Query executed. time=" << timer.elapsed();

        QDataStream stream(indices);

        quint8 type, length;
        stream >> type >> length;
        if (type == 0)
        {
            // a range of indices
            quint32 start, end;
            stream >> start >> end;
            QVector <quint32> indexVector;
            getUniqueRandomNumbers(indexVector, start, end, end-start+1);


            for (quint32 i = start; i <= end; i++)
                quizArray << indexVector.at(i-start);

            numTotalRacks = end-start+1;
        }
        if (type == 1)
        {
            // a list of indices
            quint16 size;
            stream >> size;
            QVector <quint32> tempVector;
            QVector <quint32> indexVector;
            indexVector.resize(size);
            quint32 index;
            getUniqueRandomNumbers(tempVector, 0, size-1, size);
            for (quint16 i = 0; i < size; i++)
            {
                stream >> index;
                indexVector[tempVector.at(i)] = index;
            }

            for (quint16 i = 0; i < size; i++)
                quizArray << indexVector.at(i);

            numTotalRacks = size;
        }


        //        qDebug() << "Generated quiz array, time=" << timer.elapsed() << quizArray;

    }
    else
    {
        // daily challenges
        // just copy the daily challenge array
        qDebug() << challenges.keys();
        qDebug() << wordList;
        if (challenges.contains(wordList))
        {
            wordLength = challenges.value(wordList).wordLength;
            quizArray = challenges.value(wordList).dbIndices;
        }

        numTotalRacks = maxRacks;   // this line may be unnecessary, numTotalRacks is set in prepareTableAlphagrams before the start of the game
    }

    quizIndex = 0;
    /* point alphaInfo to the correct vector for this particular word length. this
        assumes a table can only have one word length at the moment */

    //alphaInfo = (QVector<alphagramInfo>*)&(alphagramData.at(wordLength-2));
    neverStarted = true;


}

void UnscrambleGame::prepareTableAlphagrams()
{
    // load alphagrams into the gamesolutions hash and the alphagrams list

    gameSolutions.clear();
    unscrambleGameQuestions.clear();
    alphagramIndices.clear();
    if (cycleState == TABLE_TYPE_CYCLE_MODE && quizIndex == quizArray.size())
    {
        quizArray = missedArray;		// copy missedArray to quizArray
        quizIndex = 0;	// and set the index back at the beginning
        missedArray.clear();	// also clear the missed array, cuz we're starting a new one.
        table->sendTableMessage("The list has been exhausted. Now quizzing on missed list.");

        numRacksSeen = 0;
        numTotalRacks = quizArray.size();
        //      numTotalRacks = missedFileWriter
    }
    QStringList lineList;
    QString line;
    numTotalSolutions = 0;
    QTime timer, timer2;
    timer.start();

    QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
    query.setForwardOnly(true);
    query.exec("BEGIN TRANSACTION");
    query.prepare(QString("SELECT alphagram, words from alphagrams where length = ? and lexiconName = ? and probability = ?"));
    query.bindValue(0, wordLength);
    query.bindValue(1, lexiconName);

    for (quint8 i = 0; i < maxRacks; i++)
    {
        unscrambleGameQuestionData thisQuestionData;

        if (quizIndex == quizArray.size())
        {
            thisQuestionData.alphagram = "";
            thisQuestionData.numNotYetSolved = 0;
            if (i == 0)
            {
                listExhausted = true;
                if (cycleState != TABLE_TYPE_DAILY_CHALLENGES)
                {
                    table->sendTableMessage("This list has been completely exhausted. Please exit table and have a nice day.");

                }
                else
                    table->sendTableMessage("This challenge is over. To view scores, please exit table and "
                                            "select 'Get today's scores' from the 'Challenges' button.");
            }
        }
        else
        {
            numRacksSeen++;
            quint32 index = quizArray.at(quizIndex);

            query.bindValue(2, index);
            timer2.start();
            query.exec();
            qDebug() << " singleAlpha" << timer2.restart();
            while (query.next())
            {
                thisQuestionData.alphagram = query.value(0).toString();
                QStringList sols = query.value(1).toString().split(" ");
                int size = sols.size();
                thisQuestionData.numNotYetSolved = size;
                thisQuestionData.probability = index;
                thisQuestionData.solutions = sols;
                for (int k = 0; k < size; k++)
                {
                    gameSolutions.insert(sols.at(k), thisQuestionData.alphagram);
                    numTotalSolutions++;
                }
                //qDebug() << quizIndex << index << thisQuestionData.alphagram << thisQuestionData.solutions;
            }
            quizIndex++;



        }
        unscrambleGameQuestions.append(thisQuestionData);
        alphagramIndices.insert(thisQuestionData.alphagram, i);
    }
    query.exec("END TRANSACTION");
    qDebug() << "finished PrepareTableAlphagrams, time=" << timer.elapsed();

}

void UnscrambleGame::sendUserCurrentAlphagrams(ClientSocket* socket)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << table->tableNumber;
    out << (quint8) SERVER_TABLE_ALPHAGRAMS;
    out << (quint8) maxRacks;
    for (int i = 0; i < maxRacks; i++)
    {
        out << unscrambleGameQuestions.at(i).alphagram;
        out << unscrambleGameQuestions.at(i).numNotYetSolved;
        out << unscrambleGameQuestions.at(i).solutions;
    }
    fixHeaderLength();
    socket->write(block);
}                 

void UnscrambleGame::sendTimerValuePacket(quint16 timerValue)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8)SERVER_TABLE_TIMER_VALUE;
    out << timerValue;
    fixHeaderLength();
    table->sendGenericPacket();
}

void UnscrambleGame::sendNumQuestionsPacket()
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8) SERVER_TABLE_NUM_QUESTIONS;
    out << numRacksSeen;
    out << numTotalRacks;
    fixHeaderLength();
    table->sendGenericPacket();

}

void UnscrambleGame::sendGiveUpPacket(QString username)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8)SERVER_TABLE_GIVEUP;
    out << username;
    fixHeaderLength();
    table->sendGenericPacket();
}

void UnscrambleGame::sendGuessRightPacket(QString username, QString answer, quint8 index)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8)SERVER_TABLE_GUESS_RIGHT;
    out << username << answer << index;
    fixHeaderLength();
    table->sendGenericPacket();
}

/*************** static and utility functions ****************/

void getUniqueRandomNumbers(QVector<quint32>&numbers, quint32 start, quint32 end, int numNums)
{
    // takes all the numbers between start and end, including start and end,
    // randomly shuffles, and returns the first numNums numbers of the shuffled array.

    //  qDebug() << "gurn" << start << end << numNums;
    int size = end - start + 1;
    numbers.resize(numNums);
    if (size < 1) size = start - end + 1;
    if (numNums > size) return;

    QVector <quint32> pool;
    pool.resize(size);
    for (int i = 0; i < pool.size(); i++)
    {
        pool[i] = i + start;
    }
    int choose, temp;
    for (int i = 0; i < numbers.size(); i++)
    {
        choose = qrand() % size;
        numbers[i] = pool[choose];
        size--;
        temp = pool[choose];
        pool[choose] = pool[size];
        pool[size] = temp;
    }
}

void UnscrambleGame::loadWordLists()
{
    ListMaker::createListDatabase();	// also connects to the databases.
// TODO this function must be rewritten, we are connecting to many databases.
    QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
    query.exec("SELECT listname, lexiconName from wordlists");
    QList <WordList> orderedWordLists;
    orderedWordLists.clear();
    while (query.next())
        orderedWordLists << WordList(query.value(0).toString(), LIST_TYPE_REGULAR, query.value(1).toString());

    writeHeaderData();
    out << (quint8) SERVER_WORD_LISTS;		// word lists
    out << (quint8) ListMaker::lexiconMap.size();
    foreach(QString lexiconName, ListMaker::lexiconMap.keys())
        out << lexiconName.toAscii(); // TODO fix
    out << (quint8) 2;           // two types right now, regular, and challenge.
    out << (quint8) 'R';	// regular
    out << (quint16)orderedWordLists.size();
    foreach (WordList wl, orderedWordLists)
    {
        // TODO fix
        /*out << (quint8)ListMaker::lexiconList.indexOf(wl.lexiconName) << wl.name.toAscii();*/
    }
    out << (quint8) 'D';/* //TODO FIX
    out << (quint16) (14*ListMaker::lexiconList.size()); // 14 per lexicon
    for (int i = 0; i < ListMaker::lexiconList.size(); i++)
    {

        for (int j = 2; j <= 15; j++)
            out << (quint8)i << QString("Today's " + ListMaker::lexiconList[i] + " %1s").arg(j).toAscii();
    }*/
    fixHeaderLength();
    wordListDataToSend = block; // copy to a block so that we don't have to recompute this everytime someone logs in
}

void UnscrambleGame::sendLists(ClientSocket* socket)
{
    socket->write(wordListDataToSend);
}



void UnscrambleGame::generateDailyChallenges()
{
    /* test randomness of algorithm */

    /*
    QVector <int> numFreq;
    int low = 0, high = 499;
    int poolSize = high-low+1;
    int subsetSize = 50;
    numFreq.resize(poolSize);
    numFreq.fill(0, poolSize);

    for (int i = 0; i < 1000000; i++)
    {
        QVector <quint16> nums;
        getUniqueRandomNumbers(nums, low, high, subsetSize);
        for (int j = 0; j < subsetSize; j++)
            numFreq[nums[j]]++;
    }
    qDebug() << numFreq;
    //
    return;
    */
    midnightSwitchoverToggle = !midnightSwitchoverToggle;

    QList <challengeInfo> vals = challenges.values();
    foreach (challengeInfo ci, vals)
        delete ci.highScores;

    challenges.clear();
    QTime timer;

    QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
    query.exec("BEGIN TRANSACTION");
    /*for (int j = 0; j < ListMaker::lexiconList.size(); j++)
    {
        for (int i = 2; i <= 15; i++)
        {
            challengeInfo tmpChallenge;
            tmpChallenge.highScores = new QHash <QString, highScoreData>;

            QString challengeName = QString("Today's " + ListMaker::lexiconList[j] + " %1s").arg(i);

            query.exec(QString("SELECT numalphagrams from wordlists where listname = '" + ListMaker::lexiconList[j] + " %1s'").arg(i));
            int wordCount = 0;
            while (query.next())
            {
                wordCount = query.value(0).toInt();
            }
            if (wordCount != 0)
            {
                getUniqueRandomNumbers(tmpChallenge.dbIndices, 1+(i<<24), wordCount+(i<<24), qMin(wordCount, 50));
                qDebug() << "generated" + challengeName;
                qDebug() << tmpChallenge.dbIndices;
                tmpChallenge.wordLength = i;
                challenges.insert(challengeName, tmpChallenge);
            }
        }
    }
    query.exec("END TRANSACTION");
*/ // TODO FIX
}


