#include "mainwindow.h"

#include <QPushButton>
#include <QTabWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QTextEdit>

MainWindow::MainWindow()
{

  setWindowTitle("Aerolith");
  QTabWidget *mainTabWidget = new QTabWidget;
  setCentralWidget(mainTabWidget);
  

  // create login widget
  QWidget *loginWidget = new QWidget;
  QLineEdit *username = new QLineEdit;

  QPushButton *submit = new QPushButton("Submit");
  
  QLabel *statusLabel = new QLabel("Please enter your desired username");  
  QLabel *userLabel = new QLabel("Username: ");
 
  
  QGridLayout *loginWidgetLayout = new QGridLayout;
  loginWidgetLayout->addWidget(userLabel, 0, 0);
  loginWidgetLayout->addWidget(username, 0, 1);
  loginWidgetLayout->addWidget(submit, 3, 1);
  loginWidgetLayout->addWidget(statusLabel, 5, 1);

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

  



  QVBoxLayout *gameBoardLayout = new QVBoxLayout;
  chatLE = new QLineEdit;
  chatText = new QTextEdit;
  gameBoardLayout->addLayout(wordLayout);
  gameBoardLayout->addLayout(solutionLayout);
  gameBoardLayout->addWidget(chatLE);
  gameBoardLayout->addWidget(chatText);

  gameBoardWidget->setLayout(gameBoardLayout);

  mainTabWidget->addTab(gameBoardWidget, "Game Board");

  chatText->document()->setMaximumBlockCount(5000);  // at most 5000 newlines.

  connect(chatLE, SIGNAL(returnPressed()), this, SLOT(submitChatLEContents()));


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
