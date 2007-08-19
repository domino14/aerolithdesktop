#include "playerInfoWidget.h"

PlayerInfoWidget::PlayerInfoWidget()
{
  QGridLayout *playerListsLayout = new QGridLayout;
	
  for (int i = 0; i < 6; i++)
    {
      playerAvatars[i] = new avatarLabel;
      playerAvatars[i]->setFixedWidth(40);
      
      connect(playerAvatars[i], SIGNAL(leftMouseClicked()), this, SLOT(possibleChangeAvatarLeft()));
      connect(playerAvatars[i], SIGNAL(rightMouseClicked()), this, SLOT(possibleChangeAvatarRight()));
      
      playerNames[i] = new QLabel("");
      playerNames[i]->setAlignment(Qt::AlignCenter);
      playerNames[i]->setFixedWidth(80);
      playerLists[i] = new QListWidget();
      playerLists[i]->setFixedWidth(120);
      playerLists[i]->setMinimumHeight(100);
      playerLists[i]->setFrameShape(QFrame::Box);
      playerStatus[i] = new QLabel("");
      playerStatus[i]->setFixedWidth(120);
      playerStatus[i]->setAlignment(Qt::AlignHCenter);
      
      if (i != 0)
	{
	  playerAvatars[i]->hide();
	  playerLists[i]->hide();
	  playerNames[i]->hide();
	  playerStatus[i]->hide();
	}
      playerInfoLayout[i] = new QHBoxLayout;
      playerInfoLayout[i]->addWidget(playerAvatars[i]);
      playerInfoLayout[i]->addWidget(playerNames[i]);
      
      playerListsLayout->addLayout(playerInfoLayout[i], 0, i*2);
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

void PlayerInfoWidget::possibleChangeAvatarLeft()
{
	avatarLabel* clickedLabel = static_cast<avatarLabel*> (sender());
	if (clickedLabel->property("username").toString() == myUsername)
	{
	
		quint8 avatarID = clickedLabel->property("avatarID").toInt();
		if (avatarID == NUM_AVATAR_IDS) avatarID = 1;
		else avatarID++;

		emit avatarChange(avatarID);
	}
}

void PlayerInfoWidget::possibleChangeAvatarRight()
{
	avatarLabel* clickedLabel = static_cast<avatarLabel*> (sender());
	if (clickedLabel->property("username").toString() == myUsername)
	{
	
		quint8 avatarID = clickedLabel->property("avatarID").toInt();
		if (avatarID == 1) avatarID = NUM_AVATAR_IDS;
		else avatarID--;

		emit avatarChange(avatarID);
	}
}

void PlayerInfoWidget::setMyUsername(QString username)
{
	myUsername = username;
}


void PlayerInfoWidget::clearAndHide()
{
  for (int i = 0; i < 6; i++)
    {
      playerNames[i]->setText("");
      playerStatus[i]->setText("");
      playerLists[i]->clear();
      playerAvatars[i]->setText("");
      
      playerLists[i]->hide();
      playerNames[i]->hide();
      playerStatus[i]->hide();
      playerAvatars[i]->hide();

    }
}

void PlayerInfoWidget::setupForGameStart()
{
  for (int i = 0; i < 6; i++)
    {
      playerLists[i]->clear();
      playerStatus[i]->setText("");
    }

}

void PlayerInfoWidget::answered(QString username, QString answer)
{
  if (seats.contains(username))
    {
      int indexOfPlayer = seats.value(username);
        playerLists[indexOfPlayer]->insertItem(0, answer);
	playerLists[indexOfPlayer]->item(0)->setTextAlignment(Qt::AlignCenter);
	playerStatus[indexOfPlayer]->setText(QString("%1 words").arg(playerLists[indexOfPlayer]->count()));
	
    }
}

void PlayerInfoWidget::addPlayers(QStringList playerList)
{
  for (int i = 0; i < playerList.size(); i++)
    {
      playerAvatars[i]->show();
      playerNames[i]->setText(playerList[i]);
      playerNames[i]->show();
      playerLists[i]->show();
      playerStatus[i]->show();
      seats.insert(playerList[i], i);
    }
}

void PlayerInfoWidget::addPlayer(QString player, bool gameStarted)
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
	
  playerAvatars[spot]->show();
  playerNames[spot]->setText(player);
  playerNames[spot]->show();
  playerLists[spot]->show();
  playerStatus[spot]->show();

  seats.insert(player, spot);
  
  if (gameStarted == false)
    for (int i = 0; i < 6; i++)
      playerStatus[i]->setText("");
  
}

void PlayerInfoWidget::removePlayer(QString player, bool gameStarted)
{
  int seat;
  if (seats.contains(player))
    {
      seat = seats.value(player);
    }
  else
    {
      QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10002)");
      return;
    }

  seats.remove(player);
  playerNames[seat]->setText("");
  playerStatus[seat]->setText("");
  playerLists[seat]->clear();
  playerAvatars[seat]->setText("");
  
  playerAvatars[seat]->hide();
  playerNames[seat]->hide();
  playerLists[seat]->hide();
  playerStatus[seat]->hide();


  // clear "Ready" for everyone if someone left.
  if (gameStarted == false)
    for (int i = 0; i < 6; i++)
      playerStatus[i]->setText("");
}

void PlayerInfoWidget::leaveTable()
{
  seats.clear(); // clear seats hash when WE leave.
}

void PlayerInfoWidget::setReadyIndicator(QString username)
{
	int seat;
  if (seats.contains(username))
	  seat = seats.value(username);
  else
  {
	  QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10004)");
      return;
  }
    playerStatus[seat]->setText("Ready.");

}

void PlayerInfoWidget::setAvatar(QString username, quint8 avatarID)
{
	int seat;
  if (seats.contains(username))
	  seat = seats.value(username);
  else
  {
	  QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10005)");
      return;
  }
	playerAvatars[seat]->setPixmap(QString(":images/face%1.png").arg(avatarID));
	playerAvatars[seat]->setProperty("avatarID", QVariant(avatarID));
	playerAvatars[seat]->setProperty("username", QVariant(username));	// shouldn't have to repeat this, 
																		// will eventually come up with an actual seat system

}
//===============================================================//
SinglePlayerInfoWidget::SinglePlayerInfoWidget()
{
  clearAndHide();
}

void SinglePlayerInfoWidget::clearAndHide()
{

}


//===============================================================//
avatarLabel::avatarLabel(QWidget *parent) : QLabel(parent)
{
	left_pressed = false;
	right_pressed = false;

}

void avatarLabel::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		left_pressed = true;
	}
	else if (e->button() == Qt::RightButton)
	{
		right_pressed = true;
	}
}

void avatarLabel::mouseReleaseEvent(QMouseEvent *e)
{
	if (left_pressed && e->button() == Qt::LeftButton && inLabel(e->pos()))
		emit leftMouseClicked();
	else if (right_pressed && e->button() == Qt::RightButton && inLabel(e->pos()))
		emit rightMouseClicked();

	left_pressed = false;
	right_pressed = false;

}

bool avatarLabel::inLabel(const QPoint &p)
{
	return rect().contains(p);
}
//===============================================================//
