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

  
  QWidget *gameBoardWidget = new QWidget;	 // the 'overall' widget
  
  QTableWidget *wordsWidget = new QTableWidget(9, 5);

	wordsWidget->horizontalHeader()->hide();

  wordsWidget->setSelectionMode(QAbstractItemView::NoSelection);
  wordsWidget->verticalHeader()->hide();
  for (int i = 0; i < 5; i++)
	wordsWidget->setColumnWidth(i, 150);
  for (int i = 0; i < 9; i++)
	wordsWidget->setRowHeight(i, 20);

  wordsWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  wordsWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  wordsWidget->setFixedSize(752,182);	// this feels extremely cheap and i hate it

  /*
  {
  float available( wordsWidget->viewport()->width() );
  for ( int col = 0; col < wordsWidget->columnCount(); ++col )
      available -= wordsWidget->columnWidth( col );
  float perColumn = available / wordsWidget->columnCount();
  for ( int col = 0; col < wordsWidget->columnCount(); ++col )
      wordsWidget->setColumnWidth(col, wordsWidget->columnWidth( col ) + perColumn );
  }{
  float available( wordsWidget->viewport()->height() );
  for ( int row = 0; row < wordsWidget->rowCount(); ++row )
      available -= wordsWidget->rowHeight(row);
  float perRow = available / wordsWidget->rowCount();
  for ( int row = 0; row < wordsWidget->rowCount(); ++row )
	  wordsWidget->setRowHeight(row, wordsWidget->rowHeight(row) + perRow );
  }*/
//  wordsWidget->setFixedSize(wordsWidget->maximumViewportSize());
  //roomTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);

  // solution box
  QLabel *solutionLabel = new QLabel("Guess:");
  solutionLE = new QLineEdit;
  solutionLE->setFixedWidth(100);
  QPushButton *alpha = new QPushButton("Alpha");
  QPushButton *shuffle = new QPushButton("Shuffle");
  
  QHBoxLayout *solutionLayout = new QHBoxLayout;
  solutionLayout->addWidget(solutionLabel);
  solutionLayout->addWidget(solutionLE);
  solutionLayout->addSpacing(300);
  solutionLayout->addWidget(alpha);
  solutionLayout->addWidget(shuffle);

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

		if (i > 1) { playerLists[i]->hide(); playerNames[i]->hide(); }
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
  
  QTableWidget *roomTable = new QTableWidget(0, 5);
  QStringList headerLabels;
  headerLabels << "#" << "Word List" << "Players" << "Max # Players"<< "Join?";
  roomTable->setHorizontalHeaderLabels(headerLabels);
  roomTable->setGridStyle(Qt::NoPen);
  roomTable->setSelectionMode(QAbstractItemView::NoSelection);
  roomTable->verticalHeader()->hide();
  roomTable->setColumnWidth(0, 30);
  roomTable->setColumnWidth(1, 200);
  roomTable->setColumnWidth(2, 300);
  roomTable->setColumnWidth(3, 100);
  roomTable->setColumnWidth(4, 75);
  roomTable->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
  
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
  
  
  blockSize = 0; 




  
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
      
      switch(packetType)
	{
	case 'L':
	  {
	    QString username;
	    in >> username;
	    QListWidgetItem *it = new QListWidgetItem(username, peopleConnected);
	    if (username == currentUsername) connectStatusLabel->setText("You have connected!");
	  }
	  break;
	case 'X':
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
	  
	case '!':
	  {
	    QString errorString;
	    in >> errorString;
	    QMessageBox::information(this, "Aerolith client", errorString);
	  }
	  break;
	case 'C':
	  {
	    QString username;
	    in >> username;
	    QString text;
	    in >> text;
	    chatText->append(QString("[")+username+"] " + text);
	  }
	  break;
	case 'P':
	  {
	    QString username, message;
	    in >> username >> message;
	    chatText->append(QString("[")+username+ " writes to you] " + message);
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
      commsSocket->connectToHost("cesar.boldlygoingnowhere.org", 1988);
      connectStatusLabel->setText("Connecting to server...");
      toggleConnection->setText("Disconnect");
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
  out << (quint16)25344;	// magic byte
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
  gameStackedWidget->setCurrentIndex(1);
}
