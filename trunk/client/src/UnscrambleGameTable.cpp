#include "UnscrambleGameTable.h"

UnscrambleGameTable::UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	tableUi.setupUi(this);


	connect(tableUi.pushButtonSolutions, SIGNAL(clicked()), this, SIGNAL(shouldShowSolutions()));	
	
	connect(tableUi.pushButtonGiveUp, SIGNAL(clicked()), this, SIGNAL(giveUp()));
	connect(tableUi.pushButtonStart, SIGNAL(clicked()), this, SIGNAL(sendStartRequest()));
	//connect(playerInfoWidget, SIGNAL(avatarChange(quint8)), this, SIGNAL(avatarChange(quint8)));
	connect(tableUi.lineEditSolution, SIGNAL(returnPressed()), this, SLOT(enteredGuess()));
	connect(tableUi.pushButtonExit, SIGNAL(clicked()), this, SIGNAL(exitThisTable()));

	connect(tableUi.listWidgetPeopleInRoom, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(sendPM(QListWidgetItem* )));

	tableUi.textEditChat->setTextInteractionFlags(Qt::TextSelectableByMouse);

	
	//connect(tableUi.pushButtonAlphagrams, SIGNAL(clicked()), wordsWidget, SLOT(alphagrammizeWords()));
	//connect(tableUi.pushButtonShuffle, SIGNAL(clicked()), wordsWidget, SLOT(shuffleWords()));

	connect(tableUi.lineEditChat, SIGNAL(returnPressed()), this, SLOT(enteredChat()));

	// add player widgets

	QWidget* tmpWidget = new QWidget(this);
	playerUis[0].setupUi(tmpWidget);
	tmpWidget->move(220, 510);

	tmpWidget = new QWidget(this);
	playerUis[1].setupUi(tmpWidget);
	tmpWidget->move(650, 510);

	tmpWidget = new QWidget(this);
	playerUis[2].setupUi(tmpWidget);
	tmpWidget->move(870, 250);

	tmpWidget = new QWidget(this);
	playerUis[3].setupUi(tmpWidget);
	tmpWidget->move(650, 10);

	tmpWidget = new QWidget(this);
	playerUis[4].setupUi(tmpWidget);
	tmpWidget->move(220, 10);

	tmpWidget = new QWidget(this);
	playerUis[5].setupUi(tmpWidget);
	tmpWidget->move(10, 250);


	

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


	gfxScene.setSceneRect(0, 0, 980, 700);
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
//	playerInfoWidget->clearAndHide();	// won't do this anymore - clear all individual widgets
//	wordsWidget->clearCells();			// instead get rid of all tiles on table
	tableUi.pushButtonExit->setText(QString("Exit table %1").arg(tableNum));
	tableUi.lineEditChat->clear();
	tableUi.textEditChat->clear();

}

void UnscrambleGameTable::leaveTable()
{
//	playerInfoWidget->leaveTable();		// clear the seats hash
	tableUi.listWidgetPeopleInRoom->clear();
}

void UnscrambleGameTable::addPlayers(QStringList plist)
{
	tableUi.listWidgetPeopleInRoom->addItems(plist);
	//playerInfoWidget->addPlayers(plist);
}

void UnscrambleGameTable::addPlayer(QString player, bool gameStarted)
{
//	playerInfoWidget->addPlayer(player, gameStarted);
	tableUi.listWidgetPeopleInRoom->addItem(player);
}

void UnscrambleGameTable::removePlayer(QString player, bool gameStarted)
{
//	playerInfoWidget->removePlayer(player, gameStarted);
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


void UnscrambleGameTable::setMyUsername(QString username)
{
	myUsername = username;
}

void UnscrambleGameTable::gotTimerValue(quint16 timerval)
{
	tableUi.lcdNumberTimer->display(timerval);
}

void UnscrambleGameTable::gotWordListInfo(QString info)
{
	tableUi.labelWordListInfo->setText(info);
}

void UnscrambleGameTable::setAvatar(QString, quint8)
{

}