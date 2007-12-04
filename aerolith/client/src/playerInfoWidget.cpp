#include "playerInfoWidget.h"

PlayerInfoWidget::PlayerInfoWidget()
{


	QHBoxLayout *playerInfoWidgetsLayout = new QHBoxLayout;
	for (int i = 0; i < 6; i++)
	{
		places[i] = new SinglePlayerInfoWidget(this, i);
		connect(places[i]->thisAvatarLabel, SIGNAL(leftMouseClicked()), this, SLOT(possibleChangeAvatarLeft()));
		connect(places[i]->thisAvatarLabel, SIGNAL(rightMouseClicked()), this, SLOT(possibleChangeAvatarRight()));
		playerInfoWidgetsLayout->addWidget(places[i]);
		//      playerInfoWidgetsLayout->addStretch(1);
	}
	setLayout(playerInfoWidgetsLayout);

}

void PlayerInfoWidget::possibleChangeAvatarLeft()
{
	avatarLabel* clickedLabel = static_cast<avatarLabel*> (sender());
	if (clickedLabel->username == myUsername)
	{

		quint8 avatarId = clickedLabel->avatarId;
		if (avatarId == NUM_AVATAR_IDS) avatarId = 1;
		else avatarId++;

		emit avatarChange(avatarId);
	}
}

void PlayerInfoWidget::possibleChangeAvatarRight()
{
	avatarLabel* clickedLabel = static_cast<avatarLabel*> (sender());
	if (clickedLabel->username == myUsername)
	{

		quint8 avatarId = clickedLabel->avatarId;
		if (avatarId == 1) avatarId = NUM_AVATAR_IDS;
		else avatarId--;

		emit avatarChange(avatarId);
	}
}

void PlayerInfoWidget::setMyUsername(QString username)
{
	myUsername = username;
}

quint8 PlayerInfoWidget::getMySeat()
{
	return mySeat;
}
void PlayerInfoWidget::setMaxPlayers(quint8 maxPlayers)
{
	for (int i = 0; i < 6; i++)
		places[i]->hide();
	for (int i = 0; i < maxPlayers; i++)
	{
		places[i]->show();
	}

}

void PlayerInfoWidget::clearAndHide()
{
	/*
	for (int i = 0; i < 6; i++)
	places[i]->clearAndHide();
	*/
	/*
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

	}*/
}

void PlayerInfoWidget::setupForGameStart()
{
	for (int i = 0; i < 6; i++)
	{
		//places[i]->setupForGameStart();
		/*    playerLists[i]->clear();
		playerStatus[i]->setText("");*/
	}

}

void PlayerInfoWidget::answered(QString username, QString answer)
{
	if (seats.contains(username))
	{

		int indexOfPlayer = seats.value(username);
		//places[indexOfPlayer]->answered(answer);

		/*
		playerLists[indexOfPlayer]->insertItem(0, answer);
		playerLists[indexOfPlayer]->item(0)->setTextAlignment(Qt::AlignCenter);
		playerStatus[indexOfPlayer]->setText(QString("%1 words").arg(playerLists[indexOfPlayer]->count()));
		*/
	}
}

void PlayerInfoWidget::addPlayers(QStringList playerList)
{
	/*
	for (int i = 0; i < playerList.size(); i++)
	{
	playerAvatars[i]->show();
	playerNames[i]->setText(playerList[i]);
	playerNames[i]->show();
	playerLists[i]->show();
	playerStatus[i]->show();
	seats.insert(playerList[i], i);
	}
	*/
}

void PlayerInfoWidget::addPlayer(QString player, bool gameStarted)
{
	/*
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
	*/

}

void PlayerInfoWidget::removePlayer(QString player, bool gameStarted)
{
	/*
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

	*/
}

void PlayerInfoWidget::leaveTable()
{
	seats.clear(); // clear seats hash when WE leave.
}

void PlayerInfoWidget::setReadyIndicator(QString username)
{
	/*
	int seat;
	if (seats.contains(username))
	seat = seats.value(username);
	else
	{
	QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10004)");
	return;
	}
	playerStatus[seat]->setText("Ready.");
	*/
}


void PlayerInfoWidget::setAvatar(QString username, quint8 avatarID)
{
	/*
	int seat;
	if (seats.contains(username))
	seat = seats.value(username);
	else
	{
	QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10005)");
	return;
	}
	playerAvatars[seat]->setPixmap(QString(":images/face%1.png").arg(avatarID));
	playerAvatars[seat]->avatarId = avatarID;
	playerAvatars[seat]->username = username;
	// will eventually come up with an actual seat system
	*/
}
//===============================================================//
SinglePlayerInfoWidget::SinglePlayerInfoWidget(QWidget* parent, quint8 seatNumber) : QWidget(parent)
{
	this->seatNumber = seatNumber;
	//  clearAndHide();
	//QGridLayout *playerListLayout = new QGridLayout;
	setupUi(this);

}

/*void SinglePlayerInfoWidget::clearAndHide()
{

}*/


//===============================================================//

//===============================================================//
