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
  exitTable = new QPushButton("Exit Table #");
  QPushButton *changeFont = new QPushButton("Change font");
	
  connect(changeFont, SIGNAL(clicked()), wordsWidget, SLOT(changeFont()));
  
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
  topSolutionLayout->addWidget(timerDial);
  topSolutionLayout->addSpacing(50);
  topSolutionLayout->addWidget(wordListInfo);
  topSolutionLayout->addSpacing(50);
  topSolutionLayout->addWidget(changeFont);
  topSolutionLayout->addStretch(1);
  
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
  gameBoardLayout->addWidget(playerInfoWidget);
  
  setLayout(gameBoardLayout);

  connect(alpha, SIGNAL(clicked()), wordsWidget, SLOT(alphagrammizeWords()));
  connect(shuffle, SIGNAL(clicked()), wordsWidget, SLOT(shuffleWords()));

}

void UnscrambleGameTable::closeEvent(QCloseEvent* event)
{
  event->ignore();
  exitTable->animateClick();
}

void UnscrambleGameTable::resetTable(quint16 tableNum, QString wordListName)
{
  setWindowTitle(QString("Table %1 - Word List: %2").arg(tableNum).arg(wordListName));
  timerDial->display(0);
  wordListInfo->clear();
  playerInfoWidget->clearAndHide();
  wordsWidget->clearCells();
  exitTable->setText(QString("Exit table %1").arg(tableNum));
}
