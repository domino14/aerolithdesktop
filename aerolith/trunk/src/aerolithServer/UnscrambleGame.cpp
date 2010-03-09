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
#include "databasehandler.h"
extern QByteArray block;
extern QDataStream out;

const quint8 COUNTDOWN_TIMER_VAL = 3;
const quint8 maxRacks = 50;
QHash <QString, challengeInfo> UnscrambleGame::challenges;
bool UnscrambleGame::midnightSwitchoverToggle;
int UnscrambleGame::numTotalQuestions = 0;

QByteArray UnscrambleGame::wordListDataToSend;
//QVector<QVector <QVector<alphagramInfo> > > UnscrambleGame::alphagramData;


UnscrambleGame::UnscrambleGame(Table* table) : TableGame(table)
{
    qDebug() << "UnscrambleGame constructor";
}

QByteArray UnscrambleGame::initialize(DatabaseHandler* dbHandler)
{
    this->dbHandler = dbHandler;

    wroteToMissedFileThisRound = false;

    quint8 tableTimerValMin;

    table->host->connData.in >> listType;   // TODO need to check to make sure client can't send malicious packet
    // that lacks some of these fields!

    if (listType == LIST_TYPE_NAMED_LIST)
    {
        table->host->connData.in >> wordList;
    }
    else if (listType == LIST_TYPE_INDEX_RANGE_BY_WORD_LENGTH)
    {
        table->host->connData.in >> wordLength >> lowProbIndex >> highProbIndex;
        wordList = QString("The %1s: %2 to %3").
                   arg(wordLength).arg(lowProbIndex).arg(highProbIndex);
    }
    else if (listType == LIST_TYPE_ALL_WORD_LENGTH)
    {
        table->host->connData.in >> wordLength;
        wordList = QString("The %1s").arg(wordLength);
    }
    else if (listType == LIST_TYPE_DAILY_CHALLENGE)
    {
        table->host->connData.in >> wordList;
    }
    else if (listType == LIST_TYPE_USER_LIST)
    {
        table->host->connData.in >> wordList;
        table->host->connData.in >> quizSet;  /* TODO need some sanity checks on these inputs! */
        table->host->connData.in >> missedSet;
        wordList = "(User list) " + wordList;
    }

    table->host->connData.in >> lexiconName >> cycleState >> tableTimerValMin;

    if (dbHandler->availableDatabases.contains(lexiconName))
    {
        wordDbConName = lexiconName + "DB_server";  // this is how it is in databaseHandler. todo perhaps should make this a variable.

    }


    connect(&gameTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));
    connect(&countdownTimer, SIGNAL(timeout()), this, SLOT(updateCountdownTimer()));

    gameStarted = false;
    countingDown = false;
    startEnabled = true;
    tableTimerVal = (quint16)tableTimerValMin * (quint16)60;
    currentTimerVal = tableTimerVal;
    countdownTimerVal = COUNTDOWN_TIMER_VAL;

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



    // compute array to be sent out as table information array
    writeHeaderData();
    out << (quint8) SERVER_NEW_TABLE;
    out << (quint16) table->tableNumber;
    out << (quint8) GAME_TYPE_UNSCRAMBLE;
    out << lexiconName;
    out << wordList;
    out << table->maxPlayers;
    out << table->isPrivate;
    fixHeaderLength();

    return block;



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
        sendUserCurrentQuestions(client);

    // possibly send scores, solved solutions, etc. in the future.
    if (table->peopleInTable.size() == 1 && table->maxPlayers == 1 && cycleState != TABLE_TYPE_DAILY_CHALLENGES)
    {
        qDebug() << "playerJoined";


    }


}

void UnscrambleGame::cleanupBeforeDelete()
{
    numTotalQuestions -= thisMaxQuestions;
}

void UnscrambleGame::playerLeftGame(ClientSocket* socket)
{
    // if this is the ONLY player, stop game.
    if (table->peopleInTable.size() == 1 && table->maxPlayers == 1)
    {
        gameEndRequest(socket);
    }

    if (socket == table->originalHost)
    {
        if (listType == LIST_TYPE_USER_LIST)
        {
            /* the host is no longer able to send out indices to the table.  inform the players of this */
            table->sendTableMessage("The original host of the table has left. "
                                    "Since the host was the one to create this list, you cannot quiz on it any longer.");


        }
    }
}

void UnscrambleGame::gameStartRequest(ClientSocket* client)
{
    if (!client->connData.isSitting) return;    // can't request to start the game if you're standing
    if (startEnabled == true && gameStarted == false && countingDown == false)
    {
        bool startTheGame = true;

        sendReadyBeginPacket(client->connData.seatNumber);
        client->playerData.readyToPlay = true;

        foreach (ClientSocket* thisClient, table->sittingList)
        {
            if (thisClient && thisClient->playerData.readyToPlay == false)
            {
                startTheGame = false;
                break;
            }
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
                    if (table->originalHost)
                    {
                        if (challenges.value(wordList).
                            highScores->contains(table->originalHost->connData.userName.toLower()))
                            table->sendTableMessage(table->originalHost->connData.userName + " has already played this "
                                                    "challenge. You can play again, "
                                                    "but only the first game's results count toward today's high scores!");
                    }
                }
            }

        }
    }

}


void UnscrambleGame::correctAnswerSent(ClientSocket* socket, quint8 space, quint8 specificAnswer)
{
    if (socket->connData.isSitting)
    {
        qDebug() << "cas" << space << specificAnswer;
        if (space < numRacksThisRound)
        {
            qDebug() << "  " << unscrambleGameQuestions[space].numNotYetSolved << unscrambleGameQuestions[space].notYetSolved;
            if (unscrambleGameQuestions[space].notYetSolved.contains(specificAnswer))
            {
                unscrambleGameQuestions[space].notYetSolved.remove(specificAnswer);
                unscrambleGameQuestions[space].numNotYetSolved--;
                numTotalSolvedSoFar++;

                sendCorrectAnswerPacket(socket->connData.seatNumber, space, specificAnswer);
                if (numTotalSolvedSoFar == numTotalSolutions) endGame();
            }
        }
    }
}

void UnscrambleGame::gameEndRequest(ClientSocket* socket)
{
    if (gameStarted == true)
    {
        bool giveUp = true;

        sendGiveUpPacket(socket->connData.userName);
        socket->playerData.gaveUp = true;

        foreach (ClientSocket* thisSocket, table->sittingList)
        {
            if (thisSocket && thisSocket->playerData.gaveUp == false)
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
    prepareTableQuestions();
    sendGameStartPacket();

    foreach (ClientSocket* socket, table->peopleInTable)
        sendUserCurrentQuestions(socket);


    sendTimerValuePacket(tableTimerVal);
    sendNumQuestionsPacket();
    currentTimerVal = tableTimerVal;
    gameTimer.start(1000);
    thisTableSwitchoverToggle = midnightSwitchoverToggle;


}

void UnscrambleGame::performSpecificSitActions(ClientSocket* socket)
{
    socket->playerData.gaveUp = false;
    socket->playerData.readyToPlay = false;
    socket->playerData.score = 0;
}

void UnscrambleGame::performSpecificStandActions(ClientSocket* socket)
{
    socket->playerData.gaveUp = false;
    socket->playerData.readyToPlay = false;
    socket->playerData.score = 0;
}

void UnscrambleGame::endGame()
{
    gameTimer.stop();
    gameStarted = false;

    sendGameEndPacket();
    sendTimerValuePacket((quint16)0);

    foreach (ClientSocket* client, table->peopleInTable)
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
            for (int i = 0; i < numRacksThisRound; i++)
            {
                if (unscrambleGameQuestions.at(i).numNotYetSolved > 0)
                {
                    missedArray << unscrambleGameQuestions.at(i).probIndex;
                }

            }
        }
        qDebug() << "Missed to date:" << missedArray.size() << "racks.";
    }
    else if (cycleState == TABLE_TYPE_DAILY_CHALLENGES) // daily challenges
    {
        startEnabled = false;
        table->sendTableMessage("This challenge is over! To see scores or to try another challenge, exit the table "
                                "and make the appropriate selections with the Challenges button.");

        // search for player.
        if (table->originalHost && challenges.contains(wordList))
        {
            if (challenges.value(wordList).highScores->contains(table->originalHost->connData.userName.toLower()))
                table->sendTableMessage(table->originalHost->connData.userName + " has already played this challenge. "
                                        "These results will not count towards this day's high scores.");
            else
            {
                if (midnightSwitchoverToggle == thisTableSwitchoverToggle)
                {
                    highScoreData tmp;
                    tmp.userName = table->originalHost->connData.userName;
                    tmp.numSolutions = numTotalSolutions;
                    tmp.numCorrect = numTotalSolvedSoFar;
                    tmp.timeRemaining = currentTimerVal;
                    challenges.value(wordList).highScores->insert(table->originalHost->connData.userName.toLower(), tmp);

                }
                else
                    table->sendTableMessage("The daily lists have changed while you were playing. Please try again with the new list!");
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

        QByteArray indices;

        if (listType == LIST_TYPE_NAMED_LIST)
        {

            QSqlQuery query(QSqlDatabase::database(wordDbConName));
            query.setForwardOnly(true);

            query.prepare("SELECT probindices from wordlists where listname = ?");
            query.bindValue(0, wordList);
            query.exec();

            while (query.next())
            {
                indices = query.value(0).toByteArray();
            }
        }
        else if (listType == LIST_TYPE_INDEX_RANGE_BY_WORD_LENGTH)
        {  
            lowProbIndex += (wordLength << 24);
            highProbIndex += (wordLength << 24);    // this is the encoding we use for probability.

            QDataStream stream(&indices, QIODevice::WriteOnly);
            stream << (quint8)0 << wordLength << lowProbIndex << highProbIndex;

            // TODO maybe it'll be more efficient to process numbers directly rather
            // than packing into a QByteArray just to immediately unpack below!
        }
        else if (listType == LIST_TYPE_ALL_WORD_LENGTH)
        {
            QDataStream stream(&indices, QIODevice::WriteOnly);

            lowProbIndex = 1 + (wordLength << 24);  // probability 1 is the first
            if (wordLength >=2 && wordLength <= 15)
                highProbIndex = dbHandler->lexiconMap.value(lexiconName).alphagramsPerLength[wordLength] + (wordLength << 24);

            stream << (quint8)0 << wordLength << lowProbIndex << highProbIndex;
            qDebug() << "all" << wordLength << lowProbIndex << highProbIndex;
        }

        if (listType != LIST_TYPE_USER_LIST)
        {

            QDataStream stream(indices);

            quint8 type, length;
            stream >> type >> length;   /*TODO remove length from the parameters */
            if (type == 0)
            {
                // a range of indices
                quint32 start, end;
                stream >> start >> end;
                qDebug() << "Type0" << start << end;
                QVector <quint32> indexVector;
                getUniqueRandomNumbers(indexVector, start, end, end-start+1);


                for (quint32 i = start; i <= end; i++)
                    quizArray << indexVector.at(i-start);

                numTotalRacks = end-start+1;
            }
            if (type == 1)
            {
                // a list of indices
                quint32 size;
                stream >> size;
                QVector <quint32> tempVector;
                QVector <quint32> indexVector;
                indexVector.resize(size);
                quint32 index;
                getUniqueRandomNumbers(tempVector, 0, size-1, size);
                for (quint32 i = 0; i < size; i++)
                {
                    stream >> index;
                    indexVector[tempVector.at(i)] = index;
                }

                for (quint32 i = 0; i < size; i++)
                    quizArray << indexVector.at(i);

                numTotalRacks = size;
            }


            qDebug() << "Generated quiz array, time=" << timer.elapsed();
            qDebug() << " array size" << quizArray.size();
            //qDebug() << quizArray;
        }
        else    // multiple_indices
        {
            quint32 size = quizSet.size();   // already assigned earlier in initialize()

            QVector <quint32> tempVector;
            QVector <quint32> indexVector;
            indexVector.resize(size);

            getUniqueRandomNumbers(tempVector, 0, size-1, size);
            QList <quint32> setVals = quizSet.values();
            for (quint32 i = 0; i < size; i++)
            {
                indexVector[tempVector.at(i)] = setVals[i];
            }
            quizArray.resize(size);
            for (quint32 i = 0; i < size; i++)
                quizArray[i] = indexVector.at(i);

            missedArray = missedSet.values().toVector();
            numTotalRacks = size;

        }

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

    neverStarted = true;
    thisMaxQuestions = missedArray.size() + quizArray.size();
    numTotalQuestions += thisMaxQuestions;

}

void UnscrambleGame::prepareTableQuestions()
{
    unscrambleGameQuestions.clear();

    numTotalSolutions = 0;
    numTotalSolvedSoFar = 0;
    if (cycleState == TABLE_TYPE_CYCLE_MODE && quizIndex == quizArray.size())
    {
        quizArray = missedArray;		// copy missedArray to quizArray
        quizIndex = 0;	// and set the index back at the beginning
        missedArray.clear();	// also clear the missed array, cuz we're starting a new one.
        table->sendTableMessage("The list has been exhausted. Now quizzing on missed list.");
        sendListExhaustedMessage();
        numRacksSeen = 0;
        numTotalRacks = quizArray.size();
    }

    QTime timer;
    timer.start();

    QSqlQuery query(QSqlDatabase::database(wordDbConName));

    query.exec("BEGIN TRANSACTION");
    query.prepare(QString("SELECT words from alphagrams where probability = ?"));
    numRacksThisRound = 0;

    for (quint8 i = 0; i < maxRacks; i++)
    {
        UnscrambleGameQuestionData thisQuestionData;

        if (quizIndex == quizArray.size())
        {
            if (i == 0)
            {
                if (cycleState != TABLE_TYPE_DAILY_CHALLENGES)
                {
                    table->sendTableMessage("This list has been completely exhausted. Please exit table and have a nice day.");
                    sendListCompletelyExhaustedMessage();
                }
                else
                {
                    table->sendTableMessage("This challenge is over. To view scores, please exit table and "
                                            "select 'Get today's scores' from the 'Challenges' button.");
                    sendListCompletelyExhaustedMessage();
                }
            }
            break;
        }
        else
        {
            numRacksSeen++;
            numRacksThisRound++;
            quint32 index = quizArray.at(quizIndex);

            query.bindValue(0, index);
            query.exec();

            while (query.next())
            {
                QStringList sols = query.value(0).toString().split(" ");
                int size = sols.size();
                thisQuestionData.numNotYetSolved = size;
                thisQuestionData.probIndex = index;
                for (int k = 0; k < size; k++)
                {
                    thisQuestionData.notYetSolved.insert(k);
                    numTotalSolutions++;
                }
            }
            quizIndex++;
            thisQuestionData.exists = true;
        }
        unscrambleGameQuestions.append(thisQuestionData);
    }
    query.exec("END TRANSACTION");
    qDebug() << "finished PrepareTableQuestions, time=" << timer.elapsed();

}

void UnscrambleGame::sendUserCurrentQuestions(ClientSocket* socket)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << table->tableNumber;
    out << (quint8) SERVER_TABLE_QUESTIONS;
    out << (quint8) numRacksThisRound;
    for (int i = 0; i < numRacksThisRound; i++)
    {
        out << unscrambleGameQuestions.at(i).probIndex;
        out << unscrambleGameQuestions.at(i).numNotYetSolved;
        foreach (quint8 notSolved, unscrambleGameQuestions.at(i).notYetSolved)
            out << notSolved;
    }
    fixHeaderLength();
    socket->write(block);
}

void UnscrambleGame::sendListExhaustedMessage()
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << table->tableNumber;
    out << (quint8) SERVER_TABLE_UNSCRAMBLEGAME_MAIN_QUIZ_DONE;
    fixHeaderLength();
    table->sendGenericPacket();
}

void UnscrambleGame::sendListCompletelyExhaustedMessage()
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << table->tableNumber;
    out << (quint8) SERVER_TABLE_UNSCRAMBLEGAME_FULL_QUIZ_DONE;
    fixHeaderLength();
    table->sendGenericPacket();
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

void UnscrambleGame::sendCorrectAnswerPacket(quint8 seatNumber, quint8 space, quint8 specificAnswer)
{
    writeHeaderData();
    out << (quint8) SERVER_TABLE_COMMAND;
    out << (quint16) table->tableNumber;
    out << (quint8) SERVER_TABLE_CORRECT_ANSWER;
    out << seatNumber << space << specificAnswer;
    fixHeaderLength();
    table->sendGenericPacket();
}

void UnscrambleGame::handleMiscPacket(ClientSocket* socket, quint8 header)
{
    switch (header)
    {
    case CLIENT_TABLE_UNSCRAMBLEGAME_CORRECT_ANSWER:
        {
            quint8 space, specificAnswer;
            socket->connData.in >> space >> specificAnswer;
            correctAnswerSent(socket, space, specificAnswer);

        }
        break;
    default:
        socket->disconnectFromHost();


    }


}


/*************** static and utility functions ****************/



void UnscrambleGame::loadWordLists(DatabaseHandler* dbHandler)
{


    writeHeaderData();
    out << (quint8) SERVER_WORD_LISTS;		// word lists
    out << (quint8) dbHandler->availableDatabases.size();
    foreach(QString lexiconName, dbHandler->availableDatabases)
        out << lexiconName.toAscii();
    out << (quint8) 2;           // two types right now, regular, and challenge.
    out << (quint8) 'R';	// regular

    for (quint8 i = 0; i < dbHandler->availableDatabases.size(); i++)
    {
        QSqlQuery query(dbHandler->lexiconMap.value(dbHandler->availableDatabases[i]).db);
        query.exec("SELECT listname from wordlists");
        QStringList wordLists;
        while (query.next())
            wordLists << query.value(0).toString();

        out << i;   // lexicon index
        out << (quint16)wordLists.size();
        foreach (QString wordlistName, wordLists)
            out << wordlistName.toAscii();
    }

    out << (quint8) 'D';
    for (quint8 i = 0; i < dbHandler->availableDatabases.size(); i++)
    {
        out << i;
        if (dbHandler->availableDatabases.at(i) != "Volost")
        {
            out << (quint16)14;
            for (int j = 2; j <= 15; j++)
                out << QString("Today's " + dbHandler->availableDatabases.at(i) + " %1s").arg(j).toAscii();
        }
        else
        {
            out << (quint16)2;
            out << QString("Today's Volost 7s").toAscii();
            out << QString("Today's Volost 8s").toAscii();
        }
    }
    fixHeaderLength();
    wordListDataToSend = block; // copy to a block so that we don't have to recompute this everytime someone logs in
}

void UnscrambleGame::sendLists(ClientSocket* socket)
{
    socket->write(wordListDataToSend);
}



void UnscrambleGame::generateDailyChallenges(DatabaseHandler* dbHandler)
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

    for (quint8 i = 0; i < dbHandler->availableDatabases.size(); i++)
    {
        QSqlQuery query(dbHandler->lexiconMap.value(dbHandler->availableDatabases[i]).db);
        query.exec("BEGIN TRANSACTION");
        for (int j = 2; j <= 15; j++)
        {
            query.exec(QString("SELECT count(*) from alphagrams where length = %1").arg(j));
            int wordCount = 0;
            while (query.next())
            {
                wordCount = query.value(0).toInt();
            }
            if (wordCount != 0)
            {
                challengeInfo tmpChallenge;
                tmpChallenge.highScores = new QHash <QString, highScoreData>;
                getUniqueRandomNumbers(tmpChallenge.dbIndices, 1 + (j << 24), wordCount+(j << 24), qMin(wordCount, 50));
                tmpChallenge.wordLength = j;
                QString challengeName = QString("Today's " + dbHandler->availableDatabases.at(i) + " %1s").arg(j);
                challenges.insert(challengeName, tmpChallenge);
                qDebug() << "Generated" << challengeName << tmpChallenge.dbIndices;
            }

        }
        query.exec("END TRANSACTION");
    }
    qDebug() << "Generated daily challenges!";
}


