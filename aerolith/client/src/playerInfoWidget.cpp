#include "playerInfoWidget.h"

playerInfoWidget::playerInfoWidget()
{
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
  setLayout(playerListsLayout);
}

void playerInfoWidget::clearAndHide()
{
  for (int i = 0; i < 6; i++)
    {
      playerNames[i]->setText("");
      playerStatus[i]->setText("");
      playerLists[i]->clear();

      playerLists[i]->hide();
      playerNames[i]->hide();
      playerStatus[i]->hide();

    }
}

void playerInfoWidget::setupForGameStart()
{
  for (int i = 0; i < 6; i++)
    {
      playerLists[i]->clear();
      playerStatus[i]->setText("");
    }

}

void playerInfoWidget::answered(QString username, QString answer)
{
  if (seats.contains(username))
    {
      int indexOfPlayer = seats.value(username);
        playerLists[indexOfPlayer]->insertItem(0, answer);
	playerLists[indexOfPlayer]->item(0)->setTextAlignment(Qt::AlignCenter);
	playerStatus[indexOfPlayer]->setText(QString("%1 words").arg(playerLists[indexOfPlayer]->count()));
	
    }
}

void playerInfoWidget::addPlayers(QStringList playerList)
{
  for (int i = 0; i < playerList.size(); i++)
    {
      playerNames[i]->setText(playerList[i]);
      playerNames[i]->show();
      playerLists[i]->show();
      playerStatus[i]->show();
      seats.insert(playerList[i], i);
    }
}

void playerInfoWidget::addPlayer(QString player)
{
  bool spotfound = false;
  int spot;

  for (int i = 0; i < 6; i++)
    {
      if (playerNames[i]->text() == "")
	{
	  spotfound = true;
	  spot = i;
	  break;
	}
    }

  if (spotfound == false)
    {
      QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10001");
      return;
    }

  playerNames[spot]->setText(player);
  playerNames[spot]->show();
  playerLists[spot]->show();
  playerStatus[spot]->show();

  seats.insert(player, spot);
}

void playerInfoWidget::removePlayer(QString player, bool gameStarted)
{
  int seat;
  if (seats.contains(player))
    {
      seat = seats.value(player);
    }
  else
    {
      QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10002");
      return;
    }

  seats.remove(player);
  playerNames[seat]->setText("");
  playerStatus[seat]->setText("");
  playerLists[seat]->clear();
  
  playerNames[seat]->hide();
  playerLists[seat]->hide();
  playerStatus[seat]->hide();


  // clear "Ready" for everyone if someone left.
  if (gameStarted == false)
    for (int i = 0; i < 6; i++)
      playerStatus[i]->setText("");
}

void playerInfoWidget::leaveTable()
{
  seats.clear(); // clear seats hash when WE leave.
}

void playerInfoWidget::setReadyIndicator(QString username)
{
  if (seats.contains(username))
    playerStatus[seats.value(username)]->setText("Ready.");

}
