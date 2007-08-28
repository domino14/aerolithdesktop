#include "mainwindow.h"

const quint16 MAGIC_NUMBER = 25347;
const QString WindowTitle = "Aerolith 0.3.1";
const QString thisVersion = "0.3.1";

bool highScoresLessThan(const tempHighScoresStruct& a, const tempHighScoresStruct& b)
{
  if (a.numCorrect == b.numCorrect) return (a.timeRemaining > b.timeRemaining);
  else return (a.numCorrect > b.numCorrect);
}

MainWindow::MainWindow() : PLAYERLIST_ROLE(Qt::UserRole), 
out(&block, QIODevice::WriteOnly)
{

  setWindowTitle(WindowTitle);
  //QTabWidget *mainTabWidget = new QTabWidget;
  //setCentralWidget(mainTabWidget);
  
  
  
  // create game board widget
  //gameBoardGroupBox = new QGroupBox("Game board");
  
  
  gameBoardWidget = new UnscrambleGameTable(0, Qt::Window);
  gameBoardWidget->setWindowTitle("Table");
  
  connect(gameBoardWidget, SIGNAL(giveUp()), this, SLOT(giveUpOnThisGame()));
  connect(gameBoardWidget, SIGNAL(sendStartRequest()), this, SLOT(submitReady()));
  connect(gameBoardWidget, SIGNAL(avatarChange(quint8)), this, SLOT(changeMyAvatar(quint8)));
  connect(gameBoardWidget, SIGNAL(guessSubmitted(QString)), this, SLOT(submitGuess(QString)));
  connect(gameBoardWidget, SIGNAL(chatTable(QString)), this, SLOT(chatTable(QString)));

  
  centralWidget = new QWidget;	 // the 'overall'  widget
  
  // the room selector will be anotehr group box
  
  QGroupBox *roomSelectorGroupBox = new QGroupBox("Table selector");
  
  roomTable = new QTableWidget(0, 6);
  QStringList headerLabels;
  headerLabels << "#" << "Word List" << "Players" << "#" << "Max #" << "Join?";
  roomTable->setHorizontalHeaderLabels(headerLabels);
  roomTable->setGridStyle(Qt::NoPen);
  roomTable->setSelectionMode(QAbstractItemView::NoSelection);
  roomTable->verticalHeader()->hide();
  roomTable->setColumnWidth(0, 30);
  roomTable->setColumnWidth(1, 200);
  roomTable->setColumnWidth(2, 300);
  roomTable->setColumnWidth(3, 50);
  roomTable->setColumnWidth(4, 50);
  roomTable->setColumnWidth(5, 75);
  roomTable->setFixedWidth(750);
  roomTable->setMinimumHeight(400);
  roomTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
  roomTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  QVBoxLayout *roomSelectorLayout = new QVBoxLayout;
  roomSelectorLayout->addWidget(roomTable, 0, Qt::AlignHCenter);
  QHBoxLayout *newRoomLayout = new QHBoxLayout;
  QPushButton *newRoom = new QPushButton("Create new table");
  newRoom->setFixedWidth(150);
  QPushButton *dailyChallenges = new QPushButton("Daily challenges");
  dailyChallenges->setFixedWidth(150);
  
  QMenu *challengesMenu = new QMenu;
  for (int i = 4; i <= 15; i++)
    challengesMenu->addAction(QString("Daily %1s").arg(i));
  challengesMenu->addAction("Get today's scores");
  
  connect(challengesMenu, SIGNAL(triggered(QAction*)), this, SLOT(dailyChallengeSelected(QAction*)));
  dailyChallenges->setMenu(challengesMenu);
  
  
  newRoomLayout->addWidget(newRoom);
  newRoomLayout->addWidget(dailyChallenges);
  roomSelectorLayout->addLayout(newRoomLayout);
  
  
  roomSelectorGroupBox->setLayout(roomSelectorLayout);
  
  //		gameStackedWidget = new QStackedWidget;
  //	gameStackedWidget->addWidget(roomSelectorGroupBox);
  //	gameStackedWidget->addWidget(gameBoardGroupBox);
  
  //gameStackedWidget->setFixedWidth(850);
  
  
  connect(newRoom, SIGNAL(clicked()), this, SLOT(createNewRoom()));
  
  QGroupBox *chatGroupBox = new QGroupBox("Chat");
  
  
  // chat related stuff
  chatLE = new QLineEdit;
  chatLE->setMaxLength(300);
  chatText = new QTextEdit;
  chatText->setReadOnly(true);
  chatText->setTextInteractionFlags(Qt::TextSelectableByMouse);
  QVBoxLayout *chatLayout = new QVBoxLayout();
  
  chatLayout->addWidget(chatLE);
  chatLayout->addSpacing(5);
  
  QHBoxLayout *chatBoxLayout = new QHBoxLayout;
  chatBoxLayout->addWidget(chatText);
  chatText->setFrameShape(QFrame::Box);
  peopleConnected = new QListWidget;
  peopleConnected->setFrameShape(QFrame::Box);
  peopleConnected->setFixedWidth(150);
  chatBoxLayout->addWidget(peopleConnected);
  chatLayout->addLayout(chatBoxLayout);
  chatGroupBox->setLayout(chatLayout);
  connect(peopleConnected, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(sendPM(QListWidgetItem* )));
  
  QVBoxLayout *overallGameBoardLayout = new QVBoxLayout;
  //		overallGameBoardLayout->addWidget(gameStackedWidget);
  //	overallGameBoardLayout->setAlignment(gameStackedWidget, Qt::AlignHCenter);
  overallGameBoardLayout->addWidget(roomSelectorGroupBox);
  overallGameBoardLayout->addStretch(1);
  overallGameBoardLayout->addWidget(chatGroupBox);
  centralWidget->setLayout(overallGameBoardLayout);
 
  
  //gameBoardGroupBox->setFixedWidth(800);
  chatText->setFocusPolicy(Qt::ClickFocus);
  peopleConnected->setFocusPolicy(Qt::NoFocus);
  
  //mainTabWidget->setTabEnabled(1, false);
  chatText->document()->setMaximumBlockCount(5000);  // at most 5000 newlines.
  
  connect(chatLE, SIGNAL(returnPressed()), this, SLOT(submitChatLEContents()));
  
  // commsSocket
  
  commsSocket = new QTcpSocket;
  connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
  connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)), 
	  this, SLOT(displayError(QAbstractSocket::SocketError)));
  connect(commsSocket, SIGNAL(connected()), this, SLOT(connectedToServer()));
  
  connect(commsSocket, SIGNAL(disconnected()), this, SLOT(serverDisconnection()));
  connect(gameBoardWidget, SIGNAL(exitThisTable()), this, SLOT(leaveThisTable()));
  
  createTableDialog = new QDialog(this);
  uiTable.setupUi(createTableDialog);
  
  solutionsDialog = new QDialog(gameBoardWidget);
  uiSolutions.setupUi(solutionsDialog);
  uiSolutions.solutionsTableWidget->verticalHeader()->hide();
  
  scoresDialog = new QDialog(this);
  uiScores.setupUi(scoresDialog);
  uiScores.scoresTableWidget->verticalHeader()->hide();
  connect(uiScores.getScoresPushbutton, SIGNAL(clicked()), this, SLOT(getScores()));
  
  loginDialog = new QDialog(this);
  uiLogin.setupUi(loginDialog);
  //  loginDialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
  connect(uiLogin.loginPushButton, SIGNAL(clicked()), this, SLOT(toggleConnectToServer()));
  
  connect(uiLogin.registerPushButton, SIGNAL(clicked()), this, SLOT(showRegisterPage()));
  connect(uiLogin.cancelRegPushButton, SIGNAL(clicked()), this, SLOT(showLoginPage()));
  
  connect(uiLogin.submitRegPushButton, SIGNAL(clicked()), this, SLOT(registerName()));
  solutionsDialog->setAttribute(Qt::WA_QuitOnClose, false);
  scoresDialog->setAttribute(Qt::WA_QuitOnClose, false);
  loginDialog->setAttribute(Qt::WA_QuitOnClose, false);   
  gameBoardWidget->setAttribute(Qt::WA_QuitOnClose, false);
  
  gameStarted = false;
  connect(gameBoardWidget->solutions, SIGNAL(clicked()), solutionsDialog, SLOT(show())); 
  blockSize = 0; 
  
  currentTablenum = 0;
  uiLogin.stackedWidget->setCurrentIndex(0);
  
  
  
  setCentralWidget(centralWidget);  
  out.setVersion(QDataStream::Qt_4_2);  
  
  QTime midnight(0, 0, 0);
  qsrand(midnight.msecsTo(QTime::currentTime()));
  
  if (QFile::exists(QDir::homePath()+"/.zyzzyva/lexicons/OWL2+LWL.db"))
    {
      chatText->append("<font color=red>A suitable Zyzzyva installation was found!</font>");	
      wordDb = QSqlDatabase::addDatabase("QSQLITE");
      wordDb.setDatabaseName(QDir::homePath() + "/.zyzzyva/lexicons/OWL2+LWL.db");
      wordDb.open();
    }
  else
    {
      chatText->append("<font color=red>A suitable Zyzzyva installation was not found. You will not be able to see definitions and hooks for the words at the end of each round. Zyzzyva is a free word study tool found at http://www.zyzzyva.net</font>");
    }
  
  
  
  QMenu* helpMenu = menuBar()->addMenu("Help");
  QAction* helpAction = helpMenu->addAction("Aerolith Help");
  QMenu* loginMenu = menuBar()->addMenu("Connect");
  QAction* connectAction = loginMenu->addAction("Connect to Aerolith");
  
  connect(helpAction, SIGNAL(triggered()), this, SLOT(aerolithHelpDialog()));
  connect(connectAction, SIGNAL(triggered()), loginDialog, SLOT(raise()));

  connect(connectAction, SIGNAL(triggered()), loginDialog, SLOT(show()));
  missedColorBrush.setColor(Qt::red);
  
  gameTimer = new QTimer();
  connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));
  //connect(qApp, SIGNAL(lastWindowClosed()), this, SLOT(
  
  show();
  loginDialog->show();
  //  loginDialog->activateWindow();
  loginDialog->raise();
  uiLogin.usernameLE->setFocus(Qt::OtherFocusReason);
  setWindowIcon(QIcon(":images/aerolith.png"));
}

void MainWindow::writeHeaderData()
{
  out.device()->seek(0);
  block.clear();
  out << (quint16)MAGIC_NUMBER;	// magic number
  out << (quint16)0; // length 
}

void MainWindow::fixHeaderLength()
{
  out.device()->seek(sizeof(quint16));
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
}

void MainWindow::submitChatLEContents()
{
  //  chatText->append(QString("From chat: ") + chatLE->text());
  
  if (chatLE->text().indexOf("/msg ") == 0)
    {
      QString restOfText = chatLE->text().mid(5);
      QString username = restOfText.mid(0, restOfText.indexOf(" "));
      QString message = restOfText.mid(username.length()+1);
      if (message.simplified() == "" || message.simplified() == " ")
	{
	  chatLE->clear();
	  return;
	}
      chatText->append("[You write to " + username + "] " + message);
      writeHeaderData();
      out << (quint8)'p';
      out << username;
      out << message;
      fixHeaderLength();
      commsSocket->write(block);
      chatLE->clear();
    }
  /*  else if (chatLE->text().indexOf("/shout ") == 0)
    {
      writeHeaderData();
      out << (quint8)'c';
      out << chatLE->text().mid(7);
      fixHeaderLength();
      commsSocket->write(block);
      chatLE->clear();
      }*/
 /* else if (chatLE->text().indexOf("/me ") == 0)
    {
      writeHeaderData();
      out << (quint8)'a';
      out << chatLE->text().mid(4);
      fixHeaderLength();
      commsSocket->write(block);
      chatLE->clear();
    }*/
  else
    {
      writeHeaderData();
      out << (quint8)'c';
      out << chatLE->text();
      fixHeaderLength();
      commsSocket->write(block);
      chatLE->clear();
    }
  /*  else
	{
		  
	  }*/
}

void MainWindow::chatTable(QString textToSend)
{
	if (textToSend.indexOf("/msg ") == 0)
    {
      QString restOfText = textToSend.mid(5);
      QString username = restOfText.mid(0, restOfText.indexOf(" "));
      QString message = restOfText.mid(username.length()+1);
      if (message.simplified() == "" || message.simplified() == " ")
	{
	 
	  return;
	}
      gameBoardWidget->tableChat->append("[You write to " + username + "] " + message);
      writeHeaderData();
      out << (quint8)'p';
      out << username;
      out << message;
      fixHeaderLength();
      commsSocket->write(block);
    }
	else if (textToSend.indexOf("/me ") == 0)
    {
      writeHeaderData();
      out << (quint8)'=';
	  out << (quint16)currentTablenum;
	  out << (quint8)'a';
      out << textToSend.mid(4);
      fixHeaderLength();
      commsSocket->write(block);
    }
	else
	{


  writeHeaderData();
  out << (quint8)'=';
  out << (quint16)currentTablenum;
  out << (quint8)'c';
  out << textToSend;
  fixHeaderLength();
  commsSocket->write(block);
	}

}

void MainWindow::submitGuess(QString guess)
{
	// chatText->append(QString("From solution: ") + solutionLE->text());
	// solutionLE->clear();
  if (guess.length() > 15) return;
  
  writeHeaderData();
  out << (quint8) '=';
  out << (quint16) currentTablenum;
  out << (quint8) 's'; // from solution box
  out << guess;
  fixHeaderLength();
  commsSocket->write(block);
  //  gameBoardWidget->solutionLE->clear();
}

void MainWindow::readFromServer()
{
	// same structure as server's read

	//QDataStream needs EVERY byte to be available for reading!
	while (commsSocket->bytesAvailable() > 0)
	{
		if (blockSize == 0)
		{
			if (commsSocket->bytesAvailable() < 4)
				return;

			quint16 header;
			quint16 packetlength;

			// there are 4 bytes available. read them in!

			in >> header;
			in >> packetlength;
			if (header != (quint16)MAGIC_NUMBER) // magic number
			{
			  chatText->append("Wrong magic number (you have the wrong version of the client)");
			  //				commsSocket->disconnectFromHost();	
			  //				return;
			}
			blockSize = packetlength;

		}

		if (commsSocket->bytesAvailable() < blockSize)
			return;

		// ok, we can now read the WHOLE packet
		// ideally we read the 'case' byte right now, and then call a process function with
		// 'in' (the QDataStream) as a parameter!
		// the process function will set blocksize to 0 at the end
		quint8 packetType;
		in >> packetType; // this is the case byte!
		qDebug() << "Packet type " << (char)packetType << "block length" << blockSize;
		switch(packetType)
		{
		case '?':

		  // keep alive
		  writeHeaderData();
		  out << (quint8)'?';
		  fixHeaderLength();
		  commsSocket->write(block);		  
		  break;

		case 'E':	// logged in (entered)
			{
				QString username;
				in >> username;
				QListWidgetItem *it = new QListWidgetItem(username, peopleConnected);
				if (username == currentUsername) 
				{
				  uiLogin.connectStatusLabel->setText("You have connected!");
				  loginDialog->hide();
				  setWindowTitle(QString(WindowTitle + " - logged in as ") + username);
				  sendClientVersion();   // not yet. add this for the actual version 0.1.2
				  gameBoardWidget->playerInfoWidget->setMyUsername(username);
				  currentTablenum = 0;
				}
			}
			break;
		case 'X':	// logged out
			{
				QString username;
				in >> username;
				for (int i = 0; i < peopleConnected->count(); i++)
					if (peopleConnected->item(i)->text() == username)
					{
						QListWidgetItem *it = peopleConnected->takeItem(i);
						delete it;
					}

			}
			break;

		case '!':	// error
			{
				QString errorString;
				in >> errorString;
				QMessageBox::information(loginDialog, "Aerolith client", errorString);
				//	chatText->append("<font color=red>" + errorString + "</font>");
			}
			break;
		case 'C':	// chat
			{
				QString username;
				in >> username;
				QString text;
				in >> text;
				chatText->append(QString("[")+username+"] " + text);
			}
			break;
		case 'P':	// PM
			{
				QString username, message;
				in >> username >> message;
				chatText->append(QString("[")+username+ " writes to you] " + message);
				gameBoardWidget->tableChat->append(QString("[")+username + " writes to you] " + message);
			}
			break;
		case 'T':	// New table
			{
				// there is a new table

				// static information
				quint16 tablenum;
				QString wordListDescriptor;
				quint8 maxPlayers;
				//		QStringList 

				in >> tablenum >> wordListDescriptor >> maxPlayers;
				// create table
				handleCreateTable(tablenum, wordListDescriptor, maxPlayers);
			}
			break;
		case 'J':	// player joined table
			{
				quint16 tablenum;
				QString playerName;

				in >> tablenum >> playerName; // this will also black out the corresponding button for can join
				handleAddToTable(tablenum, playerName);
				if (playerName == currentUsername)
				{
					currentTablenum = tablenum;
					//gameStackedWidget->setCurrentIndex(1);
					int row = findRoomTableRow(tablenum);
					QString wList = roomTable->item(row, 1)->text();
					
					gameBoardWidget->resetTable(tablenum, wList, playerName);
					gameBoardWidget->show();
	
				}
				if (currentTablenum == tablenum)
					modifyPlayerLists(tablenum, playerName, 1);
				//  chatText->append(QString("%1 has entered %2").arg(playerName).arg(tablenum));


			}
			break;
		case 'L':
			{
				// player left table
				quint16 tablenum;
				QString playerName;
				in >> tablenum >> playerName;

				if (currentTablenum == tablenum)
				{//i love shoe
					modifyPlayerLists(tablenum, playerName, -1);
				}
				
				if (playerName == currentUsername)
				{
					currentTablenum = 0;
					//gameStackedWidget->setCurrentIndex(0);
					gameBoardWidget->hide();
				}

				// chatText->append(QString("%1 has left %2").arg(playerName).arg(tablenum));
				handleLeaveTable(tablenum, playerName);

			}

			break;
		case 'K':
			{
				// table has ceased to exist
				quint16 tablenum;
				in >> tablenum;
				//	    chatText->append(QString("%1 has ceasd to exist").arg(tablenum));
				handleDeleteTable(tablenum);
			}	
			break;
		case 'W':
			{
				// word lists
				QString listTitle;
				in >> listTitle;
				uiTable.elistsCbo->addItem(listTitle);

			}
			break;
		case '+':
			// table command
			// an additional byte is needed
			{
				quint16 tablenum;
				in >> tablenum; 
				if (tablenum != currentTablenum)
				{
				  qDebug() << "HORRIBLE ERROR" << tablenum << currentTablenum;
					QMessageBox::critical(this, "Aerolith client", "Critical error 10003");
					exit(0);
				}
				quint8 addByte;
				in >> addByte;

				handleTableCommand(tablenum, addByte);

			}
			break;
		case 'S':
			// server message
			{
				QString serverMessage;
				in >> serverMessage;

				chatText->append("<font color=green>" + serverMessage + "</font>");


			}
			break;
		case 'H':
		  // high scores!
		  {
		    displayHighScores();

		  }


		  break;
		case 'I':
			// avatar id
			{
				QString username;
				quint8 avatarID;
				in >> username >> avatarID;
				// username changed his avatar to avatarID. if we want to display this avatar, display it
				// i.e. if we are in a table. in the future consider changing this to just a table packet but do the check now
					// just in case.
				if (currentTablenum != 0)
				{
					// we are in a table
					gameBoardWidget->playerInfoWidget->setAvatar(username, avatarID);

				}
				// then here we can do something like chatwidget->setavatar( etc). but this requires the server
				// to send avatars to more than just the table. so if we want to do this, we need to change the server behavior!
				// this way we can just send everyone's avatar on login. consider changing this!
			}
			break;
		default:
			QMessageBox::critical(this, "Aerolith client", "Don't understand this packet!");
			commsSocket->disconnectFromHost();
		}

		blockSize = 0;
	}
}


void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
  switch (socketError) 
    {
    case QAbstractSocket::RemoteHostClosedError:
      break;
    case QAbstractSocket::HostNotFoundError:
      chatText->append("<font color=red>The host was not found. Please check the "
		       "host name and port settings.</font>");
      break;
    case QAbstractSocket::ConnectionRefusedError:
      chatText->append("<font color=red>The connection was refused by the peer. "
		       "Make sure the Aerolith server is running, "
		       "and check that the host name and port "
		       "settings are correct.</font>");
      break;
    default:
      chatText->append(QString("<font color=red>The following error occurred: %1.</font>")
		       .arg(commsSocket->errorString()));
    }
  
  uiLogin.loginPushButton->setText("Connect");
  uiLogin.connectStatusLabel->setText("Disconnected.");
}

void MainWindow::toggleConnectToServer()
{
	if (commsSocket->state() != QAbstractSocket::ConnectedState)
	{
	  connectionMode = MODE_LOGIN;
		commsSocket->abort();
		
		commsSocket->connectToHost(uiLogin.serverLE->text(), uiLogin.portLE->text().toInt());
		uiLogin.connectStatusLabel->setText("Connecting to server...");
		uiLogin.loginPushButton->setText("Disconnect");
		

		roomTable->clearContents();
		roomTable->setRowCount(0);
	}
	else
	{	
	  uiLogin.connectStatusLabel->setText("Disconnected from server");
		commsSocket->disconnectFromHost();
		uiLogin.loginPushButton->setText("Connect");
		//gameStackedWidget->setCurrentIndex(2);
		//centralWidget->hide();
		loginDialog->show();
		loginDialog->raise();
	}

}

void MainWindow::serverDisconnection()
{

	uiLogin.connectStatusLabel->setText("You are disconnected.");
	peopleConnected->clear();
//	QMessageBox::information(this, "Aerolith Client", "You have been disconnected.");
	uiLogin.loginPushButton->setText("Connect");
	//centralWidget->hide();
	loginDialog->show();
	loginDialog->raise();
	roomTable->clearContents();
	roomTable->setRowCount(0);
	setWindowTitle("Aerolith - disconnected");
	uiTable.elistsCbo->clear();
	uiTable.ulistsCbo->clear();
}

void MainWindow::connectedToServer()
{
	// only after connecting!
	blockSize = 0;
	in.setDevice(commsSocket);
	in.setVersion(QDataStream::Qt_4_2);


	// here we see if we are registering a name, or if we are connecting with an existing
	// name/password
	
	if (connectionMode == MODE_LOGIN)
	  {

	    currentUsername = uiLogin.usernameLE->text();
		
	    
	    writeHeaderData();
	    out << (quint8)'e';
	    out << currentUsername;
		out << uiLogin.passwordLE->text();
	    fixHeaderLength();
	    commsSocket->write(block);
	  }
	else if (connectionMode == MODE_REGISTER)
	  {
	    writeHeaderData();
	    out << (quint8)'r';
	    out << uiLogin.desiredUsernameLE->text();
	    out << uiLogin.desiredFirstPasswordLE->text();
	    fixHeaderLength();
	    commsSocket->write(block);
 	  }
}

void MainWindow::sendPM(QListWidgetItem* item)
{
	chatLE->setText(QString("/msg ") + item->text() + " ");
	chatLE->setFocus(Qt::OtherFocusReason);
}

void MainWindow::createNewRoom()
{

	// reset dialog to defaults first.
	uiTable.cycleRbo->setChecked(true);
	uiTable.existingRbo->setChecked(true);
	uiTable.userRbo->setEnabled(false);
	uiTable.playersSpinBox->setValue(1);
	uiTable.timerSpinBox->setValue(4);
	int retval = createTableDialog->exec();
	if (retval == QDialog::Rejected) return;

	// else its not rejected

	writeHeaderData();
	out << (quint8)'t';
	if (uiTable.existingRbo->isChecked() == true)
		out << uiTable.elistsCbo->currentText();
	else
		out << uiTable.ulistsCbo->currentText();

	out << (quint8)uiTable.playersSpinBox->value();

	if (uiTable.cycleRbo->isChecked() == true) out << (quint8)1;
	else if (uiTable.endlessRbo->isChecked() == true) out << (quint8)2;
	else out << (quint8)0;

	out << (quint8)uiTable.timerSpinBox->value();
	fixHeaderLength();
	commsSocket->write(block);

	/*  writeHeaderData();
	out << (quint8)'t';
	out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
	out << (quint8)6;
	fixHeaderLength();
	commsSocket->write(block);*/
}

void MainWindow::joinTable()
{
	QPushButton* buttonThatSent = static_cast<QPushButton*> (sender());
	QVariant tn = buttonThatSent->property("tablenum");
	quint16 tablenum = (quint16)tn.toInt();
	writeHeaderData();
	out << (quint8)'j';
	out << (quint16) tablenum;
	fixHeaderLength();
	commsSocket->write(block);
}

void MainWindow::leaveThisTable()
{
	writeHeaderData();
	out << (quint8)'l';
	out << (quint16)currentTablenum;
	fixHeaderLength();
	commsSocket->write(block);
}

int MainWindow::findRoomTableRow(quint16 tablenum)
{
	// straight linear search.
	// i guess this is ok. i'm not gonna have that many clients :P
	// and even if i do, it makes more sense to split across servers
	for (int i = 0; i < roomTable->rowCount(); i++)
	{
		if (roomTable->item(i, 0)->text().toInt() == (int)tablenum)
			return i;
	}
	return roomTable->rowCount();
}

void MainWindow::handleTableCommand(quint16 tablenum, quint8 commandByte)
{
	switch (commandByte)
	{
	case 'M':
		// a message
		{
			QString message;
			in >> message;
			gameBoardWidget->tableChat->append("<font color=green>" + message + "</font>");


		}
		break;
	case 'T':
		// a timer byte
		{
			quint16 timerval;	
			in >> timerval;
			gameBoardWidget->timerDial->display(timerval);


		}

		break;
	case 'B':
		// a request for beginning the game from a player
		// refresh ready light for each player.
		{
			QString username;
			in >> username;
			gameBoardWidget->playerInfoWidget->setReadyIndicator(username);
		}
		break;
	case 'S':
		// the game has started
		{
		  gameBoardWidget->playerInfoWidget->setupForGameStart();
		  gameBoardWidget->tableChat->append("<font color=red>The game has started!</font>");
		  gameStarted = true;
		  rightAnswers.clear();
		  //gameTimer->start(1000);
		}

		break;
	case 'E':
		// the game has ended
		
		gameBoardWidget->tableChat->append("<font color=red>This round is over.</font>");
		gameStarted = false;
		populateSolutionsTable();
		///gameTimer->stop();
		break;
		

	case 'C':
		// chat
		{
			QString username, chat;
			in >> username;
			in >> chat;
			gameBoardWidget->tableChat->append("[" + username + "] " + chat);
		}
		break;

	case 'W':
		// 45 alphagrams!!!

		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 5; j++)
			{
				QString alphagram;
				in >> alphagram;
				quint8 numSolutionsNotYetSolved;
				in >> numSolutionsNotYetSolved;
				QStringList solutions;
				in >> solutions;
				gameBoardWidget->wordsWidget->setCellProperties(i, j, alphagram, solutions, numSolutionsNotYetSolved);
			}
			uiSolutions.solutionsTableWidget->clearContents();
			uiSolutions.solutionsTableWidget->setRowCount(0);
			uiSolutions.solsLabel->clear();
			gameBoardWidget->wordsWidget->prepareForStart();
			break;

	case 'N':
		// word list info

		{
			quint16 numRacksSeen;
			quint16 numTotalRacks;
			in >> numRacksSeen >> numTotalRacks;
			gameBoardWidget->wordListInfo->setText(QString("%1 / %2").arg(numRacksSeen).arg(numTotalRacks));
			break;
		}

	case 'U':
		// someone cried uncle
		{
			QString username;
			in >> username;
			gameBoardWidget->tableChat->append("<font color=red>" + username + " has requested to end this game. </font>");
		}
		break;
	case 'A':
		{
		  QString username, answer;
			quint8 row, column;
			in >> username >> answer >> row >> column;
			gameBoardWidget->wordsWidget->answeredCorrectly(row, column);
			gameBoardWidget->playerInfoWidget->answered(username, answer);
			rightAnswers.insert(answer);

		}
		break;
	}


}

void MainWindow::handleCreateTable(quint16 tablenum, QString wordListDescriptor, quint8 maxPlayers)
{
	QTableWidgetItem *tableNumItem = new QTableWidgetItem(QString("%1").arg(tablenum));
	QTableWidgetItem *descriptorItem = new QTableWidgetItem(wordListDescriptor);
	QTableWidgetItem *maxPlayersItem = new QTableWidgetItem(QString("%1").arg(maxPlayers));
	QTableWidgetItem *playersItem = new QTableWidgetItem("");
	QTableWidgetItem *numPlayersItem = new QTableWidgetItem("0");
	QPushButton* buttonItem = new QPushButton("Join");
	buttonItem->setProperty("tablenum", QVariant((quint16)tablenum));
	connect(buttonItem, SIGNAL(clicked()), this, SLOT(joinTable()));
	buttonItem->setEnabled(false);

	QStringList playerList;
	playersItem->setData(PLAYERLIST_ROLE, QVariant(playerList));

	roomTable->insertRow(roomTable->rowCount());
	roomTable->setItem(roomTable->rowCount()-1, 0, tableNumItem);
	roomTable->setItem(roomTable->rowCount()-1, 1, descriptorItem);
	roomTable->setItem(roomTable->rowCount()-1, 2, playersItem);
	roomTable->setItem(roomTable->rowCount()-1, 3, numPlayersItem);
	roomTable->setItem(roomTable->rowCount()-1, 4, maxPlayersItem);
	roomTable->setCellWidget(roomTable->rowCount()-1, 5, buttonItem);



	// tablenums.insert(tablenum, roomTable->rowCount()-1);
}

void MainWindow::modifyPlayerLists(quint16 tablenum, QString player, int modification)
{

  // if player = currentusername
  
  int row = findRoomTableRow(tablenum);
  QVariant plistVar = roomTable->item(row, 2)->data(PLAYERLIST_ROLE);
  QStringList plist = plistVar.toStringList();
  
  // plist contains all the players
  
  if (player == currentUsername)	// I joined. therefore, populate the list from the beginning
    {
      if (modification == -1) 
	{
	  gameBoardWidget->leaveTable();

	  return; // the widget will be hid anyway, so we don't need to hide the individual lists
	  //however, we hide when adding when we join down below
	}
      else 
	{
	  gameBoardWidget->addPlayers(plist);
	  // add all players including self
	  return;
	}
    }

  // if we are here then a player has joined/left a table that we were already in
  
  // modification = -1 remove
  // or 1 add
  
  if (modification == 1)
	  // player has been added
	  // find a spot
	  gameBoardWidget->addPlayer(player, gameStarted);
  
  else if (modification == -1)
    gameBoardWidget->removePlayer(player, gameStarted);


}



void MainWindow::handleDeleteTable(quint16 tablenum)
{
	int row = findRoomTableRow(tablenum);

	roomTable->removeRow(row);
}

void MainWindow::handleLeaveTable(quint16 tablenum, QString player)
{
	int row = findRoomTableRow(tablenum);

	QVariant plistVar = roomTable->item(row, 2)->data(PLAYERLIST_ROLE);
	QStringList plist = plistVar.toStringList();
	plist.removeAll(player);
	QString textToSet="";
	foreach(QString thisplayer, plist)
		textToSet += thisplayer + " ";

	roomTable->item(row, 2)->setText(textToSet);
	roomTable->item(row, 2)->setData(PLAYERLIST_ROLE, QVariant(plist));


	quint8 curNumPlayers = roomTable->item(row,3)->text().toShort();
	quint8 maxNumPlayers = roomTable->item(row,4)->text().toShort();
	curNumPlayers--;
	roomTable->item(row, 3)->setText(QString("%1").arg(curNumPlayers));
	if (curNumPlayers >= maxNumPlayers)
		roomTable->cellWidget(row, 5)->setEnabled(false);
	else
		roomTable->cellWidget(row, 5)->setEnabled(true);
}

void MainWindow::handleAddToTable(quint16 tablenum, QString player)
{
	// this will change button state as well
	int row = findRoomTableRow(tablenum);

	quint8 curNumPlayers = roomTable->item(row,3)->text().toShort();
	quint8 maxNumPlayers = roomTable->item(row,4)->text().toShort();
	curNumPlayers++;
	roomTable->item(row, 3)->setText(QString("%1").arg(curNumPlayers));

	QVariant plistVar = roomTable->item(row, 2)->data(PLAYERLIST_ROLE);
	QStringList plist = plistVar.toStringList();
	plist << player;
	QString textToSet="";
	foreach(QString thisplayer, plist)
		textToSet += thisplayer + " ";

	roomTable->item(row, 2)->setText(textToSet);
	roomTable->item(row, 2)->setData(PLAYERLIST_ROLE, QVariant(plist));
	if (curNumPlayers >= maxNumPlayers)
		roomTable->cellWidget(row, 5)->setEnabled(false);
	else
		roomTable->cellWidget(row, 5)->setEnabled(true);

}


void MainWindow::giveUpOnThisGame()
{
	writeHeaderData();
	out << (quint8)'=';
	out << (quint16)currentTablenum;
	out << (quint8)'u';
	fixHeaderLength();
	commsSocket->write(block);
}

void MainWindow::submitReady()
{
  writeHeaderData();
  out << (quint8)'=';
  out << (quint16)currentTablenum;
  out << (quint8)'b';
  fixHeaderLength();
  commsSocket->write(block);

}

void MainWindow::aerolithHelpDialog()
{
  QString infoText;

  infoText = "- To send text to a single player, double click his or her name and type in your message, or type /msg username message in the chat box.<BR>";
  //infoText += "- To send text to everyone, type in /shout message in the chat box. <BR>";
  infoText += "- Cycle mode allows you to go through all the words in a list, and at the end, keep going through the missed words.<BR>";
  QMessageBox::information(this, "Aerolith how-to", infoText);
}

void MainWindow::displayHighScores()
{
  quint8 wordLength;
  quint16 numSolutions;
  quint16 numEntries;
  in >> wordLength >> numSolutions >> numEntries;
  QString username;
  quint16 numCorrect;
  quint16 timeRemaining;
  uiScores.scoresTableWidget->clearContents();
  uiScores.scoresTableWidget->setRowCount(0);

  
  QList <tempHighScoresStruct> temp;
  for (int i = 0; i < numEntries; i++)
    {
      in >> username >> numCorrect >> timeRemaining;
      temp << tempHighScoresStruct(username, numCorrect, timeRemaining);
    }
  qSort(temp.begin(), temp.end(), highScoresLessThan);
  
  for (int i = 0; i < numEntries; i++)
    {

      QTableWidgetItem* rankItem = new QTableWidgetItem(QString("%1").arg(i+1));
      QTableWidgetItem* usernameItem = new QTableWidgetItem(temp.at(i).username);
      QTableWidgetItem* correctItem = new QTableWidgetItem(QString("%1%").arg(100.0*(double)temp.at(i).numCorrect/(double)numSolutions, 0, 'f', 1));
      QTableWidgetItem* timeItem = new QTableWidgetItem(QString("%1 s").arg(temp.at(i).timeRemaining));
      uiScores.scoresTableWidget->insertRow(uiScores.scoresTableWidget->rowCount());
      uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 0, rankItem);
      uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 1, usernameItem);
      uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 2, correctItem);
      uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 3, timeItem);
    }

  uiScores.scoresTableWidget->resizeColumnsToContents();
}

void MainWindow::populateSolutionsTable()
{
	int numTotalSols = 0, numWrong = 0;
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 5; j++)
		{
			gameBoardWidget->wordsWidget->item(i, j)->setText("");
			QStringList theseSols = gameBoardWidget->wordsWidget->getCellSolutions(i, j);
			QString alphagram = gameBoardWidget->wordsWidget->getCellAlphagram(i, j);

			if (alphagram != "") // if alphagram exists.
			{
				QTableWidgetItem *tableAlphagramItem = new QTableWidgetItem(alphagram);
				tableAlphagramItem->setTextAlignment(Qt::AlignCenter);
				int alphagramRow = uiSolutions.solutionsTableWidget->rowCount();


				for (int i = 0; i < theseSols.size(); i++)
				{
					numTotalSols++;
					uiSolutions.solutionsTableWidget->insertRow(uiSolutions.solutionsTableWidget->rowCount());
					QTableWidgetItem *wordItem = new QTableWidgetItem(theseSols.at(i));
					if (wordDb.isOpen())
					{
						QString backHooks, frontHooks, definition;
						QSqlQuery query;

						query.exec("select front_hooks, back_hooks, definition from words where word = '" + theseSols.at(i) + "'");
						qDebug() << "select front_hooks, back_hooks, definition from words where word = '" + theseSols.at(i) + "'";
						while (query.next())
						{
							frontHooks = query.value(0).toString();
							backHooks = query.value(1).toString();
							definition = query.value(2).toString();
						}
						QTableWidgetItem *backHookItem = new QTableWidgetItem(backHooks);
						uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 3, backHookItem);							
						QTableWidgetItem *frontHookItem = new QTableWidgetItem(frontHooks);
						uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 1, frontHookItem);
						QTableWidgetItem *definitionItem = new QTableWidgetItem(definition);
						uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 4, definitionItem);
					}


					if (!rightAnswers.contains(theseSols.at(i)))
					{
						numWrong++;
						wordItem->setForeground(missedColorBrush);
						QFont wordItemFont = wordItem->font();
						wordItemFont.setBold(true);
						wordItem->setFont(wordItemFont);
					}
					wordItem->setTextAlignment(Qt::AlignCenter);
					uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount() - 1, 2, wordItem);

				}
				uiSolutions.solutionsTableWidget->setItem(alphagramRow, 0, tableAlphagramItem);
			}
		}
	uiSolutions.solutionsTableWidget->resizeColumnsToContents();
	double percCorrect;
	if (numTotalSols == 0) percCorrect = 0.0;
	else
		percCorrect = (100.0 * (double)(numTotalSols - numWrong))/(double)(numTotalSols);
	uiSolutions.solsLabel->setText(QString("Number of total solutions: %1   Percentage correct: %2 (%3 of %4)").arg(numTotalSols).arg(percCorrect).arg(numTotalSols-numWrong).arg(numTotalSols));
}

void MainWindow::sendClientVersion()
{
	writeHeaderData();
	out << (quint8)'v';
	out << thisVersion;
	fixHeaderLength();
	commsSocket->write(block);
}

void MainWindow::changeMyAvatar(quint8 avatarID)
{
	writeHeaderData();
	out << (quint8) 'i' << avatarID;
	fixHeaderLength();
	commsSocket->write(block);
}

void MainWindow::updateGameTimer()
{
  if (gameBoardWidget->timerDial->value() == 0) return;
  
  gameBoardWidget->timerDial->display(gameBoardWidget->timerDial->value() - 1);

}

void MainWindow::dailyChallengeSelected(QAction* challengeAction)
{
  if (challengeAction->text() == "Get today's scores")
    {
      uiScores.scoresTableWidget->clearContents();
      uiScores.scoresTableWidget->setRowCount(0);
      scoresDialog->show();
    }
  else
    {
	writeHeaderData();
	out << (quint8)'t';
	out << challengeAction->text(); // create a table 
	out << (quint8)1; // 1 player
	out << (quint8)3; // 3 is for daily challenges (TODO: HARDCODE BAD)

	if (challengeAction->text() == "Daily 4s")
	  out << (quint8)3; // 3 mins
	else
	  out << (quint8)4;

	fixHeaderLength();
	commsSocket->write(block);
    }
}

void MainWindow::getScores()
{
  uiScores.scoresTableWidget->clearContents();
  uiScores.scoresTableWidget->setRowCount(0);
  writeHeaderData();
  out << (quint8)'h';
  out << (quint8)(uiScores.scoresComboBox->currentIndex() + 4);
  fixHeaderLength();
  commsSocket->write(block);


}

void MainWindow::registerName()
{

  //  int retval = registerDialog->exec();
  //if (retval == QDialog::Rejected) return;
  // validate password
  if (uiLogin.desiredFirstPasswordLE->text() != uiLogin.desiredSecondPasswordLE->text())
    {
      QMessageBox::warning(this, "Aerolith", "Your passwords do not match! Please try again");
      uiLogin.desiredFirstPasswordLE->clear();
      uiLogin.desiredSecondPasswordLE->clear();
      return;
      
    }



  // 

  connectionMode = MODE_REGISTER;
  commsSocket->abort();
  commsSocket->connectToHost(uiLogin.serverLE->text(), uiLogin.portLE->text().toInt());
  uiLogin.connectStatusLabel->setText("Connecting to server...");
  uiLogin.loginPushButton->setText("Disconnect");
}

void MainWindow::showRegisterPage()
{
  uiLogin.stackedWidget->setCurrentIndex(1);
}

void MainWindow::showLoginPage()
{
  uiLogin.stackedWidget->setCurrentIndex(0);
}
