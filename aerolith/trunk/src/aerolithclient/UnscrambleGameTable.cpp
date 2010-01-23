//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#include "UnscrambleGameTable.h"
#include "commonDefs.h"

UnscrambleGameTable::UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f, DatabaseHandler *dbHandler) :
        GameTable(parent, f, 8)
{
    currentWordLength = 0;
    this->dbHandler = dbHandler;
    //this->setStyle(new QWindowsStyle);
    tableUi.setupUi(this);
    savingAllowed = false;

    connect(tableUi.pushButtonGiveUp, SIGNAL(clicked()), this, SIGNAL(giveUp()));
    connect(tableUi.pushButtonStart, SIGNAL(clicked()), this, SIGNAL(sendStartRequest()));
    connect(tableUi.lineEditSolution, SIGNAL(returnPressed()), this, SLOT(enteredGuess()));
    connect(tableUi.pushButtonExit, SIGNAL(clicked()), this, SLOT(exitButtonPressed()));
    connect(tableUi.pushButtonSave, SIGNAL(clicked()), this, SLOT(saveGame()));
    connect(tableUi.listWidgetPeopleInRoom, SIGNAL(sendPM(QString)), this, SIGNAL(sendPM(QString)));
    connect(tableUi.listWidgetPeopleInRoom, SIGNAL(viewProfile(QString)), this, SIGNAL(viewProfile(QString)));
    connect(tableUi.pushButtonStand, SIGNAL(clicked()), this, SIGNAL(standUp()));
    connect(tableUi.comboBoxTablePrivacy, SIGNAL(currentIndexChanged(int)), this, SLOT(changeTablePrivacy(int)));
    connect(tableUi.pushButtonInvite, SIGNAL(clicked()), SIGNAL(showInviteDialog()));
    connect(tableUi.pushButtonBoot, SIGNAL(clicked()), SLOT(showBootDialog()));

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
    connect(uiPreferences.checkBoxTallTiles, SIGNAL(toggled(bool)), this, SLOT(changeTileAspectRatio(bool)));
    connect(uiPreferences.checkBoxWordBorders, SIGNAL(toggled(bool)), this, SLOT(drawWordBorders(bool)));


    connect(uiPreferences.groupBoxUseTiles, SIGNAL(clicked(bool)), this, SLOT(changeUseTiles(bool)));
    connect(uiPreferences.checkBoxUseFixedWidthFont, SIGNAL(toggled(bool)), this, SLOT(useFixedWidthFontForRectangles(bool)));
    connect(tableUi.pushButtonFontToggle, SIGNAL(clicked()), this, SLOT(pushedFontToggleButton()));
    //	connect(tableUi.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(setZoom(int)));
    tableUi.textEditChat->setTextInteractionFlags(Qt::TextSelectableByMouse);
    tableUi.textEditChat->document()->setMaximumBlockCount(500);
    tableUi.textEditGuesses->document()->setMaximumBlockCount(500);

    connect(tableUi.pushButtonAlphagrams, SIGNAL(clicked()), this, SLOT(alphagrammizeWords()));
    connect(tableUi.pushButtonShuffle, SIGNAL(clicked()), this, SLOT(shuffleWords()));

    connect(tableUi.lineEditChat, SIGNAL(returnPressed()), this, SLOT(enteredChat()));

    // load tilesList and chipsList


    //gfxScene.setSceneRect(0, 0, 980, 720);
    tableUi.graphicsView->viewport()->setFocusPolicy(Qt::NoFocus);
    tableUi.graphicsView->setScene(&gfxScene);
    tableUi.graphicsView->setSceneRect(tableUi.graphicsView->contentsRect());
    tableUi.graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    tableUi.graphicsView->setBackgroundBrush(QImage(":/images/canvas.png"));
    tableUi.graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    tableUi.graphicsView->setCacheMode(QGraphicsView::CacheBackground);
    //tableUi.graphicsView->setAcceptedMouseButtons(0);
    tableItem = gfxScene.addPixmap(QPixmap(":/images/table.png"));
    tableItem->setZValue(-1);
    tableItem->setAcceptedMouseButtons(0);

    setWindowIcon(QIcon(":/images/aerolith.png"));

    move(0, 0);



    //connect(

    solutionsDialog = new QDialog(this);
    uiSolutions.setupUi(solutionsDialog);
    uiSolutions.solutionsTableWidget->verticalHeader()->hide();

    solutionsDialog->setAttribute(Qt::WA_QuitOnClose, false);
    preferencesWidget->setAttribute(Qt::WA_QuitOnClose, false);

    connect(tableUi.pushButtonSolutions, SIGNAL(clicked()), solutionsDialog, SLOT(show()));
    qDebug() << "Style sheet: " << uiSolutions.solutionsTableWidget->styleSheet();
    uiSolutions.solutionsTableWidget->setItemDelegate(new QItemDelegate(uiSolutions.solutionsTableWidget));
    // anyone wanna debug why i need to add the previous line?
    // the missed table items don't show up red if i don't add the line! (on qt 4.4.0+)

    // generate gfx items

    for (int i = 0; i < 50; i++)
    {
        WordRectangle *w = new WordRectangle;
        wordRectangles << w;
        gfxScene.addItem(w);
        w->hide();
        w->setPos(-100, -100);
        w->setZValue(0);
        connect (w, SIGNAL(mousePressed()), this, SLOT(rectangleWasClicked()));
    }

    // 50 letters * 15 =
    for (int i = 0; i < 750; i++)
    {
        Tile *t = new Tile;
        tiles << t;
        gfxScene.addItem(t);
        t->hide();
        t->setZValue(1);
        connect(t, SIGNAL(mousePressed()), this, SLOT(tileWasClicked()));
    }

    for (int i = 0; i < 50; i++)
    {
        Chip *c = new Chip;
        chips << c;
        gfxScene.addItem(c);
        c->hide();
        c->setZValue(2);
    }

    for (int i = 0; i < maxPlayers; i++)
    {
        Chip *c = new Chip;
        gfxScene.addItem(c);
        c->setChipString("");
        c->hide();
        c->setZValue(3);
        QBrush readyChipBrush = QBrush(Qt::black);
        QPen foregroundPen = QPen(Qt::white);
        QPen edgePen = QPen(QColor(1, 1, 1).lighter(150), 0);
        c->setChipProperties(readyChipBrush, foregroundPen, edgePen);
        readyChips << c;
    }



    readyChips.at(0)->setPos(200, 390);
    readyChips.at(1)->setPos(370, 390);
    readyChips.at(2)->setPos(540, 390);
    readyChips.at(3)->setPos(710, 390);
    readyChips.at(4)->setPos(800, 330);
    readyChips.at(5)->setPos(800, 190);
    readyChips.at(6)->setPos(800, 50);

    readyChips.at(7)->setPos(785, 385);

    for (int i = 0; i < maxPlayers; i++)
        playerWidgets.at(i)->raise();

    playerWidgets.at(0)->move(150, 480);
    playerWidgets.at(1)->move(320, 480);
    playerWidgets.at(2)->move(490, 480);
    playerWidgets.at(3)->move(650, 480);
    playerWidgets.at(4)->move(880, 320);
    playerWidgets.at(5)->move(880, 180);
    playerWidgets.at(6)->move(880, 40);

    playerWidgets.at(7)->move(820, 450);




    verticalVariation = 2.0;


    loadUserPreferences();


#ifdef Q_WS_MAC

#endif

}

UnscrambleGameTable::~UnscrambleGameTable()
{
    qDebug() << "UnscrambleGameTable destructor";
    while (!chips.isEmpty())
        delete chips.takeFirst();
    while (!tiles.isEmpty())
        delete tiles.takeFirst();

}

void UnscrambleGameTable::setPrivacy(bool p)
{
    isPrivate = p;
    if (p)
        tableUi.comboBoxTablePrivacy->setCurrentIndex(1);
    else
        tableUi.comboBoxTablePrivacy->setCurrentIndex(0);
}

void UnscrambleGameTable::setCurrentSug(SavedUnscrambleGame sug)
{
    currentSug = sug;
    savedGameModified = false;
}

void UnscrambleGameTable::setSavingAllowed(bool a)
{
    savingAllowed = a;
    tableUi.pushButtonSave->setEnabled(a);
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
    case 6: // gray
        linearGrad.setColorAt(0, this->palette().color(QPalette::Window));
        linearGrad.setColorAt(1, this->palette().color(QPalette::Window));
        tileBrush = QBrush(linearGrad);
        break;
    }
    QPen edgePen;
    if (uiPreferences.checkBoxTileBorders->isChecked()) edgePen = QPen(Qt::black, 0.4);
    else edgePen = QPen(tileBrush, 0.4);

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
    else if (index == 3)
    {
        tableItem->setPixmap(QPixmap(":/images/table3.png"));
        tableItem->show();
    }
}

void UnscrambleGameTable::changeTileBorderStyle(bool borderOn)
{
    QBrush tileBrush = tiles.at(0)->getTileBrush();
    QPen edgePen;
    if (borderOn)
        edgePen = QPen(Qt::black, 0.4);
    else
        edgePen = QPen(tileBrush, 0.4);

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

void UnscrambleGameTable::changeTileAspectRatio(bool on)
{
    if (on) heightScale = 1.25;
    else heightScale = 1.0;
    foreach (wordQuestion wq, wordQuestions)
    {
        if (wq.numNotYetSolved > 0)
        {
            foreach(Tile* tile, wq.tiles)
            {
                tile->setWidth(tile->getWidth(), heightScale);
            }
        }
    }
    gfxScene.update();
}

void UnscrambleGameTable::drawWordBorders(bool on)
{
    foreach (WordRectangle* wr, wordRectangles)
    {
        if (on && uiPreferences.groupBoxUseTiles->isChecked())
        {
            wr->hideText();
            wr->setTransparentBG();
            wr->show();
        }
        else
        {
            wr->showText();
            wr->setOpaqueBG();
            wr->hide();
        }
    }

    gfxScene.update();

}

void UnscrambleGameTable::changeUseTiles(bool on)
{
    if (on)
    {
        uiPreferences.groupBoxUseRectangles->setEnabled(false);
        foreach (wordQuestion wq, wordQuestions)
        {
            if (wq.numNotYetSolved > 0)
            {
                foreach(Tile* tile, wq.tiles)
                {
                    tile->show();
                }
            }
        }

        foreach (WordRectangle* wr, wordRectangles)
        {
            wr->hide();

        }
        tableUi.pushButtonFontToggle->hide();
        drawWordBorders(uiPreferences.checkBoxWordBorders->isChecked());
    }
    else
    {
        uiPreferences.groupBoxUseRectangles->setEnabled(true);
        foreach (Tile* t, tiles)
            t->hide();
        foreach (WordRectangle* wr, wordRectangles)
        {
            wr->show();
            wr->setOpaqueBG();
            wr->showText();
        }
        tableUi.pushButtonFontToggle->show();
    }

    gfxScene.update();

}

void UnscrambleGameTable::useFixedWidthFontForRectangles(bool on)
{
    foreach (WordRectangle* wr, wordRectangles)
        wr->setFixedWidthFont(on);


    gfxScene.update();
    
}

void UnscrambleGameTable::pushedFontToggleButton()
{
    uiPreferences.checkBoxUseFixedWidthFont->animateClick();

}

void UnscrambleGameTable::changeBackground(int index)
{
    QString colorHtml = "black";
    switch (index)
    {
    case 0:
        tableUi.graphicsView->setBackgroundBrush(QImage(":/images/canvas.png"));
        colorHtml = "black";
        break;

    case 1:
        tableUi.graphicsView->setBackgroundBrush(QBrush(this->palette().color(QPalette::Window)));
        colorHtml = "black";
        break;

    case 2:
        tableUi.graphicsView->setBackgroundBrush(QBrush(Qt::white));
        colorHtml = "black";
        break;

    case 3:
        tableUi.graphicsView->setBackgroundBrush(QImage(":/images/lava.png"));
        colorHtml = "white";
        break;

    case 4:
        tableUi.graphicsView->setBackgroundBrush(QImage(":/images/stars.png"));
        colorHtml = "white";
        break;
    }


    QString sheet = QString("QLabel { font: bold; color: %1; }").arg(colorHtml);
    tableUi.labelWordListInfo->setStyleSheet(sheet);
    tableUi.labelGuess->setStyleSheet(sheet);
    tableUi.labelYourGuesses->setStyleSheet(sheet);

    for (int i = 0; i < maxPlayers; i++)
    {
        playerUis.at(i).labelUsername->setStyleSheet(sheet);
        playerUis.at(i).labelAddInfo->setStyleSheet(sheet);
    }

}

void UnscrambleGameTable::setZoom(int zoom)
{
    QMatrix matrix;
    matrix.scale((double)zoom/50.0, (double)zoom/50.0);

    tableUi.graphicsView->setMatrix(matrix);

}

void UnscrambleGameTable::setReadyIndicator(quint8 seat)
{
    readyChips.at(seat)->show();
}

void UnscrambleGameTable::clearReadyIndicators()
{
    for (int i = 0; i < maxPlayers; i++)
        readyChips.at(i)->hide();
}



void UnscrambleGameTable::enteredChat()
{
    emit chatTable(tableUi.lineEditChat->text());
    tableUi.lineEditChat->clear();

}


void UnscrambleGameTable::enteredGuess()
{

    QString guess = tableUi.lineEditSolution->text().simplified().toUpper();
    if (guess == "") return;


    if (answerHash.contains(guess))
    {
        const wordQuestion* wq = &(wordQuestions.at(answerHash.value(guess)));
        int pos = wq->solutions.indexOf(guess);         // TODO should find something more efficient than a linear search
        emit correctAnswerSubmitted(wq->space, pos);
        qDebug() << "enteredguess" << wq->space << pos;
    }



    tableUi.textEditGuesses->append(guess);

    tableUi.lineEditSolution->clear();

}

void UnscrambleGameTable::closeEvent(QCloseEvent* event)
{
    event->ignore();
    tableUi.pushButtonExit->animateClick();
}

void UnscrambleGameTable::resetTable(quint16 tableNum, QString wordListName, QString myUsername)
{
    setWindowTitle(QString("Table %1 - Word List: %2 - Logged in as %3 - Table Host: %4").arg(tableNum).arg(wordListName).arg(myUsername).arg(""));
    tableUi.labelWordListInfo->clear();
    tableUi.lcdNumberTimer->display(0);
    clearPlayerWidgets();
    clearAllWordTiles();

    tableUi.pushButtonExit->setText(QString("Exit table %1").arg(tableNum));
    tableUi.lineEditChat->clear();
    tableUi.textEditChat->clear();
    tableUi.graphicsView->centerOn(tableItem);

    peopleInTable.clear();
    tableUi.listWidgetPeopleInRoom->clear();

    tableUi.lineEditSolution->setFocus();
    tableUi.textEditGuesses->clear();

    answerHash.clear();
    savedGameModified = false;
}

void UnscrambleGameTable::leaveTable()
{
    playerLeaveTable();	// clear the seats hash
    tableUi.listWidgetPeopleInRoom->clear();
}

void UnscrambleGameTable::addPlayers(QStringList plist)
{
    tableUi.listWidgetPeopleInRoom->addItems(plist);
    peopleInTable.append(plist);
    //    addPlayersToWidgets(plist);
}

void UnscrambleGameTable::addPlayer(QString player, bool gameStarted)
{
    //   addPlayerToWidgets(player, gameStarted);
    peopleInTable.append(player);
    tableUi.listWidgetPeopleInRoom->addItem(player);
}

void UnscrambleGameTable::removePlayer(QString player, bool gameStarted)
{
    peopleInTable.removeAll(player);
    //    removePlayerFromWidgets(player, gameStarted);
    for (int i = 0; i < tableUi.listWidgetPeopleInRoom->count(); i++)
    {
        if (tableUi.listWidgetPeopleInRoom->item(i)->text() == player)
        {
            QListWidgetItem *it = tableUi.listWidgetPeopleInRoom->takeItem(i);
            delete it;
        }
    }
}


void UnscrambleGameTable::gotChat(QString chat)
{
    tableUi.textEditChat->append(chat);
}

void UnscrambleGameTable::listRequest()
{
    qDebug() << "Received a list request.";
    QVector <quint32> indexList;

    if (randomizedQuizList.size() > 0)
    {
        indexList = randomizedQuizList.mid(0, 50);  // take the left-most 50
        randomizedQuizList = randomizedQuizList.mid(50);    // and remove them from the list


    }
    else if (randomizedMissedList.size() > 0)
    {
        indexList = randomizedMissedList.mid(0, 50);  // take the left-most 50
        randomizedMissedList = randomizedMissedList.mid(50);    // and remove them from the list

    }
    else
    {
        QMessageBox::critical(this, "!", "Shouldn't have a request!");
        return;
    }


    emit uploadList(indexList);
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
    QTime time;
    time.start();

    if (wordDb.isOpen())
    {

        QSqlQuery transactionQuery(wordDb);
        QSqlQuery alphagramQuery(wordDb);
        transactionQuery.exec("BEGIN TRANSACTION");
        alphagramQuery.prepare("select words, probability from alphagrams "
                               "where alphagram = ?");
        for (int i = 0; i < wordQuestions.size(); i++)
        {

            QStringList theseSols = wordQuestions.at(i).solutions;
            QString alphagram = wordQuestions.at(i).alphagram;
            if (alphagram != "") // if alphagram exists.
            {

                QTableWidgetItem *tableAlphagramItem = new QTableWidgetItem(alphagram);
                tableAlphagramItem->setTextAlignment(Qt::AlignCenter);
                int alphagramRow = uiSolutions.solutionsTableWidget->rowCount();
                alphagramQuery.bindValue(0, alphagram);
                alphagramQuery.exec();

                QString words;
                int probability;
                bool wrongAnswer = false;   // got this alphagram wrong?
                while (alphagramQuery.next())   // should only be one result.
                {
                    probability = alphagramQuery.value(1).toInt() & 0xFFFFFF;
                    words = alphagramQuery.value(0).toString();

                    QSqlQuery wordQuery(wordDb);
                    wordQuery.prepare("select definition, lexiconstrings, front_hooks, back_hooks from words where word = ?");

                    for (int j = 0; j < theseSols.size(); j++)
                    {

                        wordQuery.bindValue(0, theseSols.at(j));
                        wordQuery.exec();

                        while (wordQuery.next())
                        {

                            numTotalSols++;

                            uiSolutions.solutionsTableWidget->insertRow(numTotalSols-1);


                            uiSolutions.solutionsTableWidget->setItem(numTotalSols-1, 4,
                                                                      new QTableWidgetItem(wordQuery.value(3).toString()));
                            uiSolutions.solutionsTableWidget->setItem(numTotalSols-1, 2,
                                                                      new QTableWidgetItem(wordQuery.value(2).toString()));
                            uiSolutions.solutionsTableWidget->setItem(numTotalSols-1, 5,
                                                                      new QTableWidgetItem(wordQuery.value(0).toString()));

                            QTableWidgetItem* wordItem = new QTableWidgetItem(theseSols.at(j) +
                                                                              wordQuery.value(1).toString());
                            if (!rightAnswers.contains(theseSols.at(j)))
                            {
                                numWrong++;
                                wordItem->setForeground(missedColorBrush);
                                QFont wordItemFont = wordItem->font();
                                wordItemFont.setBold(true);
                                wordItem->setFont(wordItemFont);
                                wrongAnswer = true;

                            }

                            wordItem->setTextAlignment(Qt::AlignCenter);
                            uiSolutions.solutionsTableWidget->setItem(numTotalSols-1, 3, wordItem); // word
                        }
                    }
                }


                uiSolutions.solutionsTableWidget->setItem(alphagramRow, 1, tableAlphagramItem);

                if (wrongAnswer)
                {
                    tableAlphagramItem->setForeground(missedColorBrush);
                    QFont alphFont = tableAlphagramItem->font();
                    alphFont.setBold(true);

                    tableAlphagramItem->setFont(alphFont);

                    if (savingAllowed)
                    {
                        if (currentSug.brandNew)
                        {
                            currentSug.brandNew = false;
                            currentSug.curQuizList = currentSug.origIndices;
                        }
                        Q_ASSERT(currentSug.curQuizList.contains(wordQuestions.at(i).probIndex));
                        currentSug.curQuizList.remove(wordQuestions.at(i).probIndex);
                        currentSug.curMissedList.insert(wordQuestions.at(i).probIndex);
                        if (!currentSug.seenWholeList)
                        {
                            // also put it here if we haven't seen the whole list.
                            currentSug.firstMissed.insert(wordQuestions.at(i).probIndex);
                        }
                        savedGameModified = true;
                    }
                }


                uiSolutions.solutionsTableWidget->setItem(alphagramRow, 0,
                                                          new QTableWidgetItem(QString::number(probability)));

            }
        }
        transactionQuery.exec("END TRANSACTION");
        uiSolutions.solutionsTableWidget->resizeColumnsToContents();
        double percCorrect;
        if (numTotalSols == 0) percCorrect = 0.0;
        else
            percCorrect = (100.0 * (double)(numTotalSols - numWrong))/(double)(numTotalSols);
        uiSolutions.solsLabel->setText(QString("Number of total solutions: %1   Percentage correct: %2 (%3 of %4)").arg(numTotalSols).arg(percCorrect).arg(numTotalSols-numWrong).arg(numTotalSols));
    }
    else
        qDebug() << "Database is not open.";

    qDebug() << "Time to populate sols" << time.elapsed();
}

void UnscrambleGameTable::alphagrammizeWords()
{
    if (uiPreferences.groupBoxUseTiles->isChecked())
    {
        foreach (wordQuestion wq, wordQuestions)
        {
            // server always sends alphagram, so arrange tiles in order
            int tileWidth = getTileWidth(wq.alphagram.length());
            // chipX, chipY is 19 to the left of the tile
            for (int i = 0; i < wq.tiles.size(); i++)
            {
                wq.tiles.at(i)->setPos(wq.chip->x() + tileWidth*(i+1), wq.chip->y() + verticalVariation* (double)qrand()/RAND_MAX);
            }

        }
    }
    else
    {
        foreach (WordRectangle *wr, wordRectangles)
            wr->alphagrammizeText();

        gfxScene.update();
    }
}

void UnscrambleGameTable::shuffleWords()
{
    if (uiPreferences.groupBoxUseTiles->isChecked())
    {
        foreach (wordQuestion wq, wordQuestions)
        {
            for (int i = 0; i < wq.tiles.size(); i++)
            {
                swapXPos(wq.tiles.at(i), wq.tiles.at(qrand() % wq.tiles.size()));
                wq.tiles.at(i)->setPos(wq.tiles.at(i)->x(), wq.chip->y() + verticalVariation* (double)qrand()/RAND_MAX);
            }
        }
    }
    else
    {
        foreach (WordRectangle *wr, wordRectangles)
            wr->shuffleText();
        gfxScene.update();
    }
}

void UnscrambleGameTable::rectangleWasClicked()
{
    tableUi.lineEditSolution->setFocus();
    WordRectangle *wr = static_cast<WordRectangle*> (sender());
    wr->shuffleText();
    gfxScene.update();

}

void UnscrambleGameTable::tileWasClicked()
{
    tableUi.lineEditSolution->setFocus();
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

int UnscrambleGameTable::getTileWidth(int wordLength)
{
    int baseWidth = 18;
    if (wordLength > 10)
        return (int)(baseWidth*(-0.0475 * (double)wordLength + 1.3925)) + 1;
    else return baseWidth;

}

void UnscrambleGameTable::getBasePosition(int index, double& x, double& y, int tileWidth)
{

    if (index >= 0 && index < 12)
    {
        //item->setPos(150 + i*(19.0 * scale), 190 + 26*index + verticalVariation* (double)qrand()/RAND_MAX);
        x = 50 - tileWidth;
        y = 60 +26 * index;
    }
    else if (index >= 12 && index < 25)
    {
        //item->setPos(330 + i*(19.0 * scale), 150 + 26*(index-10)+ verticalVariation* (double)qrand()/RAND_MAX);
        x = 255 - tileWidth;
        y = 46 + 26*(index-12);
    }
    else if (index >= 25 && index < 38)
    {
        //item->setPos(510 + i*(19.0 * scale), 160 + 26*(index-23)+ verticalVariation* (double)qrand()/RAND_MAX);
        x = 460 - tileWidth;
        y = 56 + 26*(index-25);
    }
    else if (index >=38)
    {
        //item->setPos(690 + i*(19.0*scale),170 + 26*(index-35)+ verticalVariation* (double)qrand()/RAND_MAX);
        x = 665 - tileWidth;
        y = 66+26*(index-38);
    }
}

void UnscrambleGameTable::addNewWord(int index, quint32 probIndex,
                                     quint8 numNotYetSolved, QSet <quint8> notYetSolved)
{

    QSqlQuery query(wordDb);
    query.prepare("select words, alphagram from alphagrams "
                  "where probability = ?");

    query.bindValue(0, probIndex);
    query.exec();

    QString alphagram;
    QStringList solutions;

    while (query.next())   // should only be one result.
    {
        alphagram = query.value(1).toString();
        solutions = query.value(0).toString().split(" ");
    }

    wordQuestion thisWord(alphagram, solutions, numNotYetSolved);
    bool shouldShowTiles = uiPreferences.groupBoxUseTiles->isChecked();
    if (numNotYetSolved > 0)
    {

        int tileWidth = getTileWidth(alphagram.length());
        double chipX, chipY;
        getBasePosition(index,  chipX, chipY, tileWidth);	// gets the chip position for index
        wordRectangles.at(index)->setPos(chipX-5, chipY-5);
        wordRectangles.at(index)->setText(alphagram);
        if (!shouldShowTiles)
        {
            wordRectangles.at(index)->show();
            wordRectangles.at(index)->update();
        }
        for (int i = 0; i < alphagram.length(); i++)
        {

            Tile* item = tiles.at(index * 15 + i);
            item->resetTransform();
            item->setWidth(tileWidth, heightScale);
            item->setPos(chipX + (i + 1) * (tileWidth),
                         chipY + verticalVariation* (double)qrand()/RAND_MAX);
            item->setData(0, index);	// set the item data to index to keep track of which word this tile belongs to

            thisWord.tiles.append(item);
            item->setTileLetter(alphagram.at(i));
            if (shouldShowTiles) item->show();
        }

        Chip *item = chips.at(index);

        item->setChipNumber(numNotYetSolved);
        item->setPos(chipX, chipY);
        item->resetTransform();
        double scale = (double)tileWidth / 19.0;
        item->scale(scale, scale);
        item->show();
        thisWord.chip = item;
    }
    thisWord.space = index;
    qDebug() << "Added thisword" << index;
    thisWord.notYetSolved = notYetSolved;
    thisWord.probIndex = probIndex;
    wordQuestions << thisWord;
    foreach (QString answer, solutions)
        answerHash.insert(answer, index);

}


void UnscrambleGameTable::setIndices(QSet <quint32> qindices, QSet <quint32> mindices)
{
    //
    //    QVector <quint32> randomizedQuizSet;
    //    QVector <quint32> randomizedMissedSet;
    //    int rqindex, rmindex;
    randomizedQuizList.clear();
    randomizedMissedList.clear();

    QList <quint32> qindicesList = qindices.toList();
    QList <quint32> mindicesList = mindices.toList();

    QVector <quint32> indexVector;
    getUniqueRandomNumbers(indexVector, 0, qindices.size() - 1, qindices.size());

    for (quint32 i = 0; i < qindices.size(); i++)
        randomizedQuizList << qindicesList.at(indexVector.at(i));


    getUniqueRandomNumbers(indexVector, 0, mindices.size() - 1, mindices.size());

    for (quint32 i = 0; i < mindices.size(); i++)
        randomizedMissedList << mindicesList.at(indexVector.at(i));

    qDebug() << "Setindices";
    qDebug() << "Quiz" << randomizedQuizList;
    qDebug() << "Missed" << randomizedMissedList;
}

void UnscrambleGameTable::mainQuizDone()
{
    if (savingAllowed)
    {
        /* we are finally done with the main quiz */
        if (!currentSug.seenWholeList)
        {
            currentSug.seenWholeList = true;
        }

        currentSug.curQuizList = currentSug.curMissedList;
        currentSug.curMissedList.clear();
        savedGameModified = true;
    }
}

void UnscrambleGameTable::fullQuizDone()
{
    // we are done with the entire quiz
    if (savingAllowed)
    {
        Q_ASSERT(currentSug.curMissedList.isEmpty());
        Q_ASSERT(currentSug.curQuizList.isEmpty());

    }
}


void UnscrambleGameTable::answeredCorrectly(quint8 seatNumber, quint8 space, quint8 specificAnswer)
{
    QString alphagram = wordQuestions.at(space).alphagram;
    int tileWidth = getTileWidth(alphagram.length());
    double scale = (double)tileWidth/19.0;
    wordQuestions[space].numNotYetSolved--;
    int numSolutions = wordQuestions.at(space).numNotYetSolved;

    qDebug() << "ac" << space << specificAnswer << alphagram << numSolutions;

    if (numSolutions > 0)
    {
        Chip* chip = wordQuestions.at(space).chip;
        chip->resetTransform();
        chip->scale(scale, scale);
        chip->setChipNumber(numSolutions);

    }
    else
    {
        wordQuestions.at(space).chip->hide();
        foreach (Tile* tile, wordQuestions.at(space).tiles)
            tile->hide();

        wordRectangles.at(space)->setText("");

        if (savingAllowed)
        {
            if (currentSug.brandNew)
            {
                currentSug.brandNew = false;
                currentSug.curQuizList = currentSug.origIndices;
            }
          //  qDebug() << "Answeredcorrectly" << currentSug.curQuizList << currentSug.curMissedList;
            Q_ASSERT(currentSug.curQuizList.contains(wordQuestions.at(space).probIndex));
            currentSug.curQuizList.remove(wordQuestions.at(space).probIndex);
            savedGameModified = true;
        }
    }

    QString answer = wordQuestions[space].solutions.at(specificAnswer);
    wordQuestions[space].notYetSolved.remove(specificAnswer);

    rightAnswers.insert(answer);
    gfxScene.update();
    // maybe later color code by username

    addToPlayerList(seatNumber, answer);

}

void UnscrambleGameTable::clearAllWordTiles()
{

    wordQuestions.clear();
    foreach (Tile* tile, tiles)
        tile->hide();

    foreach (Chip* chip, chips)
        chip->hide();

    foreach (WordRectangle *wr, wordRectangles)
        wr->setText("");
    tableUi.graphicsView->centerOn(tableItem);
    gfxScene.update();
}

void UnscrambleGameTable::setupForGameStart()
{
    tableUi.graphicsView->centerOn(tableItem);
    for (int i = 0; i < maxPlayers; i++)
    {
        playerUis.at(i).listWidgetAnswers->clear();
        playerUis.at(i).labelAddInfo->clear();
    }
    rightAnswers.clear();
    answerHash.clear();
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
    settings.setValue("TallTiles", uiPreferences.checkBoxTallTiles->isChecked());
    settings.setValue("UseTiles", uiPreferences.groupBoxUseTiles->isChecked());
    settings.setValue("FixedWidthFont", uiPreferences.checkBoxUseFixedWidthFont->isChecked());
    settings.setValue("WordBorders", uiPreferences.checkBoxWordBorders->isChecked());
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
    uiPreferences.checkBoxTallTiles->setChecked(settings.value("TallTiles", false).toBool());
    uiPreferences.groupBoxUseTiles->setChecked(settings.value("UseTiles", true).toBool());
    uiPreferences.checkBoxUseFixedWidthFont->setChecked(settings.value("FixedWidthFont", false).toBool());
    uiPreferences.checkBoxWordBorders->setChecked(settings.value("WordBorders", false).toBool());

    changeTileColors(uiPreferences.comboBoxTileColor->currentIndex());
    changeFontColors(uiPreferences.comboBoxFontColor->currentIndex());
    changeTableStyle(uiPreferences.comboBoxTableStyle->currentIndex());
    changeTileBorderStyle(uiPreferences.checkBoxTileBorders->isChecked());
    changeVerticalVariation(uiPreferences.checkBoxRandomVerticalPositions->isChecked());
    changeBackground(uiPreferences.comboBoxBackground->currentIndex());
    changeTileAspectRatio(uiPreferences.checkBoxTallTiles->isChecked());
    changeUseTiles(uiPreferences.groupBoxUseTiles->isChecked());
    useFixedWidthFontForRectangles(uiPreferences.checkBoxUseFixedWidthFont->isChecked());
    drawWordBorders(uiPreferences.checkBoxWordBorders->isChecked());
    settings.endGroup();

}

void UnscrambleGameTable::saveGame()
{
    if (savingAllowed)
    {
        QByteArray ba = currentSug.toByteArray();
        emit saveCurrentGameBA(ba, lexiconName, unmodifiedListName);
        tableUi.textEditChat->append("<font color=green>Word list was saved!</font>");
        tableUi.textEditChat->append("<font color=green>To continue this word list in the future, make sure "
                                     "you select ""My Lists"" when creating a new table!</font>");
        savedGameModified = false;
    }
    else
    {
        tableUi.textEditChat->append("<font color=red>Saving is only allowed for single-player tables you've "
                                     "created.</font>");
    }
}

void UnscrambleGameTable::setHost(QString hostname)
{
    tableHost = hostname;
    tableUi.textEditChat->append("<font color=green>The host of this table is now " + hostname + ".</font>");

    QString windowTitle = this->windowTitle();
    int index = windowTitle.indexOf("Table Host:");

    setWindowTitle(windowTitle.left(index) + "Table Host: " + hostname);

    if (hostname == myUsername)
    {
        tableUi.comboBoxTablePrivacy->setEnabled(true);
        tableUi.pushButtonInvite->setEnabled(true);
        tableUi.pushButtonBoot->setEnabled(true);
    }
    else
    {
        tableUi.comboBoxTablePrivacy->setEnabled(false);
        tableUi.pushButtonInvite->setEnabled(false);
        tableUi.pushButtonBoot->setEnabled(false);
    }
}

void UnscrambleGameTable::changeTablePrivacy(int index)
{
    if (index == 0) // public
        emit setTablePrivate(false);
    else if (index == 1)
        emit setTablePrivate(true);
}

void UnscrambleGameTable::showBootDialog()
{
    QStringList playerList = peopleInTable;
    playerList.removeAll(myUsername);

    if (playerList.size() >= 1)
    {

        QString playerToBoot =     QInputDialog::getItem(this, "Boot", "Select player to boot", playerList, 0, false);
        if (playerToBoot != "")
            emit bootFromTable(playerToBoot);
    }
}

void UnscrambleGameTable::exitButtonPressed()
{
    if (savedGameModified && savingAllowed)
    {
        QMessageBox::StandardButton b = QMessageBox::warning(this, "Save game?", "Your list has been modified."
                                                             "Do you wish to save your game before quitting?",
                                                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                             QMessageBox::Cancel);
        if (b == QMessageBox::Yes)
        {
            saveGame();
        }
        else if (b == QMessageBox::No)
        {
            // do nothing, the signal at the bottom will take care of it
        }
        else if (b == QMessageBox::Cancel)
        {
            return;
        }

    }
    emit exitThisTable();

}
