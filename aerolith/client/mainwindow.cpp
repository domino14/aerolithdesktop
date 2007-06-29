#include "mainwindow.h"

MainWindow::MainWindow()
{
  
  setWindowTitle("Aerolith");
  QTabWidget *mainTabWidget = new QTabWidget;
  setCentralWidget(mainTabWidget);
  

  // create login widget
  QWidget *loginWidget = new QWidget;
  username = new QLineEdit;
 
  toggleConnection = new QPushButton("Connect");
  
  connectStatusLabel = new QLabel("Please enter your desired username");  
  QLabel *userLabel = new QLabel("Username: ");
  username->setFixedWidth(150);
  username->setMaxLength(16);
  toggleConnection->setFixedWidth(150);
  connectStatusLabel->setFixedWidth(600);
  
  QGridLayout *loginWidgetLayout = new QGridLayout;
  loginWidgetLayout->addWidget(userLabel, 0, 0);
  loginWidgetLayout->addWidget(username, 0, 1);
  loginWidgetLayout->addWidget(toggleConnection, 3, 1);
  loginWidgetLayout->addWidget(connectStatusLabel, 5, 1);

  QHBoxLayout *overallLoginWidgetLayout = new QHBoxLayout;
  overallLoginWidgetLayout->addLayout(loginWidgetLayout);
  overallLoginWidgetLayout->addStretch(1);
  
  loginWidget->setLayout(overallLoginWidgetLayout);
  
  mainTabWidget->addTab(loginWidget, "Login window");

  // create game board widget
  QGroupBox *gameBoardGroupBox = new QGroupBox("Game board");

  
  QWidget *gameBoardWidget = new QWidget;	 // the 'overall' game board widget
  
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
	tableItem[i][j] = new QTableWidgetItem("ADEEFRST");
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
  QPushButton *alpha = new QPushButton("Alpha");
  QPushButton *shuffle = new QPushButton("Shuffle");
  exitTable = new QPushButton("Exit Table #");
  QHBoxLayout *solutionLayout = new QHBoxLayout;
  solutionLayout->addWidget(solutionLabel);
  solutionLayout->addWidget(solutionLE);
  solutionLayout->addStretch(1);
  solutionLayout->addWidget(alpha);
  solutionLayout->addWidget(shuffle);
  solutionLayout->addWidget(exitTable);
  connect(solutionLE, SIGNAL(returnPressed()), this, SLOT(submitSolutionLEContents()));

  // Players box
  
  QListWidget *playerLists[6];
  QLabel *playerNames[6];
  
  QGridLayout *playerListsLayout = new QGridLayout;
  for (int i = 0; i < 6; i++)
    {
      playerNames[i] = new QLabel("ADEILNPS");	
      playerNames[i]->setAlignment(Qt::AlignHCenter);
      playerNames[i]->setFixedWidth(120);
      playerLists[i] = new QListWidget();
      playerLists[i]->setFixedWidth(120);
      playerLists[i]->setMinimumHeight(200);
      playerLists[i]->setFrameShape(QFrame::Box);
      
      //if (i > -1) { playerLists[i]->hide(); playerNames[i]->hide(); }
      playerListsLayout->addWidget(playerNames[i], 0, i*2);
      playerListsLayout->setColumnMinimumWidth((i*2)+1, 10);
      playerListsLayout->addWidget(playerLists[i], 1, i*2);
    }
  
  
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
  gameBoardWidget->setLayout(overallGameBoardLayout);
  mainTabWidget->addTab(gameBoardWidget, "Game Board");
  gameBoardGroupBox->setFixedWidth(800);


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
  
  blockSize = 0; 

  currentTablenum = 0;


  
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
      QByteArray block;
      QDataStream out(&block, QIODevice::WriteOnly);
      out.setVersion(QDataStream::Qt_4_2);
      out << (quint16)25344;	// magic number
      out << (quint16)0; // length 
      out << (quint8)'p';
      out << username;
      out << message;
      out.device()->seek(2); // position of length
      out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16)); 
      commsSocket->write(block);
      
      chatLE->clear();
    }
  else
    {

      QByteArray block;
      QDataStream out(&block, QIODevice::WriteOnly);
      out.setVersion(QDataStream::Qt_4_2);
      out << (quint16)25344;	// magic number
      out << (quint16)0; // length 
      out << (quint8)'c';
      out << chatLE->text();
      out.device()->seek(2); // position of length
      out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16)); 
      commsSocket->write(block);
      
      chatLE->clear();
    }
}


void MainWindow::submitSolutionLEContents()
{
  chatText->append(QString("From solution: ") + solutionLE->text());
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
	    if (username == currentUsername) connectStatusLabel->setText("You have connected!");
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
	    if (playerName == currentUsername)
	      {
		currentTablenum = tablenum;
		gameStackedWidget->setCurrentIndex(1);
		exitTable->setText(QString("Exit table %1").arg(tablenum));
		
	      }
	  //  chatText->append(QString("%1 has entered %2").arg(playerName).arg(tablenum));
	    handleAddToTable(tablenum, playerName);
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
	case '=':
	  // table command
	  // an additional byte is needed
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
      commsSocket->connectToHost("cesar.boldlygoingnowhere.org", 1988);
      connectStatusLabel->setText("Connecting to server...");
      toggleConnection->setText("Disconnect");

      gameStackedWidget->setCurrentIndex(0);
      roomTable->clearContents();
      roomTable->setRowCount(0);
    }
  else
    {	
      connectStatusLabel->setText("Disconnected from server");
      commsSocket->disconnectFromHost();
      toggleConnection->setText("Connect");
    }
  
}

void MainWindow::serverDisconnection()
{
  connectStatusLabel->setText("You are disconnected.");
  peopleConnected->clear();
  QMessageBox::information(this, "Aerolith Client", "You have been disconnected.");
  toggleConnection->setText("Connect");
      gameStackedWidget->setCurrentIndex(0);
      roomTable->clearContents();
      roomTable->setRowCount(0);
}

void MainWindow::writeUsernameToServer()
{
  // only after connecting!
  blockSize = 0;
  in.setDevice(commsSocket);
  in.setVersion(QDataStream::Qt_4_2);
  currentUsername = username->text();
  
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;// magic byte
  out << (quint16)0; // length 
  out << (quint8)'e';
  out << currentUsername;
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16)); 
  commsSocket->write(block);

}

void MainWindow::sendPM(QListWidgetItem* item)
{
  chatLE->setText(QString("/msg ") + item->text() + " ");

}

void MainWindow::createNewRoom()
{

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;	// magic byte
  out << (quint16)0; // length 
  out << (quint8)'t';
  out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
  out << (quint8)5;
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16)); 
  commsSocket->write(block);
}

void MainWindow::joinTable()
{
  QPushButton* buttonThatSent = static_cast<QPushButton*> (sender());
  QVariant tn = buttonThatSent->property("tablenum");
  quint16 tablenum = (quint16)tn.toInt();
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;	// magic byte
  out << (quint16)0; // length 
  out << (quint8)'j';
  out << (quint16) tablenum;
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16)); 
  commsSocket->write(block);
}

void MainWindow::leaveThisTable()
{
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_4_2);
  out << (quint16)25344;	// magic byte
  out << (quint16)0; // length 
  out << (quint8)'l';
  out << (quint16)currentTablenum;
  out.device()->seek(2); // position of length
  out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16)); 
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
  roomTable->insertRow(roomTable->rowCount());
  roomTable->setItem(roomTable->rowCount()-1, 0, tableNumItem);
  roomTable->setItem(roomTable->rowCount()-1, 1, descriptorItem);
  roomTable->setItem(roomTable->rowCount()-1, 2, playersItem);
  roomTable->setItem(roomTable->rowCount()-1, 3, numPlayersItem);
  roomTable->setItem(roomTable->rowCount()-1, 4, maxPlayersItem);
  roomTable->setCellWidget(roomTable->rowCount()-1, 5, buttonItem);
  // tablenums.insert(tablenum, roomTable->rowCount()-1);
}

void MainWindow::handleDeleteTable(quint16 tablenum)
{
  int row = findRoomTableRow(tablenum);
  
  roomTable->removeRow(row);
}

void MainWindow::handleLeaveTable(quint16 tablenum, QString player)
{
  int row = findRoomTableRow(tablenum);
  /*  if (roomTable->rowCount() - 1 < row)
    QMessageBox::critical(this, "OMG", QString("ZOMG %1 %2").arg(row).arg(roomTable->rowCount()));*/
  QTableWidgetItem* myItem = roomTable->item(row, 2);
  QString textToReplace = myItem->text();
  chatText->append("before: " + textToReplace + " PLAYER: " + player) ;
  textToReplace.replace(player + " ", "");
  chatText->append("after: " + textToReplace);
  myItem->setText(textToReplace);
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
  roomTable->item(row, 2)->setText(roomTable->item(row,2)->text() + player + " ");
  if (curNumPlayers >= maxNumPlayers)
    roomTable->cellWidget(row, 5)->setEnabled(false);
  else
    roomTable->cellWidget(row, 5)->setEnabled(true);
  


}
