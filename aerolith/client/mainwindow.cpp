#include "mainwindow.h"

MainWindow::MainWindow() : PLAYERLIST_ROLE(Qt::UserRole + 1), out(&block, QIODevice::WriteOnly)
{
  
  setWindowTitle("Aerolith");
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
  
  QTableWidget *wordsWidget = new QTableWidget(9, 5);

  wordsWidget->horizontalHeader()->hide();

  wordsWidget->setSelectionMode(QAbstractItemView::NoSelection);
  wordsWidget->verticalHeader()->hide();
  wordsWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  for (int i = 0; i < 5; i++)
	wordsWidget->setColumnWidth(i, 150);
  for (int i = 0; i < 9; i++)
	wordsWidget->setRowHeight(i, 20);

  QTableWidgetItem *tableItem[9][5];
  QFont wordFont("Helvetica", 12, QFont::Bold);
  
	
  colorBrushes[0].setColor(Qt::black);
  colorBrushes[1].setColor(Qt::gray);
  colorBrushes[2].setColor(Qt::darkBlue);
  colorBrushes[3].setColor(Qt::blue);
  colorBrushes[4].setColor(Qt::darkCyan);
  colorBrushes[5].setColor(Qt::darkMagenta);
  colorBrushes[6].setColor(Qt::darkRed);
  colorBrushes[7].setColor(Qt::red);
  colorBrushes[8].setColor(Qt::magenta);
  for (int i = 0; i < 9; i++)
    for (int j = 0; j < 5; j++)
      {
	tableItem[i][j] = new QTableWidgetItem("");
	tableItem[i][j]->setTextAlignment(Qt::AlignHCenter);
	//tableItem[i][j]->setFont(wordFont);
	// tableItem[i][j]->setForeground(colorBrushes[i]);
	wordsWidget->setItem(i, j, tableItem[i][j]);
      }
  wordsWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  wordsWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  wordsWidget->setFixedSize(752,182);	// this feels extremely cheap and i hate it but it seems to work 
  
  
  // solution box
  QLabel *solutionLabel = new QLabel("Guess:");
  solutionLE = new QLineEdit;
  solutionLE->setFixedWidth(100);
  solutionLE->setMaxLength(15);
  QPushButton *alpha = new QPushButton("Alpha");
  QPushButton *shuffle = new QPushButton("Shuffle");
  QPushButton *giveup = new QPushButton("Give up");
	exitTable = new QPushButton("Exit Table #");
  QHBoxLayout *solutionLayout = new QHBoxLayout;

  timerDial = new QDial();
  timerDial->setMaximum(200);
  timerDial->setMinimum(0);
  timerDial->setValue(0);
  timerDial->setNotchesVisible(true);
  timerDial->setEnabled(false);


  solutionLayout->addWidget(solutionLabel);
  solutionLayout->addWidget(solutionLE);
  solutionLayout->addStretch(1);
  solutionLayout->addWidget(timerDial);
  solutionLayout->addWidget(alpha);
  solutionLayout->addWidget(shuffle);
  solutionLayout->addWidget(giveup);
  solutionLayout->addSpacing(50);
  solutionLayout->addWidget(exitTable);
  connect(solutionLE, SIGNAL(returnPressed()), this, SLOT(submitSolutionLEContents()));

  // Players box
  
  QGridLayout *playerListsLayout = new QGridLayout;
  for (int i = 0; i < 6; i++)
    {
      playerNames[i] = new QLabel("");
      playerNames[i]->setAlignment(Qt::AlignHCenter);
      playerNames[i]->setFixedWidth(120);
      playerLists[i] = new QListWidget();
      playerLists[i]->setFixedWidth(120);
      playerLists[i]->setMinimumHeight(200);
      playerLists[i]->setFrameShape(QFrame::Box);
      playerLists[i]->hide();
      playerNames[i]->hide();
     
      playerListsLayout->addWidget(playerNames[i], 0, i*2);
      playerListsLayout->setColumnMinimumWidth((i*2)+1, 10);
      playerListsLayout->addWidget(playerLists[i], 1, i*2);
	  
    }
  playerListsLayout->setRowMinimumHeight(1, 200);
  
  QVBoxLayout *gameBoardLayout = new QVBoxLayout;
  gameBoardLayout->addWidget(wordsWidget);
  gameBoardLayout->addSpacing(10);
  gameBoardLayout->addLayout(solutionLayout);
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
  chatLE->setMaxLength(500);
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
	createTableDialogWindow = new QDialog;
     ui.setupUi(createTableDialogWindow);



  blockSize = 0; 

  currentTablenum = 0;
  gameStackedWidget->setCurrentIndex(2);

  setCentralWidget(centralWidget);  
  out.setVersion(QDataStream::Qt_4_2);  
}

void MainWindow::writeHeaderData()
{
  out.device()->seek(0);
  block.clear();
  out << (quint16)25344;	// magic number
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
  else
    {

      writeHeaderData();
      out << (quint8)'c';
      out << chatLE->text();
      fixHeaderLength();
      commsSocket->write(block);
      chatLE->clear();
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
	  if (header != (quint16)25344) // magic number
	    {
	      QMessageBox::information(this, "aerolith", QString("wrong magic number: %1").arg(header));
	      commsSocket->disconnectFromHost();	
	      return;
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
      qDebug() << "Packet type " << (char)packetType;
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
		setWindowTitle(QString("Aerolith - logged in as ") + username);
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
		timerDial->setValue(0);
		for (int i = 0; i < 6; i++)
		  {
		    playerNames[i]->setText("");
		    playerLists[i]->clear();
		    playerLists[i]->hide();
		    playerNames[i]->hide();
		  }
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
			ui.elistsCbo->addItem(listTitle);
			
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
  ui.elistsCbo->clear();
  ui.ulistsCbo->clear();
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

}

void MainWindow::createNewRoom()
{
	
	// reset dialog to defaults first.
  ui.cycleRbo->setChecked(true);
  ui.alphagramRbo->setChecked(true);
  ui.existingRbo->setChecked(true);
  ui.playersSpinBox->setValue(1);
  int retval = createTableDialogWindow->exec();
  if (retval == QDialog::Rejected) return;
  
  // else its not rejected

  writeHeaderData();
  out << (quint8)'t';
  if (ui.existingRbo->isChecked() == true)
      out << ui.elistsCbo->currentText();
  else
    out << ui.ulistsCbo->currentText();

  out << (quint8)ui.playersSpinBox->value();
	if (ui.cycleRbo->isChecked() == true) out << (quint8)1;
	else out << (quint8)0;
	if (ui.alphagramRbo->isChecked() == true) out << (quint8)1;
	else out << (quint8)0;

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
			timerDial->setValue(timerval);
			

		}
	
		break;
	case 'S':
		// the game has started
		{
			for (int i = 0; i <6 ; i++)
				playerLists[i]->clear();
			chatText->append("<font color=red>The game has started!</font>");
		}

		break;
	case 'E':
		// the game has ended
		{
			chatText->append("<font color=red>Time is up!</font>");

		}

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
	playerLists[seat]->clear();
	playerNames[seat]->hide();
	playerLists[seat]->hide();

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
