#include "UnscrambleGameTable.h"

UnscrambleGameTable::UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{

  wordsWidget = new wordsTableWidget();
  // solution box
  QLabel *solutionLabel = new QLabel("Guess:");
  solutionLE = new QLineEdit;
  solutionLE->setFixedWidth(100);
  solutionLE->setMaxLength(15);
  solutions = new QPushButton("Solutions");
  QPushButton *alpha = new QPushButton("Alpha");
  QPushButton *shuffle = new QPushButton("Shuffle");
  giveup = new QPushButton("Give up");
  start = new QPushButton("Start");
  stand = new QPushButton("Stand up");
  exitTable = new QPushButton("Exit Table #");
  QPushButton *changeFont = new QPushButton("Font toggle");
  QPushButton *plusFont = new QPushButton("+");
  QPushButton *minusFont = new QPushButton("-");

	
  connect(changeFont, SIGNAL(clicked()), wordsWidget, SLOT(changeFont()));
  //  connect(plusFont, SIGNAL(clicked()), wordsWidget, SLOT(increaseFontSize()));
  //connect(minusFont, SIGNAL(clicked()), wordsWidget, SLOT(decreaseFontSize()));

  
  solutions->setFocusPolicy(Qt::NoFocus);
  alpha->setFocusPolicy(Qt::NoFocus);
  shuffle->setFocusPolicy(Qt::NoFocus);
  giveup->setFocusPolicy(Qt::NoFocus);
  start->setFocusPolicy(Qt::NoFocus);
  exitTable->setFocusPolicy(Qt::NoFocus);
  changeFont->setFocusPolicy(Qt::NoFocus);
  QHBoxLayout *topSolutionLayout = new QHBoxLayout;
  
  timerDial = new QLCDNumber(4);
  timerDial->setFixedWidth(50);
  timerDial->setSegmentStyle(QLCDNumber::Flat);
  
  wordListInfo = new QLabel;
  wordListInfo->setFixedWidth(100);
  topSolutionLayout->addWidget(timerDial);
  topSolutionLayout->addSpacing(50);
  topSolutionLayout->addWidget(wordListInfo);
  topSolutionLayout->addSpacing(50);
  topSolutionLayout->addWidget(changeFont);
  topSolutionLayout->addStretch(1);
  topSolutionLayout->addWidget(stand);
  topSolutionLayout->addWidget(giveup);
  topSolutionLayout->addSpacing(50);
  topSolutionLayout->addWidget(exitTable);
  QHBoxLayout *bottomSolutionLayout = new QHBoxLayout;
  
  bottomSolutionLayout->addWidget(solutionLabel);
  bottomSolutionLayout->addWidget(solutionLE);
  bottomSolutionLayout->addStretch(1);
  //bottomSolutionLayout->addWidget(timerDial);
  bottomSolutionLayout->addWidget(start);
  bottomSolutionLayout->addWidget(solutions);
  bottomSolutionLayout->addWidget(alpha);
  bottomSolutionLayout->addWidget(shuffle);
  //solutionLayout->addSpacing(50);
  //solutionLayout->addWidget(giveup);
  //solutionLayout->addWidget(exitTable);
  
  // Players box
  
  playerInfoWidget = new PlayerInfoWidget(); // includes lists, etc
  
  QVBoxLayout *gameBoardLayout = new QVBoxLayout;
  gameBoardLayout->addLayout(topSolutionLayout);
  gameBoardLayout->addWidget(wordsWidget, 0, Qt::AlignHCenter);
  gameBoardLayout->addLayout(bottomSolutionLayout);
  gameBoardLayout->addWidget(playerInfoWidget, 0, Qt::AlignHCenter);

  peopleInTable = new QListWidget;
  chatLE = new QLineEdit;
  tableChat = new QTextEdit;

   connect(peopleInTable, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(sendPM(QListWidgetItem* )));
  chatLE->setMaxLength(300);
  tableChat->setReadOnly(true);
  tableChat->setTextInteractionFlags(Qt::TextSelectableByMouse);
  	
  QHBoxLayout *chatBoxLayout = new QHBoxLayout;
  chatBoxLayout->addWidget(tableChat);
  tableChat->setFrameShape(QFrame::Box);
  peopleInTable->setFrameShape(QFrame::Box);
  peopleInTable->setFixedWidth(150);
  chatBoxLayout->addWidget(peopleInTable);
  
  gameBoardLayout->addWidget(chatLE);
  gameBoardLayout->addSpacing(5);
  gameBoardLayout->addLayout(chatBoxLayout);
  connect(alpha, SIGNAL(clicked()), wordsWidget, SLOT(alphagrammizeWords()));
  connect(shuffle, SIGNAL(clicked()), wordsWidget, SLOT(shuffleWords()));

  setLayout(gameBoardLayout);

  setTabOrder(solutionLE, chatLE);
  setTabOrder(chatLE, solutionLE);
  tableChat->setFocusPolicy(Qt::ClickFocus);
  peopleInTable->setFocusPolicy(Qt::NoFocus);  


  connect(giveup, SIGNAL(clicked()), this, SIGNAL(giveUp()));
  connect(start, SIGNAL(clicked()), this, SIGNAL(sendStartRequest()));
  connect(playerInfoWidget, SIGNAL(avatarChange(quint8)), this, SIGNAL(avatarChange(quint8)));
  connect(solutionLE, SIGNAL(returnPressed()), this, SLOT(enteredGuess()));
  connect(exitTable, SIGNAL(clicked()), this, SIGNAL(exitThisTable()));

  connect(chatLE, SIGNAL(returnPressed()), this, SLOT(enteredChat()));
  //  connect(
  
}

void UnscrambleGameTable::enteredChat()
{
  emit chatTable(chatLE->text());
  chatLE->clear();
}

void UnscrambleGameTable::sendPM(QListWidgetItem* item)
{
	chatLE->setText(QString("/msg ") + item->text() + " ");
	chatLE->setFocus(Qt::OtherFocusReason);
}


void UnscrambleGameTable::enteredGuess()
{

  emit guessSubmitted(solutionLE->text());
  solutionLE->clear();
}

void UnscrambleGameTable::closeEvent(QCloseEvent* event)
{
  event->ignore();
  exitTable->animateClick();
}

void UnscrambleGameTable::resetTable(quint16 tableNum, QString wordListName, QString myUsername, quint8 maxPlayers)
{
  setWindowTitle(QString("Table %1 - Word List: %2 - Logged in as %3").arg(tableNum).arg(wordListName).arg(myUsername));
  timerDial->display(0);
  wordListInfo->clear();
  wordsWidget->clearCells();
  exitTable->setText(QString("Exit table %1").arg(tableNum));
  chatLE->clear();
  tableChat->clear();
  playerInfoWidget->setMaxPlayers(maxPlayers);
  


}

void UnscrambleGameTable::leaveTable()
{
  playerInfoWidget->leaveTable();
  peopleInTable->clear();
  
}

void UnscrambleGameTable::addPlayers(QStringList plist)
{
  playerInfoWidget->addPlayers(plist);
  peopleInTable->addItems(plist);
}

void UnscrambleGameTable::addPlayer(QString player, bool gameStarted)
{
  playerInfoWidget->addPlayer(player, gameStarted);
  peopleInTable->addItem(player);
}

void UnscrambleGameTable::removePlayer(QString player, bool gameStarted)
{
  playerInfoWidget->removePlayer(player, gameStarted);
  for (int i = 0; i < peopleInTable->count(); i++)
    if (peopleInTable->item(i)->text() == player)
      {
	QListWidgetItem *it = peopleInTable->takeItem(i);
	delete it;
      }
}
