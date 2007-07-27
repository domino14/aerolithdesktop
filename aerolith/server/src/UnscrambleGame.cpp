#include "UnscrambleGame.h"

extern QByteArray block;
extern QDataStream out;

const quint8 COUNTDOWN_TIMER_VAL = 3;

void UnscrambleGame::initialize(quint8 cycleState, quint8 tableTimer, QString wordListFileName)
{

  this->wordListFileName = wordListFileName;

  tempFileExists = false;
  this->cycleState = cycleState;

  gameTimer = new QTimer();
  countdownTimer = new QTimer();

  gameTimer->setSingleShot(true);
  
  connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));
  connect(countdownTimer, SIGNAL(timeout()), this, SLOT(updateCountdownTimer()));

  gameStarted = false;
  countingDown = false;
  tableTimerVal = (quint16)tableTimer * (quint16)60;
  currentTimerVal = tableTimerVal;
  countdownTimerVal = COUNTDOWN_TIMER_VAL;

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
  if (gameStarted == false && countingDown == false)
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
	      quint8 rowIndex = 0, columnIndex = 0;
	      quint8 indexOfQuestion = alphagramIndices.value(alphagram);
	      unscrambleGameQuestions[indexOfQuestion].numNotYetSolved--;
	      rowIndex = unscrambleGameQuestions[indexOfQuestion].i;
	      columnIndex = unscrambleGameQuestions[indexOfQuestion].j;
	      
	      sendGuessRightPacket(socket->connData.userName, guess, rowIndex, columnIndex);
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
      if (giveUp == true) endGame();
      
    }
  
}

void UnscrambleGame::startGame()
{
  countingDown = false;                                                           
  countdownTimer->stop();                                                         
  
  
  // code for starting the game                                                          
  // steps:                                                                              
  // 1. list should already be loaded when table was created                             
  // 2. send to everyone @ the table:                                                    
  //    - 45 alphagrams                                                                  
  gameStarted = true; 
  prepareTableAlphagrams();
  foreach (ClientSocket* socket, table->playerList)
    sendUserCurrentAlphagrams(socket);

  sendGameStartPacket();
  sendTimerValuePacket(tableTimerVal);
  //  table->currentTimerVal = table->tableTimerVal;                                         
  gameTimer->start(1000*tableTimerVal);
 


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
    for (int i = 0; i < 45; i++)                                                       
      {                                                                   
	if (unscrambleGameQuestions.at(i).numNotYetSolved > 0)                    
	  {                                                                            
	    // this one has not been solved!                                           
	    missedFileWriter << unscrambleGameQuestions.at(i).alphagram;
	    for (int j = 0; j < unscrambleGameQuestions.at(i).solutions.size(); j++)
	      missedFileWriter << " " << unscrambleGameQuestions.at(i).solutions.at(j);
	    missedFileWriter << "\n";                                             
	    qDebug() << "wrote" << unscrambleGameQuestions.at(i).alphagram <<     
	      unscrambleGameQuestions.at(i).solutions << " to missed file.";      
            
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
  endGame();
}

void UnscrambleGame::generateTempFile()
{

  qDebug() << "before generating temp file";                                         
  QFile tempInFile(wordListFileName);
  QFile tempOutFile(QString("tmp%1").arg(table->tableNumber));                           
  QTextStream inStream(&tempInFile);                                                     
  QTextStream outStream(&tempOutFile);                                                   
  tempInFile.open(QIODevice::ReadOnly | QIODevice::Text);                                
  tempOutFile.open(QIODevice::WriteOnly | QIODevice::Text);                              

  QStringList fileContents;                                                          
  while (!inStream.atEnd())                                                          
    fileContents << inStream.readLine().trimmed();                                   
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

void UnscrambleGame::prepareTableAlphagrams()
{
  // load alphagrams into the gamesolutions hash and the alphagrams list                 

                                                                                         
  if (cycleState == 0) tempFileExists = false;                             
                                                                                         
  if (tempFileExists == false)                                                    
    {                                                                                    
      inFile.close();                                                             
      outFile.close();                                                            

      generateTempFile();

      inFile.setFileName(QString("tmp%1").arg(table->tableNumber));               
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
      outFile.copy(QString("tmp%1").arg(table->tableNumber));                     
      outFile.remove();                                                           
                                                                                         
      inFile.setFileName(QString("tmp%1").arg(table->tableNumber));               
      outFile.setFileName(QString("missed%1").arg(table->tableNumber));           
      alphagramReader.setDevice(&inFile);
      missedFileWriter.setDevice(&outFile);
      inFile.open(QIODevice::ReadOnly | QIODevice::Text);                         
      outFile.open(QIODevice::WriteOnly | QIODevice::Text);                       
      missedFileWriter.seek(0);                                                   
      alphagramReader.seek(0);                                                    
      // CLOSE table->inFile                                                             
      // COPY missed%1 to tmp%1                                                          
      // repeat code above to load the file into memory, shuffle, etc                    
                                                                                         
    }                                                                                    

  QStringList lineList;                                                                  
  QString line;                                                                          
                                                                                         
  for (quint8 i = 0; i < 9; i++)                                                         
    for (quint8 j = 0; j < 5; j++)                                                       
      {                                                                                  
        unscrambleGameData thisGameData;                                                 
        thisGameData.i = i;                                                              
        thisGameData.j = j;                                                              
        if (alphagramReader.atEnd())                                              
          {                                                                              
            thisGameData.alphagram = ""; 
	    thisGameData.numNotYetSolved = 0;                                            
          }                                                                              
        else                                                                             
          {                                                                              
            line = alphagramReader.readLine();                                    
            lineList = line.split(" ");                                                  
            thisGameData.alphagram = lineList.at(0);                                     
            thisGameData.numNotYetSolved = (quint8)(lineList.size()-1);                  
            for (int k = 1; k < lineList.size(); k++)                                    
              {                                                                          
                gameSolutions.insert(lineList.at(k), lineList.at(0));             
                thisGameData.solutions << lineList.at(k);                                
              }                                                                          
          }                                                                              
                                                                                         
        unscrambleGameQuestions.append(thisGameData);                             
        alphagramIndices.insert(thisGameData.alphagram, i*5 + j);                 
      }                 
}

void UnscrambleGame::sendUserCurrentAlphagrams(ClientSocket* socket)
{
  writeHeaderData();                                                                     
  out << (quint8) '+';                                                                   
  out << table->tableNumber;                                                             
  out << (quint8) 'W';                                                                   
                                                                                         
  for (int i = 0; i < 45; i++)                                                           
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

void UnscrambleGame::sendGuessRightPacket(QString username, QString answer, quint8 row, quint8 column)
{
  writeHeaderData();
  out << (quint8) '+';
  out << (quint16) table->tableNumber;
  out << (quint8)'A';
  out << username << answer << row << column;
  fixHeaderLength();
  table->sendGenericPacket();
}
