#include "UnscrambleGame.h"
#include "commonDefs.h"

extern QByteArray block;
extern QDataStream out;

const quint8 COUNTDOWN_TIMER_VAL = 3;
const quint8 maxRacks = 50;
QHash <QString, challengeInfo> UnscrambleGame::challenges;
bool UnscrambleGame::midnightSwitchoverToggle;
extern const QString WORD_DATABASE_NAME; 

QVector <QVector<alphagramInfo> > UnscrambleGame::alphagramData;

void UnscrambleGame::initialize(quint8 cycleState, quint8 tableTimer, QString wordList)
{

	this->wordList = wordList;

	tempFileExists = false;
	this->cycleState = cycleState;

	gameTimer = new QTimer();
	countdownTimer = new QTimer();

	connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));
	connect(countdownTimer, SIGNAL(timeout()), this, SLOT(updateCountdownTimer()));

	gameStarted = false;
	countingDown = false;
	startEnabled = true;
	tableTimerVal = (quint16)tableTimer * (quint16)60;
	currentTimerVal = tableTimerVal;
	countdownTimerVal = COUNTDOWN_TIMER_VAL;

	if (cycleState == TABLE_TYPE_DAILY_CHALLENGES)
	{

		tableTimerVal = 270;	// 4.5 minutes for 50 words - server decides time for challenges!
	}
	/*

	*/
	numRacksSeen = 0;
	numTotalRacks = 0;
	numMissedRacks = 0;
}

UnscrambleGame::UnscrambleGame(tableData* table) : TableGame(table)
{
	qDebug() << "UnscrambleGame constructor";
}

UnscrambleGame::~UnscrambleGame()
{
	qDebug() << "UnscrambleGame destructor";
//	endGame();
	gameTimer->deleteLater();
	countdownTimer->deleteLater();
}

void UnscrambleGame::playerJoined(ClientSocket* client)
{
	if (gameStarted)
		sendUserCurrentAlphagrams(client);

	// possibly send scores, solved solutions, etc. in the future.
}

void UnscrambleGame::playerLeftGame(ClientSocket* socket)
{
	// if this is the ONLY player, stop game.
	if (table->playerList.size() == 1)
	{
		endGame();
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
				countdownTimer->start(1000);
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
			if (thisSocket->playerData.gaveUp == false)
			{
				giveUp = false;
				break;
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
	countdownTimer->stop();
	// code for starting the game
	// steps:
	//1. list should already be loaded when table was created
	//2. send to everyone @ the table:  
	//    - maxRacks alphagrams  
	gameStarted = true;
	prepareTableAlphagrams();
	foreach (ClientSocket* socket, table->playerList)
		sendUserCurrentAlphagrams(socket);

	sendGameStartPacket();
	sendTimerValuePacket(tableTimerVal);
	sendNumQuestionsPacket();
	currentTimerVal = tableTimerVal;
	gameTimer->start(1000);
	thisTableSwitchoverToggle = midnightSwitchoverToggle;


}

void UnscrambleGame::endGame()
{
	gameTimer->stop();
	gameStarted = false;
	// TODO here is code for the game ending
	// send game ended to everyone, etc.
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
		for (int i = 0; i < maxRacks; i++)
		{	
			if (unscrambleGameQuestions.at(i).numNotYetSolved > 0)
			{
				// this one has not been solved!
				missedFileWriter << unscrambleGameQuestions.at(i).alphagram;
				for (int j = 0; j < unscrambleGameQuestions.at(i).solutions.size(); j++)
					missedFileWriter << " " << unscrambleGameQuestions.at(i).solutions.at(j);
				missedFileWriter << "\n";
				//qDebug() << "wrote" << unscrambleGameQuestions.at(i).alphagram << unscrambleGameQuestions.at(i).solutions << " to missed file.";
				numMissedRacks++;
			}

		}
		qDebug() << "Missed to date:" << numMissedRacks << "racks.";
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

void getUniqueRandomNumbers(QVector<quint16>&numbers, quint16 start, quint16 end, quint16 numNums)
{
	quint16 size = end - start + 1;
	numbers.resize(numNums);
	if (size < 1) size = start - end + 1;
	if (numNums > size) return;
	
	QVector <quint16> pool;
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

void UnscrambleGame::prepareWordDataStructure()
{
	// loads all the words from words.db into a data structure that's arranged by probability.
	QTime timer;
	timer.start();
	
	alphagramData.resize(14);	// for lengths 2 thru 15
	QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
	for (int i = 2; i <= 15; i++)
	{
		query.exec(QString("SELECT alphagram, words from alphagrams where length = %1 order by probability").arg(i));
		while (query.next())
		{
			alphagramData[i-2].append(alphagramInfo(query.value(0).toString(), query.value(1).toString()));
		}
	
	}
	
	qDebug() << "Created data structure, time=" << timer.elapsed();
	
	
}


void UnscrambleGame::generateDailyChallenges()
{
	midnightSwitchoverToggle = !midnightSwitchoverToggle;

	QList <challengeInfo> vals = challenges.values();
	foreach (challengeInfo ci, vals)
		delete ci.highScores;

	challenges.clear();
	QTime timer;
	QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
	for (int i = 2; i <= 15; i++)
	{
		timer.start();
		challengeInfo tmpChallenge;
		tmpChallenge.highScores = new QHash <QString, highScoreData>;
		QString challengeName = QString("Today's %1s").arg(i);
		

		query.exec(QString("SELECT numalphagrams from wordlists where listname = 'OWL2 %1s'").arg(i));
		int wordCount = 0;
		while (query.next())
		{
			wordCount = query.value(0).toInt();
		}
		getUniqueRandomNumbers(tmpChallenge.dbIndices, 1, wordCount, 50);
		qDebug() << QString("generated today's %1s").arg(i);
		qDebug() << tmpChallenge.dbIndices;
		tmpChallenge.wordLength = i;
		challenges.insert(challengeName, tmpChallenge);
		qDebug() << "elapsed time" << timer.elapsed();
	}
	
}

void UnscrambleGame::generateTempFile()
{
	if (cycleState != TABLE_TYPE_DAILY_CHALLENGES)
	{

		QTime timer;
		timer.start();
		
		QFile tempOutFile(QString("temp%1").arg(table->tableNumber));
		QTextStream outStream(&tempOutFile);
		tempOutFile.open(QIODevice::WriteOnly | QIODevice::Text);
		
		QSqlQuery query(QSqlDatabase::database(WORD_DATABASE_NAME));
		query.exec(QString("SELECT wordlength, probindices from wordlists where listname = '" + wordList + "'"));
		int wordlength;
		QByteArray indices;
		while (query.next())
		{
			wordlength = query.value(0).toInt();
			indices = query.value(1).toByteArray();
		}
		
		QDataStream stream(indices);
	
		quint8 type, length;
		stream >> type >> length;
		if (type == 0)
		{
			// a range of indices
			quint16 start, end;
			stream >> start >> end;
			QVector <quint16> indexVector;
			getUniqueRandomNumbers(indexVector, start, end, end-start+1);
			const QVector <alphagramInfo> *alphaInfo = &(alphagramData.at(length-2));
			
			for (quint16 i = start; i <= end; i++)
				outStream << alphaInfo->at(indexVector.at(i-start)-1).alphagram << " " 
				<< alphaInfo->at(indexVector.at(i-start)-1).solutions << "\n";

			numTotalRacks = end-start+1;
			numMissedRacks = 0;
			tempOutFile.close();
			tempFileExists = true;
		}
		if (type == 1)
		{
			// a list of indices
			quint16 size;
			stream >> size;
			QVector <quint16> tempVector;
			QVector <quint16> indexVector;
			indexVector.resize(size);
			quint16 index;
			getUniqueRandomNumbers(tempVector, 0, size-1, size);
			for (quint16 i = 0; i < size; i++)
			{
				stream >> index;
				indexVector[tempVector.at(i)] = index;
			}
			const QVector <alphagramInfo> *alphaInfo = &(alphagramData.at(length-2));
			
			for (quint16 i = 0; i < size; i++)
				outStream << alphaInfo->at(indexVector.at(i)-1).alphagram << " " 
				<< alphaInfo->at(indexVector.at(i)-1).solutions << "\n";

			numTotalRacks = size;
			numMissedRacks = 0;
			tempOutFile.close();
			tempFileExists = true;
			
			
		}
	
		
		qDebug() << "Generated temp file, time=" << timer.elapsed();

	}
	else
	{
		// daily challenges
		// don't need to 'generate temp file'. just copy the daily challenge from the lists
		
		QFile tempOutFile(QString("temp%1").arg(table->tableNumber));
		QTextStream outStream(&tempOutFile);
		tempOutFile.open(QIODevice::WriteOnly | QIODevice::Text);
		
		if (challenges.contains(wordList))
		{
			const QVector <quint16>* dbIndices = &(challenges.value(wordList).dbIndices);
			int wordLength = challenges.value(wordList).wordLength;
			const QVector <alphagramInfo> *alphaInfo = &(alphagramData.at(wordLength-2));
			
			for(quint16 i = 0; i < dbIndices->size(); i++)
			{
				outStream << alphaInfo->at(dbIndices->at(i)-1).alphagram << " " << 
				alphaInfo->at(dbIndices->at(i)-1).solutions << "\n";
			}
		}
		tempOutFile.close();
		tempFileExists = true;
		numTotalRacks = maxRacks;
	}
}
void UnscrambleGame::prepareTableAlphagrams()
{
	// load alphagrams into the gamesolutions hash and the alphagrams list
	if (cycleState == TABLE_TYPE_RANDOM_MODE) tempFileExists = false;
	if (tempFileExists == false)
	{
		inFile.close();
		outFile.close();
		generateTempFile();
		inFile.setFileName(QString("temp%1").arg(table->tableNumber));
		alphagramReader.setDevice(&inFile);
		inFile.open(QIODevice::ReadWrite | QIODevice::Text);
		alphagramReader.seek(0);
		// generate missed file
		outFile.setFileName(QString("missed%1").arg(table->tableNumber));
		outFile.open(QIODevice::WriteOnly | QIODevice::Text);
		missedFileWriter.setDevice(&outFile);
		missedFileWriter.seek(0);
	}
	gameSolutions.clear();
	unscrambleGameQuestions.clear();
	alphagramIndices.clear();
	if (cycleState == TABLE_TYPE_CYCLE_MODE && alphagramReader.atEnd())
	{
		inFile.close();
		inFile.remove();				// delete temp file
		outFile.close();
		outFile.copy(QString("temp%1").arg(table->tableNumber));		// copy missed file to tmp file
		outFile.remove();
		inFile.setFileName(QString("temp%1").arg(table->tableNumber));	// reopen new tmp file
		outFile.setFileName(QString("missed%1").arg(table->tableNumber));	// and generate a new missed file
		alphagramReader.setDevice(&inFile);
		missedFileWriter.setDevice(&outFile);
		inFile.open(QIODevice::ReadOnly | QIODevice::Text);
		outFile.open(QIODevice::WriteOnly | QIODevice::Text);
		missedFileWriter.seek(0);
		alphagramReader.seek(0);
		table->sendTableMessage("The list has been exhausted. Now quizzing on missed list.");
		// CLOSE table->inFile
		// COPY missed%1 to tmp%1
		// repeat code above to load the file into memory, shuffle, etc
		numRacksSeen = 0;
		numTotalRacks = numMissedRacks;
		numMissedRacks = 0;
		//      numTotalRacks = missedFileWriter
	}
	QStringList lineList;
	QString line;
	numTotalSolutions = 0;
	for (quint8 i = 0; i < maxRacks; i++)
	{
		unscrambleGameData thisGameData;
		thisGameData.index = i;
		if (alphagramReader.atEnd())
		{
			thisGameData.alphagram = "";
			thisGameData.numNotYetSolved = 0;
			if (i == 0)
			{
				if (cycleState != TABLE_TYPE_DAILY_CHALLENGES)
					table->sendTableMessage("This list has been completely exhausted. Please exit table and have a nice day.");
				else
					table->sendTableMessage("This challenge is over. \
											To view scores, please exit table and select 'Get today's scores' from the 'Challenges' button.");
			}
		}
		else
		{
			numRacksSeen++;
			line = alphagramReader.readLine();
			lineList = line.split(" ");
			thisGameData.alphagram = lineList.at(0);
			thisGameData.numNotYetSolved = (quint8)(lineList.size()-1);
			for (int k = 1; k < lineList.size(); k++)
			{
				gameSolutions.insert(lineList.at(k), lineList.at(0));
				thisGameData.solutions << lineList.at(k);
				numTotalSolutions++;
			}
		}
		unscrambleGameQuestions.append(thisGameData);
		alphagramIndices.insert(thisGameData.alphagram, i);
	}
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
