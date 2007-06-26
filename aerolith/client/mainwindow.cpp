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

  
  QWidget *gameBoardWidget = new QWidget;
  
  QLabel *words[45];

  QWidget *wordsWidget = new QWidget;
  for (int i = 0; i < 45; i++)
    {

      words[i] = new QLabel(QString("AEINNORS"));
      words[i]->setFixedWidth(150);
      words[i]->setFrameShape(QFrame::Panel);
      words[i]->setAlignment(Qt::AlignHCenter);
    }
  
  

  QGridLayout *wordLayout = new QGridLayout;
  int wordsIndex = 0;
  for (int i =0; i < 9; i++)
    for (int j = 0; j < 5; j++)
      {
	wordLayout->addWidget(words[wordsIndex], i, j);
	wordsIndex++;
      }
  wordLayout->setSpacing(0);
  wordsWidget->setLayout(wordLayout);
  wordsWidget->setFixedWidth(780);
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
  QSplitter *playerListsSplitter = new QSplitter;
  for (int i = 0; i < 6; i++)
  {
    playerLists[i] = new QListWidget();
    playerLists[i]->setMaximumWidth(150);
    playerLists[i]->setMinimumHeight(250);
    if (i > 0) playerLists[i]->hide();
    playerListsSplitter->addWidget(playerLists[i]);
    playerLists[i]->setFrameShape(QFrame::Box);
  }

  QVBoxLayout *gameBoardLayout = new QVBoxLayout;
  gameBoardLayout->addWidget(wordsWidget);
  gameBoardLayout->addSpacing(10);
  gameBoardLayout->addLayout(solutionLayout);
  gameBoardLayout->addSpacing(10);
  gameBoardLayout->addWidget(playerListsSplitter);
  
  gameBoardGroupBox->setLayout(gameBoardLayout);
  
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

  QVBoxLayout *overallGameBoardLayout = new QVBoxLayout;
  overallGameBoardLayout->addWidget(gameBoardGroupBox);
  overallGameBoardLayout->setAlignment(gameBoardGroupBox, Qt::AlignHCenter);
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
