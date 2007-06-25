#include "mainwindow.h"
#include <QBuffer>

MainWindow::MainWindow()
{
  
  setWindowTitle("Aerolith");
  QTabWidget *mainTabWidget = new QTabWidget;
  setCentralWidget(mainTabWidget);
  

  // create login widget
  QWidget *loginWidget = new QWidget;
  username = new QLineEdit;
 
  QPushButton *submit = new QPushButton("Submit");
  
  connectStatusLabel = new QLabel("Please enter your desired username");  
  QLabel *userLabel = new QLabel("Username: ");
  username->setFixedWidth(150);
  username->setMaxLength(16);
  submit->setFixedWidth(150);
  connectStatusLabel->setFixedWidth(600);
  
  QGridLayout *loginWidgetLayout = new QGridLayout;
  loginWidgetLayout->addWidget(userLabel, 0, 0);
  loginWidgetLayout->addWidget(username, 0, 1);
  loginWidgetLayout->addWidget(submit, 3, 1);
  loginWidgetLayout->addWidget(connectStatusLabel, 5, 1);

  QHBoxLayout *overallLoginWidgetLayout = new QHBoxLayout;
  overallLoginWidgetLayout->addLayout(loginWidgetLayout);
  overallLoginWidgetLayout->addStretch(1);
  
  loginWidget->setLayout(overallLoginWidgetLayout);
  
  mainTabWidget->addTab(loginWidget, "Login window");

  // create game board widget

  QWidget *gameBoardWidget = new QWidget;

  QLabel *words[45];

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
    if (i > 0) playerLists[i]->hide();
    playerListsSplitter->addWidget(playerLists[i]);
  }

  
  

  // chat related stuff

  QVBoxLayout *gameBoardLayout = new QVBoxLayout;
  chatLE = new QLineEdit;
  chatText = new QTextEdit;
  gameBoardLayout->addSpacing(20);
  gameBoardLayout->addLayout(wordLayout);
  gameBoardLayout->addSpacing(20);
  gameBoardLayout->addLayout(solutionLayout);
  gameBoardLayout->addSpacing(20);
  gameBoardLayout->addWidget(playerListsSplitter);
  gameBoardLayout->addStretch(1);
  gameBoardLayout->addWidget(chatLE);
  gameBoardLayout->addSpacing(20);

  QHBoxLayout *chatLayout = new QHBoxLayout;
  chatLayout->addWidget(chatText);
  QListWidget* peopleConnected = new QListWidget;
  peopleConnected->setFixedWidth(150);
  chatLayout->addWidget(peopleConnected);
  gameBoardLayout->addLayout(chatLayout);

  gameBoardWidget->setLayout(gameBoardLayout);
  mainTabWidget->addTab(gameBoardWidget, "Game Board");
  


  //mainTabWidget->setTabEnabled(1, false);
  chatText->document()->setMaximumBlockCount(5000);  // at most 5000 newlines.

  connect(chatLE, SIGNAL(returnPressed()), this, SLOT(submitChatLEContents()));

  // commsSocket

  commsSocket = new QTcpSocket;
  connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
  connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)), 
	  this, SLOT(displayError(QAbstractSocket::SocketError)));
  connect(commsSocket, SIGNAL(connected()), this, SLOT(writeUsernameToServer()));
  connect(submit, SIGNAL(clicked()), this, SLOT(connectToServer()));

  buffer = new QBuffer(this);
  buffer->open(QIODevice::ReadWrite);



}

void MainWindow::submitChatLEContents()
{
  chatText->append(QString("From chat: ") + chatLE->text());
  chatLE->clear();
}


void MainWindow::submitSolutionLEContents()
{
  chatText->append(QString("From solution: ") + solutionLE->text());
  solutionLE->clear();
}

void MainWindow::readFromServer()
{
  qint64 bytes = buffer->write(commsSocket->readAll());
  buffer->seek(buffer->pos() - bytes);
  while (buffer->canReadLine())
    {
      QString line = buffer->readLine();
      processServerString(line);
      //      chatText->append(line.simplified());
    }


}

void MainWindow::processServerString(QString line)
{



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
}

void MainWindow::connectToServer()
{
  commsSocket->abort();
  commsSocket->connectToHost("cesar.boldlygoingnowhere.org", 1988);
  connectStatusLabel->clear();
}

void MainWindow::writeUsernameToServer()
{
  // only after connecting!
  commsSocket->write("LOGIN " + username->text().toAscii() + "\n");

}
