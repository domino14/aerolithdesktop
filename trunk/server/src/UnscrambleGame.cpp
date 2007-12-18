#include "UnscrambleGame.h"

extern QByteArray block;
extern QDataStream out;

const quint8 COUNTDOWN_TIMER_VAL = 3;
const quint8 maxRacks = 50;
QHash <QString, challengeInfo> UnscrambleGame::challenges;
bool UnscrambleGame::midnightSwitchoverToggle;

void UnscrambleGame::initialize(quint8 cycleState, quint8 tableTimer, QString wordListFileName)
{

	this->wordListFileName = wordListFileName;

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

	if (cycleState == 3)
	{
		QString temp = wordListFileName;
		temp.chop(1);
		//		wordLengths = temp.right(1).toInt();
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
	gameTimer->deleteLater();
	countdownTimer->deleteLater();
}

void UnscrambleGame::playerJoined(ClientSocket* client)
{
	if (gameStarted)
		sendUserCurrentAlphagrams(client);

	// possibly send scores, solved solutions, etc. in the future.
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
				if (cycleState == 3)
				{				
					if (challenges.contains(wordListFileName))
					{
						if (challenges.value(wordListFileName).
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
<<<<<<< .mine
	qDebug() << " ->GUESS" << socket->connData.userName << guess;
=======
	 qDebug() << " ->GUESS" << socket->connData.userName << guess;
>>>>>>> .r199


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
	if (cycleState == 1)
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
	else if (cycleState == 3) // daily challenges
	{
		startEnabled = false;
		table->sendTableMessage("This challenge is over! To see scores or to try another challenge, exit the table and make the appropriate selections with the Challenges button.");
		if (table->playerList.size() != 1)
			qDebug() << table->playerList.size() << "More or less than 1 player in a challenge table!? WTF";
		else
		{
			// search for player. 
			if (challenges.contains(wordListFileName))
			{
				if (challenges.value(wordListFileName).highScores->contains(table->playerList.at(0)->connData.userName.toLower()))
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
						challenges.value(wordListFileName).highScores->insert(table->playerList.at(0)->connData.userName.toLower(), tmp);
						
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

void UnscrambleGame::generateDailyChallenges()
{
	midnightSwitchoverToggle = !midnightSwitchoverToggle;
	QDir dir(".");
	if (!dir.exists("dailylists"))
	{
		qDebug() << "daily lists does not exist";
		if (!dir.mkdir("dailylists"))
			qDebug () << " and could not create it!";

	}
	QList <challengeInfo> vals = challenges.values();
	foreach (challengeInfo ci, vals)
		delete ci.highScores;

	challenges.clear();
	for (int i = 2; i <= 15; i++)
	{
		challengeInfo tmpChallenge;
		tmpChallenge.highScores = new QHash <QString, highScoreData>;
		QString challengeName = QString("Today's %1s").arg(i);
		challenges.insert(challengeName, tmpChallenge);

		QFile tempInFile(QString("../listmaker/lists/%1s").arg(i));
		QFile tempOutFile("dailylists/" + challengeName);
		QTextStream inStream(&tempInFile);
		QTextStream outStream(&tempOutFile);
		tempInFile.open(QIODevice::ReadOnly | QIODevice::Text);
		tempOutFile.open(QIODevice::WriteOnly | QIODevice::Text);
		QStringList fileContents;
		while (!inStream.atEnd())
			fileContents << inStream.readLine().trimmed();
		tempInFile.close();
		int index;
		int numWords = 0;
		do
		{
			index = qrand() % fileContents.size();
			outStream << fileContents.at(index) << "\n";
			fileContents.removeAt(index);
			numWords++;
		} while (numWords < maxRacks);
		tempOutFile.close();
		qDebug() << QString("generated today's %1s").arg(i);

	}
	
}

void UnscrambleGame::generateTempFile()
{
	if (cycleState != 3)
	{
		qDebug() << "before generating temp file";
		QFile tempInFile(wordListFileName);
		QFile tempOutFile(QString("temp%1").arg(table->tableNumber));
		QTextStream inStream(&tempInFile);
		QTextStream outStream(&tempOutFile);
		tempInFile.open(QIODevice::ReadOnly | QIODevice::Text);
		tempOutFile.open(QIODevice::WriteOnly | QIODevice::Text);
		QStringList fileContents;
		while (!inStream.atEnd())
			fileContents << inStream.readLine().trimmed();
		numTotalRacks = fileContents.size();
		numMissedRacks = 0;
		tempInFile.close();
		int index;
		while (fileContents.size() > 0)
		{
			index = qrand() % fileContents.size();
			outStream << fileContents.at(index) << "\n";
			fileContents.removeAt(index);
		}
		tempOutFile.close();
		tempFileExists = true;
		qDebug() << "after generating temp file";
	}
	else
	{
		// daily challenges
		// don't need to 'generate temp file'. just copy the daily challenge from the lists
		QFile toRemove(QString("temp%1").arg(table->tableNumber));
		toRemove.remove();

		QFile tempInFile("dailylists/" + wordListFileName);
		tempInFile.copy(QString("temp%1").arg(table->tableNumber));
		qDebug() << "copied" << tempInFile.fileName() << " to temp file";
		tempFileExists = true;
		tempInFile.close();
		numTotalRacks = maxRacks;
	}
}
void UnscrambleGame::prepareTableAlphagrams()
{
	// load alphagrams into the gamesolutions hash and the alphagrams list
	if (cycleState == 0) tempFileExists = false;
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
	if (cycleState == 1 && alphagramReader.atEnd())
	{
		inFile.close();
		inFile.remove();
		outFile.close();
		outFile.copy(QString("temp%1").arg(table->tableNumber));
		outFile.remove();
		inFile.setFileName(QString("temp%1").arg(table->tableNumber));
		outFile.setFileName(QString("missed%1").arg(table->tableNumber));
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
				if (cycleState != 3)
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
	out << (quint8) '+';
	out << table->tableNumber;
	out << (quint8) 'W';
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
	out << (quint8) '+';
	out << (quint16) table->tableNumber;
	out << (quint8)'T';
	out << timerValue;
	fixHeaderLength();
	table->sendGenericPacket();
}

void UnscrambleGame::sendNumQuestionsPacket()
{
	writeHeaderData();
	out << (quint8) '+';
	out << (quint16) table->tableNumber;
	out << (quint8) 'N';
	out << numRacksSeen;
	out << numTotalRacks;
	fixHeaderLength();
	table->sendGenericPacket();

}

void UnscrambleGame::sendGiveUpPacket(QString username)
{
	writeHeaderData();
	out << (quint8) '+';
	out << (quint16) table->tableNumber;
	out << (quint8)'U';
	out << username;
	fixHeaderLength();
	table->sendGenericPacket();
}

void UnscrambleGame::sendGuessRightPacket(QString username, QString answer, quint8 index)
{
	writeHeaderData();
	out << (quint8) '+';
	out << (quint16) table->tableNumber;
	out << (quint8)'A';
	out << username << answer << index;
	fixHeaderLength();
	table->sendGenericPacket();
}
