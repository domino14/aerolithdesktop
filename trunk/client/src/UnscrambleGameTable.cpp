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

GameTable::~GameTable()
{
	qDebug() << "GameTable destructor";
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
	clearReadyIndicators();

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


void GameTable::addToPlayerList(QString username, QString stringToAdd)
{
	if (seats.contains(username))
	{
		int indexOfPlayer = seats.value(username);
		playerUis.at(indexOfPlayer).listWidgetAnswers->insertItem(0, stringToAdd);
		playerUis.at(indexOfPlayer).listWidgetAnswers->item(0)->setTextAlignment(Qt::AlignCenter);
		playerUis.at(indexOfPlayer).labelAddInfo->setText(QString("<font color=black><b>%1</b></font>").
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
	clearReadyIndicators();
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
	  clearReadyIndicators();
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


	preferencesWidget = new QWidget(this);
	uiPreferences.setupUi(preferencesWidget);
	preferencesWidget->setWindowFlags(Qt::Dialog);
	connect(tableUi.pushButtonPreferences, SIGNAL(clicked()), preferencesWidget, SLOT(show()));

	connect(uiPreferences.comboBoxTileColor, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTileColors(int)));
	connect(uiPreferences.comboBoxFontColor, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFontColors(int)));
	connect(uiPreferences.comboBoxTableStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTableStyle(int)));
	connect(uiPreferences.checkBoxTileBorders, SIGNAL(toggled(bool)), this, SLOT(changeTileBorderStyle(bool)));
	connect(uiPreferences.checkBoxRandomVerticalPositions, SIGNAL(toggled(bool)), this, SLOT(changeVerticalVariation(bool)));
	connect(uiPreferences.pushButtonSavePrefs, SIGNAL(clicked()), this, SLOT(saveUserPreferences()));
	connect(uiPreferences.comboBoxBackground, SIGNAL(currentIndexChanged(int)), this, SLOT(changeBackground(int)));

	connect(tableUi.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));
	tableUi.textEditChat->setTextInteractionFlags(Qt::TextSelectableByMouse);
	tableUi.textEditChat->document()->setMaximumBlockCount(500);
	tableUi.textEditGuesses->document()->setMaximumBlockCount(500);
	
	connect(tableUi.pushButtonAlphagrams, SIGNAL(clicked()), this, SLOT(alphagrammizeWords()));
	connect(tableUi.pushButtonShuffle, SIGNAL(clicked()), this, SLOT(shuffleWords()));

	connect(tableUi.lineEditChat, SIGNAL(returnPressed()), this, SLOT(enteredChat()));

	// load tilesList and chipsList


	gfxScene.setSceneRect(0, 0, 980, 720);
	tableUi.graphicsView->setSceneRect(0, 0, 980, 720);
	tableUi.graphicsView->setScene(&gfxScene);	  	
	tableUi.graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	tableUi.graphicsView->setBackgroundBrush(QImage(":/images/canvas.png"));

	tableUi.graphicsView->setCacheMode(QGraphicsView::CacheBackground); 
	tableItem = gfxScene.addPixmap(QPixmap(":/images/table.png"));
	tableItem->setOffset(QPoint(50, 80));
	tableItem->setZValue(-1);
	tableItem->scale(1.1, 1);
	
	setWindowIcon(QIcon(":/images/aerolith.png"));

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
	preferencesWidget->setAttribute(Qt::WA_QuitOnClose, false);
	
	connect(tableUi.pushButtonSolutions, SIGNAL(clicked()), solutionsDialog, SLOT(show()));
	
	// generate gfx items
	
	// 45 letters * 15 = 
	for (int i = 0; i < 675; i++)
	{
		Tile *t = new Tile;
		tiles << t;
		gfxScene.addItem(t);
		t->hide();
		connect(t, SIGNAL(mousePressed()), this, SLOT(tileWasClicked()));
	}

	for (int i = 0; i < 45; i++)
	{
		Chip *c = new Chip;
		chips << c;
		gfxScene.addItem(c);
		c->hide();
	}

	for (int i = 0; i < numPlayers; i++)
	{
		Chip *c = new Chip;
		gfxScene.addItem(c);
		c->setChipString("");
		c->hide();
		QBrush readyChipBrush = QBrush(Qt::black);
		QPen foregroundPen = QPen(Qt::white);
		QPen edgePen = QPen(QColor(1, 1, 1).lighter(150), 0);
		c->setChipProperties(readyChipBrush, foregroundPen, edgePen);
		readyChips << c;
	}

	readyChips.at(0)->setPos(260, 470);
	readyChips.at(1)->setPos(690, 470);
	readyChips.at(2)->setPos(810, 290);
	readyChips.at(3)->setPos(690, 150);
	readyChips.at(4)->setPos(260, 150);
	readyChips.at(5)->setPos(150, 290);
	
	verticalVariation = 2.0;


	loadUserPreferences();
}

UnscrambleGameTable::~UnscrambleGameTable()
{
	qDebug() << "UnscrambleGameTable destructor";
	while (!chips.isEmpty())
		delete chips.takeFirst();
	while (!tiles.isEmpty())
		delete tiles.takeFirst();

}

void UnscrambleGameTable::changeTileColors(int option)
{

	QLinearGradient linearGrad(QPointF(0, 0), QPointF(18, 18));
	QBrush tileBrush;
	switch(option)
	{
	case 0:	//light blue
	
		linearGrad.setColorAt(0, QColor(7, 9, 184).lighter(200));			
		linearGrad.setColorAt(1, QColor(55, 75, 175).lighter(200));			
		tileBrush = QBrush(linearGrad);
		break;
	case 1:	// dark blue
		linearGrad.setColorAt(0, QColor(7, 9, 184));			
		linearGrad.setColorAt(1, QColor(55, 75, 175));			
		tileBrush = QBrush(linearGrad);
		break;
	case 2:	// light red
		linearGrad.setColorAt(0, QColor(184, 7, 9).lighter(200));			
		linearGrad.setColorAt(1, QColor(175, 55, 75).lighter(200));			
		tileBrush = QBrush(linearGrad);
		break;
	case 3:	// dark red
		linearGrad.setColorAt(0, QColor(184, 7, 9));			
		linearGrad.setColorAt(1, QColor(175, 55, 75));			
		tileBrush = QBrush(linearGrad);
		break;
	case 4:	// black
		linearGrad.setColorAt(0, QColor(55, 55, 55));			
		linearGrad.setColorAt(1, QColor(0, 0, 0));			
		tileBrush = QBrush(linearGrad);
		break;
	case 5:	// white
		linearGrad.setColorAt(0, QColor(255, 255, 255));			
		linearGrad.setColorAt(1, QColor(255, 255, 255));			
		tileBrush = QBrush(linearGrad);
		break;
	}
	QPen edgePen;
	if (uiPreferences.checkBoxTileBorders->isChecked()) edgePen = QPen(Qt::black, 2);
	else edgePen = QPen(tileBrush, 2);

	foreach (Tile* tile, tiles)
	{
		tile->setTileBrush(tileBrush);
		tile->setEdgePen(edgePen);
	}


	gfxScene.update();
}

void UnscrambleGameTable::changeFontColors(int option)
{
	QPen foregroundPen;
	if (option == 0)
		// black
	{
		foregroundPen = QPen(Qt::black);

	}
	else if (option == 1)
		// white
	{
		foregroundPen = QPen(Qt::white);
	}
	foreach (Tile* tile, tiles)
	{
		tile->setForegroundPen(foregroundPen);
	}
	gfxScene.update();
}

void UnscrambleGameTable::changeTableStyle(int index)
{
	if (index == 0)
	{
		
		tableItem->setPixmap(QPixmap(":/images/table.png"));
		tableItem->show();
	}
	else if (index == 1)
	{
		tableItem->setPixmap(QPixmap(":/images/table2.png"));
		tableItem->show();
	}
	else if (index == 2)
	{
		tableItem->hide();
	}
}

void UnscrambleGameTable::changeTileBorderStyle(bool borderOn)
{
	QBrush tileBrush = tiles.at(0)->getTileBrush();
	QPen edgePen;
	if (borderOn)
		edgePen = QPen(Qt::black, 2);
	else
		edgePen = QPen(tileBrush, 2);

	foreach (Tile* tile, tiles)
	{
		tile->setEdgePen(edgePen);
	}
	gfxScene.update();
}

void UnscrambleGameTable::changeVerticalVariation(bool vert)
{
	if (vert) verticalVariation = 2.0;
	else verticalVariation = 0.0;

	foreach (wordQuestion wq, wordQuestions)
	{
		if (wq.numNotYetSolved > 0)
		{
			foreach(Tile* tile, wq.tiles)
			{
				tile->setPos(tile->x(), wq.chip->y() + verticalVariation* (double)qrand()/RAND_MAX);
			}
		}
	}
}

void UnscrambleGameTable::changeBackground(int index)
{
	if (index == 0) tableUi.graphicsView->setBackgroundBrush(QImage(":/images/canvas.png"));
	if (index == 1) tableUi.graphicsView->setBackgroundBrush(QBrush(Qt::white));
	if (index == 2) tableUi.graphicsView->setBackgroundBrush(QImage(":/images/lava.png"));
	if (index == 3) tableUi.graphicsView->setBackgroundBrush(QImage(":/images/stars.png"));
	
}

void UnscrambleGameTable::setZoom(int zoom)
{
	QMatrix matrix;
	matrix.scale((double)zoom/50.0, (double)zoom/50.0);

	tableUi.graphicsView->setMatrix(matrix);

}

void UnscrambleGameTable::setReadyIndicator(QString username)
{
	int seat;
	if (seats.contains(username))
		seat = seats.value(username);
	else
	{
		QMessageBox::critical(0, "?", "Please notify developer about this error. (Error code 10004)");
		return;
	}
	//	playerUis.at(seat).labelAddInfo->setText("<font color=green>!</font>");

	readyChips.at(seat)->show();
	


}

void UnscrambleGameTable::clearReadyIndicators()
{
  for (int i = 0; i < numPlayers; i++)
    readyChips.at(i)->hide();


}



void UnscrambleGameTable::enteredChat()
{
	emit chatTable(tableUi.lineEditChat->text());
	tableUi.lineEditChat->clear();

}

void UnscrambleGameTable::sendPM(QListWidgetItem* item)
{
//	tableUi.lineEditChat->setText(QString("/msg ") + item->text() + " ");
//	tableUi.lineEditChat->setFocus(Qt::OtherFocusReason);
	emit sendPM(item->text());
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
	tableUi.pushButtonExit->setText(QString("Exit table %1").arg(tableNum));
	tableUi.lineEditChat->clear();
	tableUi.textEditChat->clear();
	tableUi.graphicsView->centerOn(tableItem);

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
	
	foreach (wordQuestion wq, wordQuestions)
	{
		// server always sends alphagram, so arrange tiles in order
		double scale = getScaleFactor(wq.alphagram.length());
		// chipX, chipY is 19 to the left of the tile
		for (int i = 0; i < wq.tiles.size(); i++)
		{
			wq.tiles.at(i)->setPos(wq.chip->x() + 19*scale*(i+1), wq.chip->y() + verticalVariation* (double)qrand()/RAND_MAX);
		}
	}

}

void UnscrambleGameTable::shuffleWords()
{
	foreach (wordQuestion wq, wordQuestions)
	{
		// chipX, chipY is 19 to the left of the tile
		for (int i = 0; i < wq.tiles.size(); i++)
		{
			swapXPos(wq.tiles.at(i), wq.tiles.at(qrand() % wq.tiles.size()));
			wq.tiles.at(i)->setPos(wq.tiles.at(i)->x(), wq.chip->y() + verticalVariation* (double)qrand()/RAND_MAX);
		}
	}
}

void UnscrambleGameTable::tileWasClicked()
{
	Tile* tile = static_cast<Tile*> (sender());
	// find which word question it belongs to
	int index = tile->data(0).toInt();
	if (index >= 0 && index < wordQuestions.size())
	{
		wordQuestion wq = wordQuestions.at(index);
		for (int i = 0; i < wq.tiles.size(); i++)
		{
			swapXPos(wq.tiles.at(i), wq.tiles.at(qrand() % wq.tiles.size()));
			wq.tiles.at(i)->setPos(wq.tiles.at(i)->x(), wq.chip->y() + verticalVariation* (double)qrand()/RAND_MAX);
		}


	}
}

void UnscrambleGameTable::swapXPos(Tile* a, Tile* b)
{
	// swaps the x positions of two Tiles
	qreal t = a->x();
	a->setPos(b->x(), a->y());
	b->setPos(t, b->y());
}

double UnscrambleGameTable::getScaleFactor(int wordLength)
{
	return (-0.0475 * (double)wordLength + 1.3325);
	// derived from a linear relation: scale of 1 at length 7, scale of 0.62 at length 15
}

void UnscrambleGameTable::getBasePosition(int index, double scale, double& x, double& y)
{
	int tileWidth = 19;

	if (index >= 0 && index < 10)
	{
		//item->setPos(150 + i*(19.0 * scale), 190 + 26*index + verticalVariation* (double)qrand()/RAND_MAX);
		x = 150 - tileWidth * scale;
		y = 190 +26 * index;
	}
	else if (index >= 10 && index < 23)
	{
		//item->setPos(330 + i*(19.0 * scale), 150 + 26*(index-10)+ verticalVariation* (double)qrand()/RAND_MAX);
		x = 330 - tileWidth * scale;
		y = 150 + 26*(index-10);
	}
	else if (index >= 23 && index < 35)
	{
		//item->setPos(510 + i*(19.0 * scale), 160 + 26*(index-23)+ verticalVariation* (double)qrand()/RAND_MAX);
		x = 510 - tileWidth * scale;
		y = 160 + 26*(index-23);
	}
	else if (index >=35)
	{
		//item->setPos(690 + i*(19.0*scale),170 + 26*(index-35)+ verticalVariation* (double)qrand()/RAND_MAX);
		x = 690 - tileWidth * scale;
		y = 170+26*(index-35);
	}
}

void UnscrambleGameTable::addNewWord(int index, QString alphagram, QStringList solutions, quint8 numNotSolved)
{
	QTime t;
	t.start();
	wordQuestion thisWord(alphagram, solutions, numNotSolved);
	
	if (numNotSolved > 0)
	{

		double scale = getScaleFactor(alphagram.length());
		
		double chipX, chipY;
		getBasePosition(index, scale, chipX, chipY);	// gets the chip position for index
		for (int i = 0; i < alphagram.length(); i++)
		{
			//QGraphicsPixmapItem* item = gfxScene.addPixmap(tilesList.at(alphagram.at(i).toAscii() - 'A'));
			//thisRoundItems << item;
			Tile* item = tiles.at(index * 15 + i);
			item->resetTransform();
			item->scale(scale, scale);
			item->setPos(chipX + (i + 1) * (19.0 * scale),
				chipY + verticalVariation* (double)qrand()/RAND_MAX);
			item->setData(0, index);	// set the item data to index to keep track of which word this tile belongs to

			thisWord.tiles.append(item);
			item->setTileLetter(alphagram.at(i));
			item->show();
		}
		
		//if (numNotSolved >9) numNotSolved = 9;

		Chip *item = chips.at(index);

		item->setChipNumber(numNotSolved);
		item->setPos(chipX, chipY);
		item->resetTransform();
		item->scale(scale, scale);
		item->show();
		thisWord.chip = item;
	}

	wordQuestions << thisWord;
	qDebug() << "Added new word in " << t.elapsed();
}

void UnscrambleGameTable::answeredCorrectly(int index, QString username, QString answer)
{
	QString alphagram = wordQuestions.at(index).alphagram;
	double scale = getScaleFactor(alphagram.length());


	wordQuestions[index].numNotYetSolved--;
	int numSolutions = wordQuestions.at(index).numNotYetSolved;

	if (numSolutions > 0)
	{
		Chip* chip = wordQuestions.at(index).chip;	
		chip->resetTransform();
		chip->scale(scale, scale);
		chip->setChipNumber(numSolutions);
		chip->update();
	}
	else
	{
		wordQuestions.at(index).chip->hide();
		foreach (Tile* tile, wordQuestions.at(index).tiles)
			tile->hide();
	}

	rightAnswers.insert(answer);

	// maybe later color code by username
}

void UnscrambleGameTable::clearAllWordTiles()
{ 
	
	wordQuestions.clear();
	foreach (Tile* tile, tiles)
		tile->hide();

	foreach (Chip* chip, chips)
		chip->hide();

	tableUi.graphicsView->centerOn(tableItem);

}

void UnscrambleGameTable::setupForGameStart()
{
	tableUi.graphicsView->centerOn(tableItem);
	for (int i = 0; i < numPlayers; i++)
	{
		playerUis.at(i).listWidgetAnswers->clear();
		playerUis.at(i).labelAddInfo->clear();
	}
	rightAnswers.clear();
	clearReadyIndicators();
}

void UnscrambleGameTable::saveUserPreferences()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                        "CesarWare", "Aerolith"); 
	settings.beginGroup("Unscramble Table Preferences");

	settings.setValue("TileColorOption", uiPreferences.comboBoxTileColor->currentIndex());
	settings.setValue("FontColorOption", uiPreferences.comboBoxFontColor->currentIndex());
	settings.setValue("TileBorders", uiPreferences.checkBoxTileBorders->isChecked());
	settings.setValue("RandomVerticalPositions", uiPreferences.checkBoxRandomVerticalPositions->isChecked());
	settings.setValue("TableStyle", uiPreferences.comboBoxTableStyle->currentIndex());
	settings.setValue("BackgroundStyle", uiPreferences.comboBoxBackground->currentIndex());

	settings.endGroup();
	preferencesWidget->hide();
}

void UnscrambleGameTable::loadUserPreferences()
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                        "CesarWare", "Aerolith"); 
	settings.beginGroup("Unscramble Table Preferences");

	uiPreferences.comboBoxTileColor->setCurrentIndex(settings.value("TileColorOption", 0).toInt());
	uiPreferences.comboBoxFontColor->setCurrentIndex(settings.value("FontColorOption", 0).toInt());
	uiPreferences.checkBoxTileBorders->setChecked(settings.value("TileBorders", true).toBool());
	uiPreferences.checkBoxRandomVerticalPositions->setChecked(settings.value("RandomVerticalPositions", true).toBool());
	uiPreferences.comboBoxTableStyle->setCurrentIndex(settings.value("TableStyle", 0).toInt());
	uiPreferences.comboBoxBackground->setCurrentIndex(settings.value("BackgroundStyle", 0).toInt());

	changeTileColors(uiPreferences.comboBoxTileColor->currentIndex());
	changeFontColors(uiPreferences.comboBoxFontColor->currentIndex());
	changeTableStyle(uiPreferences.comboBoxTableStyle->currentIndex());
	changeTileBorderStyle(uiPreferences.checkBoxTileBorders->isChecked());
	changeVerticalVariation(uiPreferences.checkBoxRandomVerticalPositions->isChecked());
	changeBackground(uiPreferences.comboBoxBackground->currentIndex());
	settings.endGroup();

}
