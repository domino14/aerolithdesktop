#include "UnscrambleGameTable.h"

GameTable::GameTable(QWidget *parent, Qt::WindowFlags f, int numPlayers) : QWidget(parent, f)
{
	this->numPlayers = numPlayers;

	for (int i = 0; i < numPlayers; i++)
	{
		QWidget* tmpWidget = new QWidget(this);
		Ui::playerInfoForm tempUi;
		tempUi.setupUi(tmpWidget);
		playerUis.append(tempUi);
		playerWidgets.append(tmpWidget);
	}

	connect(playerUis.at(0).labelAvatar, SIGNAL(leftMouseClicked()), this, SLOT(possibleAvatarChangeLeft()));
	connect(playerUis.at(0).labelAvatar, SIGNAL(rightMouseClicked()), this, SLOT(possibleAvatarChangeRight()));

}

void GameTable::clearAndHidePlayers(bool hide)
{
	// assumes setupUi has been called on all these Ui objects. (a widget exists for each)
	for (int i = 0; i < numPlayers; i++)
	{
		playerUis.at(i).labelAvatar->clear();
		playerUis.at(i).labelUsername->clear();
		playerUis.at(i).labelAddInfo->clear();
		playerUis.at(i).listWidgetAnswers->clear();
		if (hide) playerWidgets.at(i)->hide();
	}

}

void GameTable::playerLeaveTable()
{	
	seats.clear();
}	

void GameTable::possibleAvatarChangeLeft()
{
	avatarLabel* clickedLabel = static_cast<avatarLabel*> (sender());
	if (clickedLabel->property("username").toString() == myUsername)
	{
		quint8 avatarID = clickedLabel->property("avatarID").toInt();
		if (avatarID == NUM_AVATAR_IDS) avatarID = 1;
		else avatarID++;

		emit avatarChange(avatarID);
		qDebug() << "emitted avatar change with id: " << avatarID;
	}
}

void GameTable::possibleAvatarChangeRight()
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

void GameTable::setAvatar(QString username, quint8 avatarID)
{
	int seat;
	if (seats.contains(username))
		seat = seats.value(username);
	else
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10005)");
		return;
	}
	playerUis.at(seat).labelAvatar->setPixmap(QString(":images/face%1.png").arg(avatarID));
	playerUis.at(seat).labelAvatar->setProperty("avatarID", QVariant(avatarID));
	playerUis.at(seat).labelAvatar->setProperty("username", QVariant(username));
}

void GameTable::setReadyIndicator(QString username)
{
	int seat;
	if (seats.contains(username))
		seat = seats.value(username);
	else
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10004)");
		return;
	}
	playerUis.at(seat).labelAddInfo->setText("<font color=green>!</font>");

}

void GameTable::setupForGameStart()
{
	for (int i = 0; i < numPlayers; i++)
	{
		playerUis.at(i).listWidgetAnswers->clear();
		playerUis.at(i).labelAddInfo->clear();
	}
}

void GameTable::addToPlayerList(QString username, QString stringToAdd)
{
	if (seats.contains(username))
	{
		int indexOfPlayer = seats.value(username);
		playerUis.at(indexOfPlayer).listWidgetAnswers->insertItem(0, stringToAdd);
		playerUis.at(indexOfPlayer).listWidgetAnswers->item(0)->setTextAlignment(Qt::AlignCenter);
		playerUis.at(indexOfPlayer).labelAddInfo->setText(QString("<font color=red>%1</font>").
			arg(playerUis.at(indexOfPlayer).listWidgetAnswers->count()));

	}
}

void GameTable::addPlayersToWidgets(QStringList playerList)
{
	// adds players, including ourselves
	if (!playerList.contains(myUsername))
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 20001)");
		return;
	}
	else
	{
		// put US at seat #0
		playerList.removeAll(myUsername);
		playerWidgets.at(0)->show();
		playerUis.at(0).labelUsername->setText(myUsername);
		seats.insert(myUsername, 0);
	}

	if (playerList.size() > numPlayers-1)
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 20002)");
		return;
	}

	for (int i = 0; i < playerList.size(); i++)
	{
		playerWidgets.at(i+1)->show();
		playerUis.at(i+1).labelUsername->setText(playerList[i]);
		seats.insert(playerList[i], i+1);
	}
}

void GameTable::addPlayerToWidgets(QString username, bool gameStarted)
{

	// add a player that is NOT us
	bool spotfound = false;
	int spot;

	for (int i = 1; i < numPlayers; i++)
	{
		if (playerUis.at(i).labelUsername->text() == "")
		{
			spotfound = true;
			spot = i;
			break;
		}
	}

	if (spotfound == false)
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10001)");
		return;
	}

	playerWidgets.at(spot)->show();
	playerUis.at(spot).labelUsername->setText(username);
	seats.insert(username, spot);

	if (gameStarted == false)
		for (int i = 0; i < 6; i++)
			playerUis.at(i).labelAddInfo->setText("");
}

void GameTable::removePlayerFromWidgets(QString username, bool gameStarted)
{ 
	// a player has left OUR table
	int seat;
	if (seats.contains(username))
	{
		seat = seats.value(username);
	}
	else
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10002)");
		return;
	}

	seats.remove(username);

	playerWidgets.at(seat)->hide();
	playerUis.at(seat).labelUsername->clear();
	playerUis.at(seat).labelAddInfo->clear();
	playerUis.at(seat).listWidgetAnswers->clear();
	playerUis.at(seat).labelAvatar->clear();

	// clear "Ready" for everyone if someone left.
	if (gameStarted == false)
		for (int i = 0; i < 6; i++)
			playerUis.at(seat).labelAddInfo->clear();
}

void GameTable::setMyUsername(QString username)
{
	myUsername = username;
}


///////////////////////////
UnscrambleGameTable::UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f) : GameTable(parent, f, 6)
{

	tableUi.setupUi(this);


	connect(tableUi.pushButtonSolutions, SIGNAL(clicked()), this, SIGNAL(shouldShowSolutions()));	
	
	connect(tableUi.pushButtonGiveUp, SIGNAL(clicked()), this, SIGNAL(giveUp()));
	connect(tableUi.pushButtonStart, SIGNAL(clicked()), this, SIGNAL(sendStartRequest()));
	connect(tableUi.lineEditSolution, SIGNAL(returnPressed()), this, SLOT(enteredGuess()));
	connect(tableUi.pushButtonExit, SIGNAL(clicked()), this, SIGNAL(exitThisTable()));

	connect(tableUi.listWidgetPeopleInRoom, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(sendPM(QListWidgetItem* )));

	tableUi.textEditChat->setTextInteractionFlags(Qt::TextSelectableByMouse);

	
	//connect(tableUi.pushButtonAlphagrams, SIGNAL(clicked()), wordsWidget, SLOT(alphagrammizeWords()));
	//connect(tableUi.pushButtonShuffle, SIGNAL(clicked()), wordsWidget, SLOT(shuffleWords()));

	connect(tableUi.lineEditChat, SIGNAL(returnPressed()), this, SLOT(enteredChat()));

	// load tilesList and chipsList

	for (int i = 0; i < 26; i++)
	{
		QPixmap pix(QString(":/images/%1.png").arg((char)(i + 'A')));
		tilesList.append(pix);
	}

	for (int i = 0; i < 9; i++)
	{
		QPixmap pix(QString(":/images/chip%1.png").arg(i + 1));
		chipsList.append(pix);
	}


	gfxScene.setSceneRect(0, 0, 980, 720);
	tableUi.graphicsView->setScene(&gfxScene);	  	

	tableUi.graphicsView->setBackgroundBrush(QImage(":/images/canvas.png"));

	tableUi.graphicsView->setCacheMode(QGraphicsView::CacheBackground); 
	QGraphicsPixmapItem* tableItem = gfxScene.addPixmap(QPixmap(":/images/table.png"));
	//tableItem->setFlags(QGraphicsItem::ItemIsMovable);
	tableItem->setOffset(QPoint(50, 80));
	tableItem->setZValue(-1);
	tableItem->scale(1.1, 1);

	for (int i = 0; i < 26; i++)
	{

		QGraphicsPixmapItem* aItem = gfxScene.addPixmap(tilesList.at(i));
		aItem->setFlags(QGraphicsItem::ItemIsMovable);
		aItem->setPos(i*30, 0);
		//aItem->scale(0.7, 0.7);
		aItem->setTransformationMode(Qt::SmoothTransformation);
	}

	for (int i = 0; i < 9; i++)
	{
		QGraphicsPixmapItem* bItem = gfxScene.addPixmap(chipsList.at(i));
		bItem->setFlags(QGraphicsItem::ItemIsMovable);
		bItem->setPos(i*30, 30);
		bItem->scale(0.7, 0.7);
		bItem->setTransformationMode(Qt::SmoothTransformation);
	}


	for (int i = 0; i < 10; i++)
		gfxScene.addRect(120, 180 + (24*i), 170, 22)->setFlags(QGraphicsItem::ItemIsMovable);

	for (int i = 0; i < 16; i++)
		gfxScene.addRect(300, 130 + (24*i), 170, 22)->setFlags(QGraphicsItem::ItemIsMovable);

	for (int i = 0; i < 16; i++)
		gfxScene.addRect(480, 130 + (24*i), 170, 22)->setFlags(QGraphicsItem::ItemIsMovable);

	for (int i = 0; i < 10; i++)
		gfxScene.addRect(660, 180 + (24*i), 170, 22)->setFlags(QGraphicsItem::ItemIsMovable);





	setWindowIcon(QIcon(":/images/aerolith.png"));
	this->setWindowFlags(Qt::Dialog);

	move(0, 0);


	for (int i = 0; i < 6; i++)
		playerWidgets.at(i)->raise();
	
	playerWidgets.at(0)->move(220, 510);
	playerWidgets.at(1)->move(650, 510);
	playerWidgets.at(2)->move(870, 250);
	playerWidgets.at(3)->move(650, 10);
	playerWidgets.at(4)->move(220, 10);
	playerWidgets.at(5)->move(10, 250);	

	//connect(

}

void UnscrambleGameTable::enteredChat()
{
	emit chatTable(tableUi.lineEditChat->text());
	tableUi.lineEditChat->clear();

}

void UnscrambleGameTable::sendPM(QListWidgetItem* item)
{
	tableUi.lineEditChat->setText(QString("/msg ") + item->text() + " ");
	tableUi.lineEditChat->setFocus(Qt::OtherFocusReason);
}


void UnscrambleGameTable::enteredGuess()
{
	emit guessSubmitted(tableUi.lineEditSolution->text());
	tableUi.lineEditSolution->clear();
}

void UnscrambleGameTable::closeEvent(QCloseEvent* event)
{
	event->ignore();
	tableUi.pushButtonExit->animateClick();
}

void UnscrambleGameTable::resetTable(quint16 tableNum, QString wordListName, QString myUsername)
{
	setWindowTitle(QString("Table %1 - Word List: %2 - Logged in as %3").arg(tableNum).arg(wordListName).arg(myUsername));
	tableUi.labelWordListInfo->clear();
	tableUi.lcdNumberTimer->display(0);
	clearAndHidePlayers(true);
//	wordsWidget->clearCells();			// instead get rid of all tiles on table
	tableUi.pushButtonExit->setText(QString("Exit table %1").arg(tableNum));
	tableUi.lineEditChat->clear();
	tableUi.textEditChat->clear();

}

void UnscrambleGameTable::leaveTable()
{
	playerLeaveTable();	// clear the seats hash
	tableUi.listWidgetPeopleInRoom->clear();
}

void UnscrambleGameTable::addPlayers(QStringList plist)
{
	tableUi.listWidgetPeopleInRoom->addItems(plist);
	addPlayersToWidgets(plist);
}

void UnscrambleGameTable::addPlayer(QString player, bool gameStarted)
{
	addPlayerToWidgets(player, gameStarted);
	tableUi.listWidgetPeopleInRoom->addItem(player);
}

void UnscrambleGameTable::removePlayer(QString player, bool gameStarted)
{
	removePlayerFromWidgets(player, gameStarted);
	for (int i = 0; i < tableUi.listWidgetPeopleInRoom->count(); i++)
		if (tableUi.listWidgetPeopleInRoom->item(i)->text() == player)
		{
			QListWidgetItem *it = tableUi.listWidgetPeopleInRoom->takeItem(i);
			delete it;
		}
}


void UnscrambleGameTable::gotChat(QString chat)
{
	tableUi.textEditChat->append(chat);
}




void UnscrambleGameTable::gotTimerValue(quint16 timerval)
{
	tableUi.lcdNumberTimer->display(timerval);
}

void UnscrambleGameTable::gotWordListInfo(QString info)
{
	tableUi.labelWordListInfo->setText(info);
}




