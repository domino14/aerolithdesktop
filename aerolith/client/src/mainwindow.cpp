#include "mainwindow.h"

const quint16 MAGIC_NUMBER = 25345;
const QString WindowTitle = "Aerolith 0.1.1";
MainWindow::MainWindow() : PLAYERLIST_ROLE(Qt::UserRole), 
NUM_SOLUTIONS_ROLE(Qt::UserRole + 1),
SOLUTIONS_ROLE(Qt::UserRole + 2),
ALPHAGRAM_ROLE(Qt::UserRole + 3),
out(&block, QIODevice::WriteOnly)
{

	setWindowTitle(WindowTitle);
	//QTabWidget *mainTabWidget = new QTabWidget;
	//setCentralWidget(mainTabWidget);


	// create login widget
	QWidget *loginWidget = new QWidget;
	username = new QLineEdit;

	toggleConnection = new QPushButton("Connect");

	connectStatusLabel = new QLabel("Please enter your desired username");  
	QLabel *userLabel = new QLabel("Username: ");
	QLabel *serverLabel = new QLabel("Address: ");
	serverAddress = new QLineEdit("cesar.boldlygoingnowhere.org");
	serverAddress->setFixedWidth(150);
	username->setFixedWidth(150);
	username->setMaxLength(16);
	toggleConnection->setFixedWidth(150);
	connectStatusLabel->setFixedWidth(600);

	QGridLayout *loginWidgetLayout = new QGridLayout;
	loginWidgetLayout->addWidget(userLabel, 0, 0);
	loginWidgetLayout->addWidget(username, 0, 1);
	loginWidgetLayout->addWidget(serverLabel, 2, 0);
	loginWidgetLayout->addWidget(serverAddress, 2, 1);
	loginWidgetLayout->addWidget(toggleConnection, 3, 1);
	loginWidgetLayout->addWidget(connectStatusLabel, 5, 1);

	QHBoxLayout *overallLoginWidgetLayout = new QHBoxLayout;
	overallLoginWidgetLayout->addLayout(loginWidgetLayout);
	overallLoginWidgetLayout->addStretch(1);

	loginWidget->setLayout(overallLoginWidgetLayout);

	//  mainTabWidget->addTab(loginWidget, "Login window");

	// create game board widget
	gameBoardGroupBox = new QGroupBox("Game board");


	QWidget *centralWidget = new QWidget;	 // the 'overall'  widget

	wordsWidget = new QTableWidget(9, 5);

	wordsWidget->horizontalHeader()->hide();

	wordsWidget->setSelectionMode(QAbstractItemView::NoSelection);
	wordsWidget->verticalHeader()->hide();
	wordsWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	for (int i = 0; i < 5; i++)
		wordsWidget->setColumnWidth(i, 150);
	for (int i = 0; i < 9; i++)
		wordsWidget->setRowHeight(i, 20);

	QTableWidgetItem *tableItem[9][5];
#ifdef Q_OS_MAC
	QFont wordFont("Arial Black", 16, QFont::Normal);
#else
	QFont wordFont("Arial Black", 12, QFont::Normal);
#endif

	colorBrushes[0].setColor(Qt::black);
	colorBrushes[1].setColor(Qt::darkGreen);
	colorBrushes[2].setColor(Qt::blue);
	colorBrushes[3].setColor(Qt::darkCyan);
	colorBrushes[4].setColor(Qt::cyan);
	colorBrushes[5].setColor(Qt::darkMagenta);
	colorBrushes[6].setColor(Qt::darkRed);
	colorBrushes[7].setColor(Qt::red);
	colorBrushes[8].setColor(Qt::magenta);
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 5; j++)
		{
			tableItem[i][j] = new QTableWidgetItem("");
			tableItem[i][j]->setTextAlignment(Qt::AlignCenter);
			tableItem[i][j]->setFont(wordFont);
			// tableItem[i][j]->setForeground(colorBrushes[i]);
			wordsWidget->setItem(i, j, tableItem[i][j]);
		}
		wordsWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		wordsWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		wordsWidget->setFixedSize(752,182);	// this feels extremely cheap and i hate it but it seems to work 
		wordsWidget->setFocusPolicy(Qt::NoFocus);
		connect(wordsWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(wordsWidgetItemClicked(QTableWidgetItem*)));
		// solution box
	       	QLabel *solutionLabel = new QLabel("Guess:");
		solutionLE = new QLineEdit;
		solutionLE->setFixedWidth(100);
		solutionLE->setMaxLength(15);
		QPushButton *solutions = new QPushButton("Solutions");
		QPushButton *alpha = new QPushButton("Alpha");
		QPushButton *shuffle = new QPushButton("Shuffle");
		QPushButton *giveup = new QPushButton("Give up");
		QPushButton *start = new QPushButton("Start");
		exitTable = new QPushButton("Exit Table #");

		solutions->setFocusPolicy(Qt::NoFocus);
		alpha->setFocusPolicy(Qt::NoFocus);
		shuffle->setFocusPolicy(Qt::NoFocus);
		giveup->setFocusPolicy(Qt::NoFocus);
		start->setFocusPolicy(Qt::NoFocus);
		exitTable->setFocusPolicy(Qt::NoFocus);

		QHBoxLayout *topSolutionLayout = new QHBoxLayout;
		
		topSolutionLayout->addStretch(1);
		topSolutionLayout->addWidget(giveup);
		topSolutionLayout->addSpacing(50);
		topSolutionLayout->addWidget(exitTable);
		QHBoxLayout *bottomSolutionLayout = new QHBoxLayout;
		/*		timerDial = new QDial();
		timerDial->setMaximum(300);
		timerDial->setMinimum(0);
		timerDial->setValue(0);
		timerDial->setNotchesVisible(true);
		timerDial->setEnabled(false);*/
		timerDial = new QLabel;
		timerDial->setFixedWidth(30);
		timerDial->setAlignment(Qt::AlignCenter);
		bottomSolutionLayout->addWidget(solutionLabel);
		bottomSolutionLayout->addWidget(solutionLE);
		bottomSolutionLayout->addStretch(1);
		bottomSolutionLayout->addWidget(timerDial);
		bottomSolutionLayout->addWidget(start);
		bottomSolutionLayout->addWidget(solutions);
		bottomSolutionLayout->addWidget(alpha);
		bottomSolutionLayout->addWidget(shuffle);
		//solutionLayout->addSpacing(50);
		//solutionLayout->addWidget(giveup);
		//solutionLayout->addWidget(exitTable);
		
		connect(solutionLE, SIGNAL(returnPressed()), this, SLOT(submitSolutionLEContents()));
		connect(alpha, SIGNAL(clicked()), this, SLOT(alphagrammizeWords()));
		connect(shuffle, SIGNAL(clicked()), this, SLOT(shuffleWords()));
		connect(giveup, SIGNAL(clicked()), this, SLOT(giveUpOnThisGame()));
		connect(start, SIGNAL(clicked()), this, SLOT(submitReady()));
		// Players box
		
		QGridLayout *playerListsLayout = new QGridLayout;
		for (int i = 0; i < 6; i++)
		{
			playerNames[i] = new QLabel("");
			playerNames[i]->setAlignment(Qt::AlignHCenter);
			playerNames[i]->setFixedWidth(120);
			playerLists[i] = new QListWidget();
			playerLists[i]->setFixedWidth(120);
			playerLists[i]->setMinimumHeight(100);
			playerLists[i]->setFrameShape(QFrame::Box);

			playerStatus[i] = new QLabel("");
			playerStatus[i]->setFixedWidth(120);
			playerStatus[i]->setAlignment(Qt::AlignHCenter);
			if (i != 0)
			  {
			    playerLists[i]->hide();
			    playerNames[i]->hide();
			    playerStatus[i]->hide();
			  }
			playerListsLayout->addWidget(playerNames[i], 0, i*2);
			playerListsLayout->setColumnMinimumWidth((i*2)+1, 10);
			playerListsLayout->addWidget(playerLists[i], 1, i*2);
			playerListsLayout->addWidget(playerStatus[i], 2, i*2);
			playerLists[i]->setFocusPolicy(Qt::NoFocus);
		}
#ifdef Q_OS_MAC
		playerListsLayout->setRowMinimumHeight(1, 100);
#else
		playerListsLayout->setRowMinimumHeight(1, 150);
#endif
		QVBoxLayout *gameBoardLayout = new QVBoxLayout;
		gameBoardLayout->addLayout(topSolutionLayout);
		gameBoardLayout->addWidget(wordsWidget);
		gameBoardLayout->addSpacing(10);
		gameBoardLayout->addLayout(bottomSolutionLayout);
		gameBoardLayout->addSpacing(10);
		gameBoardLayout->addLayout(playerListsLayout);
		gameBoardGroupBox->setLayout(gameBoardLayout);

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
		roomTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
		roomTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
		QVBoxLayout *roomSelectorLayout = new QVBoxLayout;
		roomSelectorLayout->addWidget(roomTable);
		/*  QPushButton *joinRoom1 = new QPushButton("Join");
		QPushButton *joinRoom2 = new QPushButton("Join");
		roomTable->setCellWidget(0, 4, joinRoom1);
		roomTable->setCellWidget(1, 4, joinRoom2);*/

		QPushButton *newRoom = new QPushButton("Create new table");
		roomSelectorLayout->addWidget(newRoom);
		newRoom->setFixedWidth(150);
		roomSelectorGroupBox->setLayout(roomSelectorLayout);

		gameStackedWidget = new QStackedWidget;
		gameStackedWidget->addWidget(roomSelectorGroupBox);
		gameStackedWidget->addWidget(gameBoardGroupBox);
		gameStackedWidget->addWidget(loginWidget);
		//gameStackedWidget->setFixedWidth(850);


		connect(newRoom, SIGNAL(clicked()), this, SLOT(createNewRoom()));

		QGroupBox *chatGroupBox = new QGroupBox("Chat");


		// chat related stuff
		chatLE = new QLineEdit;
		chatLE->setMaxLength(300);
		chatText = new QTextEdit;
		chatText->setReadOnly(true);
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
		overallGameBoardLayout->addWidget(gameStackedWidget);
		overallGameBoardLayout->setAlignment(gameStackedWidget, Qt::AlignHCenter);
		overallGameBoardLayout->addStretch(1);
		overallGameBoardLayout->addWidget(chatGroupBox);
		centralWidget->setLayout(overallGameBoardLayout);
		//mainTabWidget->addTab(gameBoardWidget, "Game Board");
		//gameBoardGroupBox->setFixedWidth(800);
		chatText->setFocusPolicy(Qt::NoFocus);
		peopleConnected->setFocusPolicy(Qt::NoFocus);

		//mainTabWidget->setTabEnabled(1, false);
		chatText->document()->setMaximumBlockCount(5000);  // at most 5000 newlines.

		connect(chatLE, SIGNAL(returnPressed()), this, SLOT(submitChatLEContents()));

		// commsSocket

		commsSocket = new QTcpSocket;
		connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
		connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(displayError(QAbstractSocket::SocketError)));
		connect(commsSocket, SIGNAL(connected()), this, SLOT(writeUsernameToServer()));
		connect(toggleConnection, SIGNAL(clicked()), this, SLOT(toggleConnectToServer()));
		connect(commsSocket, SIGNAL(disconnected()), this, SLOT(serverDisconnection()));
		connect(exitTable, SIGNAL(clicked()), this, SLOT(leaveThisTable()));

		createTableDialog = new QDialog(this);
		uiTable.setupUi(createTableDialog);
		solutionsDialog = new QDialog(this);
		uiSolutions.setupUi(solutionsDialog);
		uiSolutions.solutionsTableWidget->verticalHeader()->hide();
		
		solutionsDialog->setAttribute(Qt::WA_QuitOnClose, false);

		gameStarted = false;
		connect(solutions, SIGNAL(clicked()), solutionsDialog, SLOT(show())); 
		blockSize = 0; 

		currentTablenum = 0;
		gameStackedWidget->setCurrentIndex(2);

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
		setTabOrder(solutionLE, chatLE);
		setTabOrder(chatLE, solutionLE);
		

		QMenu* helpMenu = menuBar()->addMenu("Help");
		QAction* helpAction = helpMenu->addAction("Aerolith Help");


		connect(helpAction, SIGNAL(triggered()), this, SLOT(aerolithHelpDialog()));
		

		//connect(qApp, SIGNAL(lastWindowClosed()), this, SLOT(
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
	else if (chatLE->text().indexOf("/shout ") == 0)
	{
		writeHeaderData();
		out << (quint8)'c';
		out << chatLE->text().mid(7);
		fixHeaderLength();
		commsSocket->write(block);
		chatLE->clear();
	}
	else

	{
		if (currentTablenum == 0)
		{
			writeHeaderData();
			out << (quint8)'c';
			out << chatLE->text();
			fixHeaderLength();
			commsSocket->write(block);
			chatLE->clear();
		}
		else
		{
			writeHeaderData();
			out << (quint8)'=';
			out << (quint16)currentTablenum;
			out << (quint8)'c';
			out << chatLE->text();
			fixHeaderLength();
			commsSocket->write(block);
			chatLE->clear();

		}
	}
}


void MainWindow::submitSolutionLEContents()
{
	// chatText->append(QString("From solution: ") + solutionLE->text());
	// solutionLE->clear();

	writeHeaderData();
	out << (quint8) '=';
	out << (quint16) currentTablenum;
	out << (quint8) 's'; // from solution box
	out << solutionLE->text();
	fixHeaderLength();
	commsSocket->write(block);
	solutionLE->clear();
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
		case 'E':	// logged in (entered)
			{
				QString username;
				in >> username;
				QListWidgetItem *it = new QListWidgetItem(username, peopleConnected);
				if (username == currentUsername) 
				{
					connectStatusLabel->setText("You have connected!");
					gameStackedWidget->setCurrentIndex(0);
					setWindowTitle(QString(WindowTitle + " - logged in as ") + username);
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
				QMessageBox::information(this, "Aerolith client", errorString);
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
					gameStackedWidget->setCurrentIndex(1);
					int row = findRoomTableRow(tablenum);
					QString wList = roomTable->item(row, 1)->text();
					gameBoardGroupBox->setTitle("Game board - Word List: " + wList);
					timerDial->setText("0");
					for (int i = 0; i < 6; i++)
					{
						playerNames[i]->setText("");
						playerLists[i]->clear();
						playerStatus[i]->setText("");
						playerLists[i]->hide();
						playerNames[i]->hide();
						playerStatus[i]->hide();
					}
					for (int i = 0; i < 9; i++)
						for (int j = 0; j < 5; j++)
							wordsWidget->item(i, j)->setText("");

					exitTable->setText(QString("Exit table %1").arg(tablenum));

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
				if (playerName == currentUsername)
				{
					currentTablenum = 0;
					gameStackedWidget->setCurrentIndex(0);
				}

				// chatText->append(QString("%1 has left %2").arg(playerName).arg(tablenum));
				handleLeaveTable(tablenum, playerName);
				if (currentTablenum == tablenum)
				{//i love shoe
					modifyPlayerLists(tablenum, playerName, -1);
				}

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
					QMessageBox::critical(this, "Aerolith client", "Critical error 10003");
					exit(0);
				}
				quint8 addByte;
				in >> addByte;

				handleTableCommand(tablenum, addByte);

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
	switch (socketError) {
  case QAbstractSocket::RemoteHostClosedError:
	  break;
  case QAbstractSocket::HostNotFoundError:
	  QMessageBox::information(this, tr("Aerolith Client"),
		  tr("The host was not found. Please check the "
		  "host name and port settings."));
	  break;
  case QAbstractSocket::ConnectionRefusedError:
	  QMessageBox::information(this, tr("Aerolith Client"),
		  tr("The connection was refused by the peer. "
		  "Make sure the Aerolith server is running, "
		  "and check that the host name and port "
		  "settings are correct."));
	  break;
  default:
	  QMessageBox::information(this, tr("Aerolith Client"),
		  tr("The following error occurred: %1.")
		  .arg(commsSocket->errorString()));
	}
	toggleConnection->setText("Connect");
	connectStatusLabel->setText("Disconnected.");
}

void MainWindow::toggleConnectToServer()
{
	if (commsSocket->state() != QAbstractSocket::ConnectedState)
	{

		commsSocket->abort();
		commsSocket->connectToHost(serverAddress->text(), 1988);
		connectStatusLabel->setText("Connecting to server...");
		toggleConnection->setText("Disconnect");


		roomTable->clearContents();
		roomTable->setRowCount(0);
	}
	else
	{	
		connectStatusLabel->setText("Disconnected from server");
		commsSocket->disconnectFromHost();
		toggleConnection->setText("Connect");
		gameStackedWidget->setCurrentIndex(2);
	}

}

void MainWindow::serverDisconnection()
{

	connectStatusLabel->setText("You are disconnected.");
	peopleConnected->clear();
	QMessageBox::information(this, "Aerolith Client", "You have been disconnected.");
	toggleConnection->setText("Connect");
	gameStackedWidget->setCurrentIndex(2);
	roomTable->clearContents();
	roomTable->setRowCount(0);
	setWindowTitle("Aerolith - disconnected");
	uiTable.elistsCbo->clear();
	uiTable.ulistsCbo->clear();
}

void MainWindow::writeUsernameToServer()
{
	// only after connecting!
	blockSize = 0;
	in.setDevice(commsSocket);
	in.setVersion(QDataStream::Qt_4_2);
	currentUsername = username->text();

	writeHeaderData();
	out << (quint8)'e';
	out << currentUsername;
	fixHeaderLength();
	commsSocket->write(block);

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
	case 'T':
		// a timer byte
		{
			quint16 timerval;	
			in >> timerval;
			timerDial->setText(QString("%1").arg(timerval));


		}

		break;
	case 'B':
		// a request for beginning the game from a player
		// refresh ready light for each player.
		{
			QString username;
			in >> username;
			int indexOfPlayer;
			for (int k = 0; k < 6; k++)
			{
				if (playerNames[k]->text() == username)
				{
					indexOfPlayer = k;
					break;
				}
			}
			playerStatus[indexOfPlayer]->setText("Ready.");

		}
		break;
	case 'S':
		// the game has started
		{
			for (int i = 0; i <6 ; i++)
			  {
				playerLists[i]->clear();
				playerStatus[i]->setText("");
			  }
			chatText->append("<font color=red>The game has started!</font>");
			gameStarted = true;
			rightAnswers.clear();
			//			uiSolutions.solutionsTableWidget->clearContents();
			//uiSolutions.solutionsTableWidget->setRowCount(0);
		}

		break;
	case 'E':
		// the game has ended
		{
			chatText->append("<font color=red>This round is over.</font>");
			gameStarted = false;

			for (int i = 0; i < 9; i++)
				for (int j = 0; j < 5; j++)
				{
					wordsWidget->item(i, j)->setText("");
					QStringList theseSols = wordsWidget->item(i,j)->data(SOLUTIONS_ROLE).toStringList();
					QString alphagram = wordsWidget->item(i, j)->data(ALPHAGRAM_ROLE).toString();
					QTableWidgetItem *tableAlphagramItem = new QTableWidgetItem(alphagram);
					tableAlphagramItem->setTextAlignment(Qt::AlignCenter);
					int alphagramRow = uiSolutions.solutionsTableWidget->rowCount();
					

					for (int i = 0; i < theseSols.size(); i++)
					{
						uiSolutions.solutionsTableWidget->insertRow(uiSolutions.solutionsTableWidget->rowCount());
						QTableWidgetItem *wordItem = new QTableWidgetItem(theseSols.at(i));
						if (wordDb.isOpen())
						{
							QString backHooks, frontHooks, definition;
							QSqlQuery query;
							query.exec("select back_hooks from words where word='"+theseSols.at(i)+"'");
							while (query.next())
								backHooks = query.value(0).toString();
							QTableWidgetItem *backHookItem = new QTableWidgetItem(backHooks);
							uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 3, backHookItem);

							query.exec("select front_hooks from words where word='"+theseSols.at(i)+"'");
							while (query.next())
								frontHooks = query.value(0).toString();
							QTableWidgetItem *frontHookItem = new QTableWidgetItem(frontHooks);
							uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 1, frontHookItem);

							query.exec("select definition from words where word='"+theseSols.at(i)+"'");
							while (query.next())
								definition = query.value(0).toString();
							QTableWidgetItem *definitionItem = new QTableWidgetItem(definition);
							uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 4, definitionItem);
						}

						if (!rightAnswers.contains(theseSols.at(i)))
						{
							wordItem->setForeground(colorBrushes[7]);
							QFont wordItemFont = wordItem->font();
							wordItemFont.setBold(true);
							wordItem->setFont(wordItemFont);
						}
						wordItem->setTextAlignment(Qt::AlignCenter);
						uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount() - 1, 2, wordItem);
		
					}
					uiSolutions.solutionsTableWidget->setItem(alphagramRow, 0, tableAlphagramItem);
				}
				uiSolutions.solutionsTableWidget->resizeColumnsToContents();

		}
		break;


	case 'C':
		// chat
		{
			QString username, chat;
			in >> username;
			in >> chat;
			chatText->append("[" + username + " to table] " + chat);
		}
		break;

	case 'W':
		// 45 alphagrams!!!

		for (int i = 0; i < 9; i++)
			for (int j = 0; j < 5; j++)
			{
				QString alphagram;
				in >> alphagram;
				quint8 numsolutions;
				in >> numsolutions;
				QStringList solutions;
				in >> solutions;
				wordsWidget->item(i, j)->setText(alphagram);
				if (alphagram != "") 
				{
					wordsWidget->item(i, j)->setForeground(colorBrushes[(numsolutions > 9 ? 8 : (numsolutions - 1))]);
					wordsWidget->item(i, j)->setData(NUM_SOLUTIONS_ROLE, QVariant(numsolutions)); // numsolutions yet to be solved
					wordsWidget->item(i, j)->setData(SOLUTIONS_ROLE, QVariant(solutions));
					wordsWidget->item(i, j)->setData(ALPHAGRAM_ROLE, QVariant(alphagram));
				}
			}
			uiSolutions.solutionsTableWidget->clearContents();
			uiSolutions.solutionsTableWidget->setRowCount(0);
			break;

	case 'U':
		// someone cried uncle
		{
			QString username;
			in >> username;
			chatText->append("<font color=red>" + username + " has requested to end this game. </font>");
		}
		break;
	case 'A':
		{
			QString username, guess;
			quint8 row, column;
			in >> username >> guess >> row >> column;
			quint8 numsolutions = wordsWidget->item(row, column)->data(NUM_SOLUTIONS_ROLE).toInt();
			numsolutions--;
			wordsWidget->item(row, column)->setData(NUM_SOLUTIONS_ROLE, QVariant(numsolutions));
			if (numsolutions > 0)
				wordsWidget->item(row, column)->setForeground(colorBrushes[(numsolutions > 9 ? 8 : (numsolutions - 1))]);
			else
				wordsWidget->item(row, column)->setText("");

			int indexOfPlayer;
			for (int k = 0; k < 6; k++)
			{
				if (playerNames[k]->text() == username)
				{
					indexOfPlayer = k;
					break;
				}
			}
			playerLists[indexOfPlayer]->insertItem(0, guess);
			playerLists[indexOfPlayer]->item(0)->setTextAlignment(Qt::AlignCenter);
			rightAnswers.insert(guess);
			playerStatus[indexOfPlayer]->setText(QString("%1 words").arg(playerLists[indexOfPlayer]->count()));
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
			seats.clear(); // clear the seats hash if we leave! 	  

			return; // the widget will be hid anyway, so we don't need to hide the individual lists
			//however, we hide when adding when we join down below
		}
		else 
		{

			// add all players including self
			for (int i = 0; i < plist.size(); i++)
			{
				playerNames[i]->setText(plist[i]);
				playerNames[i]->show();
				playerLists[i]->show();
				playerStatus[i]->show();
				seats.insert(plist[i], i);
			}


			return;
		}
	}

	// if we are here then a player has joined/left a table that we were already in

	// modification = -1 remove
	// or 1 add

	//playerNames - labels, playerLists qlistwidgets



	if (modification == 1)
	{
		// player has been added
		// find a spot
		bool spotfound = false;
		int spot;

		for (int i = 0; i < 6; i++)
			if (playerNames[i]->text() == "")
			{
				spotfound = true;
				spot = i;
				break;
			}
			//spot i
			if (spotfound == false)
			{
				QMessageBox::critical(this, "?", "Please notify developer about this error. (Error code 10001)");
				return;
			}

			playerNames[spot]->setText(player);
			playerNames[spot]->show();
			playerLists[spot]->show();
			playerStatus[spot]->show();
			seats.insert(player, spot);
	}

	else if (modification == -1)
	{
		int seat;
		if (seats.contains(player))
		{
			seat = seats.value(player);
		}
		else
		{
			QMessageBox::critical(this, "?", "Please notify developer about this error. (Error code 10002)");
			return;
		}
		seats.remove(player);
		playerNames[seat]->setText("");
		playerStatus[seat]->setText("");
		playerLists[seat]->clear();
		playerNames[seat]->hide();
		playerLists[seat]->hide();
		playerStatus[seat]->hide();
		if (gameStarted == false)
		{
			for (int i = 0; i < 6; i++)
				playerStatus[i]->setText("");
		}
	}

	/*
	for (int i = 0; i < plist.length(); i++)
	{
	}

	for (int i = 0; i < 6; i++)
	{
	if (playerNames[i]->text() != "")
	{
	}

	}
	*/  


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

void MainWindow::alphagrammizeWords()
{
	// wordsWidget
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 5; j++)
			wordsWidget->item(i,j)->setText(alphagrammizeString(wordsWidget->item(i,j)->text()));
}

void MainWindow::shuffleWords()
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 5; j++)
			wordsWidget->item(i,j)->setText(shuffleString(wordsWidget->item(i,j)->text()));
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
void MainWindow::wordsWidgetItemClicked(QTableWidgetItem* item)
{
	int i = item->row();
	int j = item->column();
	wordsWidget->item(i, j)->setText(shuffleString(wordsWidget->item(i, j)->text()));

}

QString MainWindow::alphagrammizeString(QString inputString)
{
	int letters[26];
	for (int i = 0; i < 26; i++)
		letters[i] = 0;

	for (int i = 0; i < inputString.length(); i++)
	{
		letters[inputString[i].toUpper().toAscii() - 'A']++;
	}
	QString retString = "";
	for (int i = 0; i < 26; i++)
		for (int j = 0; j < letters[i]; j++)
			retString += (unsigned char)(i + 'A');

	return retString;
}

QString MainWindow::shuffleString(QString inputString)
{
	for (int i = 0; i < inputString.length(); i++)
	{
		int j = qrand() % inputString.length();
		QChar tmp;
		tmp = inputString[i];
		inputString[i] = inputString[j];
		inputString[j] = tmp;

	}
	return inputString;
}

void MainWindow::aerolithHelpDialog()
{
  QString infoText;

  infoText = "- To send text to a single player, double click his or her name and type in your message, or type /msg username message in the chat box.<BR>";
  infoText += "- To send text to everyone, type in /shout message in the chat box. <BR>";
  infoText += "- Cycle mode allows you to go through all the words in a list, and at the end, keep going through the missed words.<BR>";
  QMessageBox::information(this, "Aerolith how-to", infoText);
}
