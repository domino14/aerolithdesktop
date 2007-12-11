#include "UnscrambleGameTable.h"

GameTable::GameTable(QWidget *parent, Qt::WindowFlags f, int numPlayers) : QWidget(parent, f)
{
	qsrand(QDateTime::currentDateTime().toTime_t());
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
UnscrambleGameTable::UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f, QSqlDatabase wordDb) : GameTable(parent, f, 6)
{


	this->wordDb = wordDb;


	tableUi.setupUi(this);



	connect(tableUi.pushButtonGiveUp, SIGNAL(clicked()), this, SIGNAL(giveUp()));
	connect(tableUi.pushButtonStart, SIGNAL(clicked()), this, SIGNAL(sendStartRequest()));
	connect(tableUi.lineEditSolution, SIGNAL(returnPressed()), this, SLOT(enteredGuess()));
	connect(tableUi.pushButtonExit, SIGNAL(clicked()), this, SIGNAL(exitThisTable()));

	connect(tableUi.listWidgetPeopleInRoom, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(sendPM(QListWidgetItem* )));

	tableUi.textEditChat->setTextInteractionFlags(Qt::TextSelectableByMouse);
	tableUi.textEditChat->document()->setMaximumBlockCount(500);
	tableUi.textEditGuesses->document()->setMaximumBlockCount(500);
	
	connect(tableUi.pushButtonAlphagrams, SIGNAL(clicked()), this, SLOT(alphagrammizeWords()));
	connect(tableUi.pushButtonShuffle, SIGNAL(clicked()), this, SLOT(shuffleWords()));

	connect(tableUi.lineEditChat, SIGNAL(returnPressed()), this, SLOT(enteredChat()));

	// load tilesList and chipsList


	gfxScene.setSceneRect(0, 0, 980, 720);
	tableUi.graphicsView->setScene(&gfxScene);	  	

	tableUi.graphicsView->setBackgroundBrush(QImage(":/images/canvas.png"));

	tableUi.graphicsView->setCacheMode(QGraphicsView::CacheBackground); 
	QGraphicsPixmapItem* tableItem = gfxScene.addPixmap(QPixmap(":/images/table.png"));
	//tableItem->setFlags(QGraphicsItem::ItemIsMovable);
	tableItem->setOffset(QPoint(50, 80));
	tableItem->setZValue(-1);
	tableItem->scale(1.1, 1);






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

	solutionsDialog = new QDialog(this);
	uiSolutions.setupUi(solutionsDialog);
	uiSolutions.solutionsTableWidget->verticalHeader()->hide();

	solutionsDialog->setAttribute(Qt::WA_QuitOnClose, false);
	
	connect(tableUi.pushButtonSolutions, SIGNAL(clicked()), solutionsDialog, SLOT(show()));
	
	// generate gfx items

	// 45 * 24 = 1080. maximum possible number of items.
	for (int i = 0; i < 1125; i++)
	{
		Tile *t = new Tile;
		gfxItems << t;
		gfxScene.addItem(t);
		t->hide();
	
	}

	gfxItemsIndex = 0;

}

UnscrambleGameTable::~UnscrambleGameTable()
{
	qDebug() << "UnscrambleGameTable destructor";
	while (!gfxItems.isEmpty())
		delete gfxItems.takeFirst();

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
	tableUi.textEditGuesses->append(tableUi.lineEditSolution->text());
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
	clearAllWordTiles();
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

void UnscrambleGameTable::clearSolutionsDialog()
{
	uiSolutions.solutionsTableWidget->clearContents();
	uiSolutions.solutionsTableWidget->setRowCount(0);
	uiSolutions.solsLabel->clear();
}

void UnscrambleGameTable::populateSolutionsTable()
{
	QBrush missedColorBrush;
	missedColorBrush.setColor(Qt::red);
	int numTotalSols = 0, numWrong = 0;
	for (int i = 0; i < wordQuestions.size(); i++)
	{
		QStringList theseSols = wordQuestions.at(i).solutions;
		QString alphagram = wordQuestions.at(i).alphagram;

		if (alphagram != "") // if alphagram exists.
		{
			QTableWidgetItem *tableAlphagramItem = new QTableWidgetItem(alphagram);
			tableAlphagramItem->setTextAlignment(Qt::AlignCenter);
			int alphagramRow = uiSolutions.solutionsTableWidget->rowCount();

			for (int i = 0; i < theseSols.size(); i++)
			{
				numTotalSols++;
				uiSolutions.solutionsTableWidget->insertRow(uiSolutions.solutionsTableWidget->rowCount());
				QTableWidgetItem *wordItem = new QTableWidgetItem(theseSols.at(i));
				if (wordDb.isOpen())
				{
					QString backHooks, frontHooks, definition;
					QSqlQuery query;

					query.exec("select front_hooks, back_hooks, definition from words where word = '" + theseSols.at(i) + "'");
					qDebug() << "select front_hooks, back_hooks, definition from words where word = '" + theseSols.at(i) + "'";
					while (query.next())
					{
						frontHooks = query.value(0).toString();
						backHooks = query.value(1).toString();
						definition = query.value(2).toString();
					}
					QTableWidgetItem *backHookItem = new QTableWidgetItem(backHooks);
					uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 3, backHookItem);							
					QTableWidgetItem *frontHookItem = new QTableWidgetItem(frontHooks);
					uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 1, frontHookItem);
					QTableWidgetItem *definitionItem = new QTableWidgetItem(definition);
					uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount()-1, 4, definitionItem);
				}

				if (!rightAnswers.contains(theseSols.at(i)))
				{
					numWrong++;
					wordItem->setForeground(missedColorBrush);
					QFont wordItemFont = wordItem->font();
					wordItemFont.setBold(true);
					wordItem->setFont(wordItemFont);
				}
				wordItem->setTextAlignment(Qt::AlignCenter);
				uiSolutions.solutionsTableWidget->setItem(uiSolutions.solutionsTableWidget->rowCount() - 1, 2, wordItem);

			}
			uiSolutions.solutionsTableWidget->setItem(alphagramRow, 0, tableAlphagramItem);
			
		}
	}	
		uiSolutions.solutionsTableWidget->resizeColumnsToContents();
		double percCorrect;
		if (numTotalSols == 0) percCorrect = 0.0;
		else
			percCorrect = (100.0 * (double)(numTotalSols - numWrong))/(double)(numTotalSols);
		uiSolutions.solsLabel->setText(QString("Number of total solutions: %1   Percentage correct: %2 (%3 of %4)").arg(numTotalSols).arg(percCorrect).arg(numTotalSols-numWrong).arg(numTotalSols));
}

void UnscrambleGameTable::alphagrammizeWords()
{

}

void UnscrambleGameTable::shuffleWords()
{

}

void UnscrambleGameTable::addNewWord(int index, QString alphagram, QStringList solutions, quint8 numNotSolved)
{
	
	wordQuestion thisWord(alphagram, solutions, numNotSolved);
	
	if (numNotSolved > 0)
	{

		double scale = 1.0;
		if (alphagram.length() == 9) scale = 0.9;
		if (alphagram.length() == 10) scale = 0.82;
		if (alphagram.length() > 10) scale = 0.7;
		double chipX, chipY;

		double verticalVariation = 2.0;

		for (int i = 0; i < alphagram.length(); i++)
		{
			//QGraphicsPixmapItem* item = gfxScene.addPixmap(tilesList.at(alphagram.at(i).toAscii() - 'A'));
			//thisRoundItems << item;
			Tile* item = (Tile*)gfxItems.at(gfxItemsIndex);
			gfxItemsIndex++;
			
			item->scale(scale, scale);
			if (index >= 0 && index < 10)
			{
				item->setPos(150 + i*(19.0 * scale), 180 + 24*index + verticalVariation* (double)qrand()/RAND_MAX);
				chipX = 150-19;
				chipY = 180+24*index;
			}
			else if (index >= 10 && index < 23)
			{
				item->setPos(330 + i*(19.0 * scale), 150 + 24*(index-10)+ verticalVariation* (double)qrand()/RAND_MAX);
				chipX = 330-19;
				chipY = 150 + 24*(index-10);
			}
			else if (index >= 23 && index < 35)
			{
				item->setPos(510 + i*(19.0 * scale), 160 + 24*(index-23)+ verticalVariation* (double)qrand()/RAND_MAX);
				chipX = 510 -19;
				chipY = 160+24*(index-23);
			}
			else if (index >=35)
			{
				item->setPos(690 + i*(19.0*scale),180 + 24*(index-35)+ verticalVariation* (double)qrand()/RAND_MAX);
				chipX = 690-19;
				chipY = 180+24*(index-35);
			}
			thisWord.tiles.append(item);
			item->setTileLetter(alphagram.at(i));
			item->show();
		}
		
		if (numNotSolved >9) numNotSolved = 9;
/*
		QGraphicsPixmapItem* item = gfxItems.at(gfxItemsIndex);
		gfxItemsIndex++;
		item->setPixmap(chipsList.at(numNotSolved-1));
		item->setTransformationMode(Qt::SmoothTransformation);
		item->scale(0.85*scale, 0.9*scale);
		item->setPos(chipX, chipY);
		thisWord.chipX = chipX;
		thisWord.chipY = chipY;
		thisWord.previousChip = item;*/
	}

	wordQuestions << thisWord;
}

void UnscrambleGameTable::answeredCorrectly(int index, QString username, QString answer)
{
	QString alphagram = wordQuestions.at(index).alphagram;
	double scale = 1.0;
	if (alphagram.length() == 9) scale = 0.9;
	if (alphagram.length() == 10) scale = 0.82;
	if (alphagram.length() > 10) scale = 0.7;
	wordQuestions[index].numNotYetSolved--;
	int numSolutions = wordQuestions.at(index).numNotYetSolved;
	if (numSolutions > 0)
	{
		/*QGraphicsPixmapItem* item = gfxItems.at(gfxItemsIndex);
		gfxItemsIndex++;
		item->setPixmap(chipsList.at(numSolutions-1));
		item->setTransformationMode(Qt::SmoothTransformation);
		item->scale(0.85*scale, 0.9*scale);
		item->setPos(wordQuestions.at(index).chipX, wordQuestions.at(index).chipY);
		wordQuestions.at(index).previousChip->hide();
		wordQuestions[index].previousChip = item;*/
	}
	else
	{
	//	wordQuestions.at(index).previousChip->hide();
		foreach (QGraphicsItem* item, wordQuestions.at(index).tiles)
			item->hide();
	}

	rightAnswers.insert(answer);

	// maybe later color code by username
}

void UnscrambleGameTable::clearAllWordTiles()
{ 
	
	gfxItemsIndex = 0;
	wordQuestions.clear();

}

void UnscrambleGameTable::setupForGameStart()
{
	for (int i = 0; i < numPlayers; i++)
	{
		playerUis.at(i).listWidgetAnswers->clear();
		playerUis.at(i).labelAddInfo->clear();
	}
	rightAnswers.clear();
}
