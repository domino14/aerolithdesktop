//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Wordgrids.
//
//    Wordgrids is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Wordgrids is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Wordgrids.  If not, see <http://www.gnu.org/licenses/>.

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>


int scoresByLength[16] =
{ 0, 0, 4, 9, 16, 25, 49, 64, 81, 100, 121, 169, 196, 225, 256, 289 };
/*0  1  2  3  4    5   6   7   8    9   10   11   12   13   14   15*/

int letterDist[26] =
{ 9, 2, 2, 4, 12, 2, 3, 2, 9, 1, 1, 4, 2, 6, 8, 2, 1, 6, 4, 6, 4, 2, 2, 1, 2, 1 };
const int letterDistSum = 98;

QBrush brushUnsolved = QBrush(QColor(255, 200, 150));
QBrush brushSolved = QBrush(QColor(155, 100, 0));
QBrush brushBonusActive = QBrush(QColor(0, 255, 0));
QBrush brushBonusInactive = QBrush(QColor(0, 60, 0));


bool isPerfectSquare(int n)
{
    // Returns true if n is a perfect square
    int i = 0;
    while (i * i < n) i++;
    return i * i == n;
} // isPerfectSquare



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(&scene);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    ui->graphicsView->viewport()->setFocusPolicy(Qt::NoFocus);

    tileRect1 = NULL;
    tileRect2 = NULL;
    lastHoverTile = NULL;
 /*   firstCorner = new QGraphicsPixmapItem();
    secondCorner = new QGraphicsPixmapItem();

    QPixmap xhairspix = QPixmap(":/images/resources/crosshairs.png");
    crossHairsWidth = xhairspix.width();

    firstCorner->setPixmap(xhairspix);
    secondCorner->setPixmap(xhairspix);
    firstCorner->setZValue(1);
    secondCorner->setZValue(1);
    scene.addItem(firstCorner);
    scene.addItem(secondCorner);*/

    QPen linePen;
    linePen.setStyle(Qt::DashLine);
    linePen.setWidth(1);


    line1 = new QGraphicsLineItem();
    line2 = new QGraphicsLineItem();
    line3 = new QGraphicsLineItem();
    line4 = new QGraphicsLineItem();
    line1->setPen(linePen);
    line2->setPen(linePen);
    line3->setPen(linePen);
    line4->setPen(linePen);

    line1->setZValue(1);
    line2->setZValue(1);
    line3->setZValue(1);
    line4->setZValue(1);
    scene.addItem(line1);
    scene.addItem(line2);
    scene.addItem(line3);
    scene.addItem(line4);

    line1->setVisible(false);
    line2->setVisible(false);
    line3->setVisible(false);
    line4->setVisible(false);

/*
    firstCorner->setVisible(false);
    secondCorner->setVisible(false);
*/


    curTileWidth = 40;
    boardWidth = 10;
    boardHeight = 10;

    lastTimerSecs = 0;

    clickState = NO_TILES_CLICKED;

    qsrand(QDateTime::currentDateTime().toTime_t());
    gameTimer.setInterval(1000);
    curScore = 0;

    ui->textEdit->document()->setMaximumBlockCount(500);
    connect(&gameTimer, SIGNAL(timeout()), SLOT(secPassed()));

    gameGoing = false;
    numSolvedLetters = 0;
    lastGridSize = 10;

    connect(&scene, SIGNAL(sceneMouseClicked(double, double)), SLOT(sceneMouseClicked(double, double)));
    connect(&scene, SIGNAL(sceneMouseMoved(double, double)), SLOT(sceneMouseMoved(double, double)));
    connect(&scene, SIGNAL(keyPressed(int)), SLOT(keyPressed(int)));

    bonusTilesAllowed = false;

    wordStructure = new WordStructure(this);
    loadedWordStructure = false;
    connect(wordStructure, SIGNAL(finishedLoadingWordStructure()), this, SLOT(finishedLoadingWordStructure()));
    wordStructure->loadWordStructure();

    ui->textEdit->append("Please wait for word structure to load...");


    shouldLoadNextNewGame = false;


}


void MainWindow::sceneMouseMoved(double x, double y)
{
    ui->textEdit->append("Moved!");
    switch (clickState)
    {
    case NO_TILES_CLICKED:

        break;
    case FIRST_TILE_CLICKED:

        bool modifiedHighlightedSet = false;
        QGraphicsItem* item = scene.itemAt(x, y);
        Tile* tile = dynamic_cast<Tile *>(item);

        if (tile && tile->getTileLetter() != "")
        {

            if (lastHoverTile != tile)
            {
                modifiedHighlightedSet = true;
                lastHoverTile = tile;
            }

            int tempX = tile->tileCoordX;
            int tempY = tile->tileCoordY;

            if (modifiedHighlightedSet)
            {
                QString debugStr = QString("! %1 %2 %3 %4").arg(x1).arg(y1).arg(tempX).arg(tempY);
                ui->labelDebug->setText(debugStr);
                highlightedTiles.clear();
                for (int j = qMin(y1, tempY); j <= qMax(y1, tempY); j++)
                {
                    for (int i = qMin(x1, tempX); i <= qMax(x1, tempX); i++)
                    {


                        int index = j*boardWidth + i;

                        if (tiles.at(index) && tiles.at(index)->getTileLetter() != "")
                        {
                            //tiles.at(index)->setTileBrush(brushBonusActive);

                            highlightedTiles.insert(tiles.at(index));


                        }


                    }
                }
                foreach (Tile* tile, tiles)
                {
                    if (tile->getTileLetter() == "")
                        if (tile != tileRect1)
                            tile->setTileBrush(brushSolved);
                        else
                            tile->setTileBrush(brushBonusActive);
                    else
                    {
                        if (highlightedTiles.contains(tile))
                            tile->setTileBrush(brushBonusActive);

                        else
                            tile->setTileBrush(brushUnsolved);
                    }

                }
            }

        }




//        x2 = qRound(x/curTileWidth);
//        y2 = qRound(y/curTileWidth);
//        if (x2 < 0) x2 = 0;
//        if (y2 < 0) y2 = 0;
//        if (x2 > boardWidth) x2 = boardWidth;
//        if (y2 > boardHeight) y2 = boardHeight;
//
//        int lx1 = x1*curTileWidth;
//        int ly1 = y1*curTileWidth;
//        int lx2 = x2*curTileWidth;
//        int ly2 = y2*curTileWidth;

  /*      secondCorner->setVisible(true);
        secondCorner->setPos(lx2 - crossHairsWidth/2, ly2 - crossHairsWidth/2);
*/



//        line1->setLine(lx1, ly1, lx2, ly1);
//        line2->setLine(lx2, ly1, lx2, ly2);
//        line3->setLine(lx2, ly2, lx1, ly2);
//        line4->setLine(lx1, ly2, lx1, ly1);
//        line1->setVisible(true);
//        line2->setVisible(true);
//        line3->setVisible(true);
//        line4->setVisible(true);
        break;
    }
}

void MainWindow::sceneMouseClicked(double x, double y)
{
    if (!gameGoing)
    {
        clickState = NO_TILES_CLICKED;
        return;
    }
    switch (clickState)
    {

//    case BONUS_TILE_SELECTED:
//        {
//            QGraphicsItem* item = scene.itemAt(x, y);
//            Tile* tile = dynamic_cast<Tile *>(item);
//            if (tile)
//            {
//                if (tile->getAddlAttribute() == 1 && bonusTilesAllowed)
//                {
//                    if (curselBonusTile)
//                    {
//                        /* if a current bonus tile is already selected then it gets unselected first */
//                        curselBonusTile->setTileBrush(brushBonusActive);
//
//                        if (curselBonusTile == tile)
//                        {
//                            /* if we clicked on the same tile just reset this tile */
//                            curselBonusTile = NULL;
//                            cornerState = BOTH_CORNERS_OFF;
//                            scene.update();
//                            break;
//                        }
//                        else
//                        {
//                            curselBonusTile = tile;
//                            tile->setTileBrush(brushUnsolved);
//                            cornerState = BONUS_TILE_SELECTED;
//                            scene.update();
//                            break;
//                        }
//                    }
//
//                }
//                else if (tile->getAddlAttribute() == 0)
//                {
//                    /* a regular tile*/
//                    if (curselBonusTile)
//                    {
//                        if (tile->getTileLetter() == "")
//                        {
//                            // this tile is solved. just put the letter in it and set requiredBonusTile
//                            tile->setTileLetter(curselBonusTile->getTileLetter());
//                            requiredBonusTile = tile;
//                            tile->setTileBrush(brushBonusActive);
//                            cornerState = BOTH_CORNERS_OFF;
//                            scene.update();
//                            break;
//                        }
//                    }
//                }
//            }
//            cornerState = BOTH_CORNERS_OFF;
//            scene.update();
//
//
//        }
//        break;

    case NO_TILES_CLICKED:
        {
            QGraphicsItem* item = scene.itemAt(x, y);
            Tile* tile = dynamic_cast<Tile *>(item);
            if (tile)
            {
//                if (tile->getAddlAttribute() == 1 && bonusTilesAllowed)
//                {
//                    foreach (Tile* t, bonusTiles)
//                    {
//                        t->setTileBrush(brushBonusActive);
//                    }
//
//                    tile->setTileBrush(brushUnsolved);
//                    curselBonusTile = tile;
//                    cornerState = BONUS_TILE_SELECTED;
//
//                    if (requiredBonusTile)
//                    {
//                        requiredBonusTile->setTileLetter("");
//                        requiredBonusTile->setTileBrush(brushSolved);
//                        requiredBonusTile = NULL;
//                    }
//
//                    scene.update();
//                    break; // bonus tile selected!
//                }

                x1 = tile->tileCoordX;
                y1 = tile->tileCoordY;
                clickState = FIRST_TILE_CLICKED;
                tile->setTileBrush(brushBonusActive);
                tileRect1 = tile;
            }

//            x1 = qRound(x/curTileWidth);
//            y1 = qRound(y/curTileWidth);
//            firstCorner->setVisible(true);
//
//            if (x1 < 0) x1 = 0;
//            if (y1 < 0) y1 = 0;
//            if (x1 > boardWidth) x1 = boardWidth;
//            if (y1 > boardHeight) y1 = boardHeight;
//            firstCorner->setPos(x1*curTileWidth - crossHairsWidth/2, y1*curTileWidth - crossHairsWidth/2);
//            cornerState = LEFT_CORNER_ON;
            break;
        }
    case FIRST_TILE_CLICKED:

        QGraphicsItem* item = scene.itemAt(x, y);
        Tile* tile = dynamic_cast<Tile *>(item);
        if (tile)
        {
            x2 = tile->tileCoordX;
            y2 = tile->tileCoordY;
//            x2 = qRound(x/curTileWidth);
//            y2 = qRound(y/curTileWidth);
//            if (x2 < 0) x2 = 0;
//            if (y2 < 0) y2 = 0;
//            if (x2 > boardWidth) x2 = boardWidth;
//            if (y2 > boardHeight) y2 = boardHeight;
//
//            firstCorner->setVisible(false);
//            secondCorner->setVisible(false);
            if (tileRect1 && tileRect1->getTileLetter() != "")
                tileRect1->setTileBrush(brushUnsolved);

            clickState = NO_TILES_CLICKED;
            possibleRectangleCheck();
            line1->setVisible(false);
            line2->setVisible(false);
            line3->setVisible(false);
            line4->setVisible(false);

        }
        break;

    }


}

void MainWindow::keyPressed(int keyCode)
{
    ui->textEdit->append("Key was pressed: " + QString::number(keyCode));
}

//void MainWindow::tileMouseCornerClicked(int x, int y)
//{
//    if (!gameGoing)
//    {
//        clickState = NO_TILES_CLICKED;
//        return;
//    }
//    switch (cornerState)
//    {
//    case BOTH_CORNERS_OFF:
//        firstCorner->setVisible(true);
//        firstCorner->setPos(x * curTileWidth - crossHairsWidth/2, y * curTileWidth - crossHairsWidth/2);
//        cornerState = LEFT_CORNER_ON;
//        x1 = x; y1 = y;
//        break;
//    case LEFT_CORNER_ON:
//        secondCorner->setVisible(true);
//        secondCorner->setPos(x * curTileWidth - crossHairsWidth/2, y * curTileWidth - crossHairsWidth/2);
//        firstCorner->setVisible(false);
//        secondCorner->setVisible(false);
//        x2 = x; y2 = y;
//        cornerState = BOTH_CORNERS_OFF;
//        possibleRectangleCheck();
//        break;
//
//    }
//
//}

void MainWindow::possibleRectangleCheck()
{
    // check the rectangle formed from x1, y1 to x2, y2 for alphagram

    if (x1 == x2 && y1 == y2) return;

    QString letters;
    bool bonusTileUsed = false;
    for (int j = qMin(y1, y2); j <= qMax(y1, y2); j++)
    {
        for (int i = qMin(x1, x2); i <= qMax(x1, x2); i++)
        {

            int index = j*boardWidth + i;
            letters += tiles.at(index)->getTileLetter();
//            if (requiredBonusTile)
//                if (tiles.at(index) == requiredBonusTile) bonusTileUsed = true;
        }
    }
//    if (requiredBonusTile && !bonusTileUsed) return;    // if a required tile was not in the rectangle just ignore it!
    if (letters.size() < 2) return;

     QString debugStr = QString("%1 %2 %3 %4 %5").arg(x1).arg(y1).arg(x2).arg(y2).arg(letters);
     ui->labelDebug->setText(debugStr);

    letters = alphagrammize(letters.toUpper());

    /* look for this string in the db*/

    if (wordStructure->wordStructure.contains(letters))
    {
        QString answers = wordStructure->wordStructure.value(letters);
        ui->listWidget->insertItem(0, answers);
        for (int j = qMin(y1, y2); j <= qMax(y1, y2); j++)
        {
            for (int i = qMin(x1, x2); i <= qMax(x1, x2); i++)
            {

                int index = j*boardWidth + i;
                tiles.at(index)->setTileLetter("");
                tiles.at(index)->setTileBrush(brushSolved);
                simpleGridRep[i][j] = ' ';
            }
        }
        int thisScore = 0;
        int wordLength = letters.size();
        /* handle special QU case */
        if (letters.contains("Q")) wordLength--;    // Q only exists with U. this is a dirty hack.
        if (wordLength <= 15)
        {
            int effectiveWordLength = wordLength;
//            if (requiredBonusTile)
//            {
//                numSolvedLetters--; // take away a letter if we used a bonus
//                effectiveWordLength--;
//                if (effectiveWordLength < 2) effectiveWordLength = 2;
//                requiredBonusTile = NULL;
//            }
            thisScore = scoresByLength[effectiveWordLength];
            solvedWordsByLength[wordLength]++;
        }
        else
            thisScore = 400;


        numSolvedLetters += wordLength;

        if (wordLength != 2)
        {
            ui->textEdit->append(QString("+%1 for %2-letter word!").
                                 arg(thisScore).arg(wordLength));
            curScore += thisScore;
        }   /* do not count 2-letter words in score! */


        if (numSolvedLetters == boardWidth * boardHeight)
        {
            // cleared whole board

            int clearBonus = boardWidth * boardHeight * 2;
            curScore += clearBonus;
            ui->textEdit->append(QString("Got +%1 for clearing whole board! Good job!").arg(clearBonus));
            int timeBonus = timerSecs;
            ui->textEdit->append(QString("Got %1 for time left!").arg(timeBonus));
            curScore += timeBonus;
            timerSecs = 0;
        }

        /* deal with bonus tiles */
        if (numSolvedLetters > 0 && numSolvedLetters < (boardWidth*boardHeight) - bonusTurnoffTiles)
        {

            if (!bonusTilesAllowed)
            {
                bonusTilesAllowed = true;
//                foreach (Tile* tile, bonusTiles)
//                {
//                    tile->setTileBrush(brushBonusActive);
//                }
            }
        }
        else
        {
            if (bonusTilesAllowed)      // this makes this message only pop up once
            {
                ui->textEdit->append(QString("Turned off bonus tiles with %1 tiles left!").
                                     arg(boardWidth*boardHeight - numSolvedLetters));
//                foreach (Tile* tile, bonusTiles)
//                {
//                    tile->setTileBrush(brushBonusInactive);
//                }
                bonusTilesAllowed = false;
            }
        }


        ui->lcdNumberScore->display(curScore);

        scene.update();
        generateFindList();

    }
    else
    {
        timerSecs -= 10;
        ui->textEdit->append("<font color=red>10-second penalty!</font>");
    }




}


QString MainWindow::alphagrammize(QString word)
{
    QString ret = word;
    letterList.clear();
    for (int i = 0; i < word.size(); i++)
        letterList << word[i].toLatin1();

    qSort(letterList);

    for (int i = 0; i < letterList.size(); i++)
        ret[i] = letterList[i];

    return ret;
}


void MainWindow::setTilesPos()
{
    foreach (Tile* tile, tiles)
    {

        tile->setPos(tile->tileCoordX * curTileWidth, tile->tileCoordY * curTileWidth);
    }

//    foreach (Tile* tile, bonusTiles)
//    {
//
//        tile->setPos(tile->tileCoordX * curTileWidth, tile->tileCoordY * curTileWidth);
//    }


    //    ui->graphicsView->resetTransform();
    //    ui->graphicsView->translate(-100, -100);
    scene.setSceneRect(QRectF(0, 0, curTileWidth*boardWidth, curTileWidth*boardHeight));
}

void MainWindow::on_pushButtonGiveUp_clicked()
{
    timerSecs = 0;
}

void MainWindow::on_pushButtonRetry_clicked()
{
    shouldLoadNextNewGame = true;
    gameToLoad = currentGameCode;
    ui->pushButtonNewGame->animateClick();
//
//    if (!loadedWordStructure) return;
//
//    bonusTurnoffTiles = ui->spinBoxRetinaTurnOff->value();
//    minLengthHints = ui->spinBoxMinLengthGen->value();
//    maxLengthHints = ui->spinBoxMaxLengthGen->value();
//
//    int i = 0;
//    foreach (Tile* tile, tiles)
//    {
//        tile->setTileLetter(thisRoundLetters.at(i));
//        tile->setTileBrush(brushUnsolved);
//        i++;
//    }
//    timerSecs = lastTimerSecs;
//    ui->lcdNumber->display(timerSecs);
//    ui->lcdNumberScore->display(0);
//    gameTimer.start();
//    ui->listWidget->clear();
//    gameGoing = true;
//    curScore = 0;
//    numSolvedLetters = 0;
//    ui->textEdit->append("---------------------------------------------");
//
//    for (int i = 0; i < 16; i++)
//        solvedWordsByLength[i] = 0;
//
//    generateFindList();
//    scene.update();
}

void MainWindow::on_pushButtonNewGame_clicked()
{
    if (!loadedWordStructure) return;


    bonusTurnoffTiles = ui->spinBoxRetinaTurnOff->value();
    minLengthHints = ui->spinBoxMinLengthGen->value();
    maxLengthHints = ui->spinBoxMaxLengthGen->value();



    int gridSize = 0;
    bool ok;
    if (!shouldLoadNextNewGame)
    {


        gridSize = QInputDialog::getInteger(this, "Grid size?", "Please select a grid size", lastGridSize,
                                            MIN_GRID_SIZE, MAX_GRID_SIZE, 1, &ok);

        if (!ok) return;
    }
    else
    {
        gridSize = sqrt(gameToLoad.length());
    }

    timerSecs = QInputDialog::getInteger(this, "Timer?", "Please select desired timer (seconds)",
                                         gridSize*gridSize*3, 1, 5000, 1, &ok);
    if (!ok) timerSecs = gridSize*gridSize*3;
    lastTimerSecs = timerSecs;

    thisRoundLetters.clear();
    foreach (Tile* tile, tiles)
        tile->deleteLater();
    tiles.clear();

//    foreach (Tile* tile, bonusTiles)
//        tile->deleteLater();
//
//    bonusTiles.clear();


    lastGridSize = gridSize;
    boardHeight = gridSize;
    boardWidth = gridSize;

//    QString bonus = "RETINA";
//    for (int i = 0; i < bonus.length(); i++)
//    {
//        Tile* tile = new Tile();
//        bonusTiles << tile;
//        scene.addItem(tile);
//        tile->setTileCoords(i-3 + boardWidth/2, -2);
//        tile->setWidth(curTileWidth, 1);
//        tile->setTileBrush(brushBonusInactive);
//        tile->setTileLetter(QString(bonus[i]));
//        tile->setAddlAttribute(1);
//    }

    bonusTilesAllowed = false;  // bonus tiles are not allowed right at the beginning
//    requiredBonusTile = NULL;


    for (int j = 0; j < boardHeight; j++)
    {
        for (int i = 0; i < boardWidth; i++)
        {
            Tile* tile = new Tile();
            tiles << tile;
            scene.addItem(tile);
            tile->setTileCoords(i, j);
            tile->setWidth(curTileWidth, 1);
            tile->setTileBrush(brushUnsolved);
            // connect(tile, SIGNAL(mousePressedCorner(int, int)), SLOT(tileMouseCornerClicked(int, int)));
        }
    }
    setTilesPos();

    int index = 0;

    if (!shouldLoadNextNewGame)
    {
        currentGameCode = "";
        foreach (Tile* tile, tiles)
        {
            int letter = qrand()%letterDistSum;
            int accum = 0;
            int lettercounter;
            for (lettercounter = 0; lettercounter < 26; lettercounter++)
            {
                accum += letterDist[lettercounter];
                if (letter < accum) break;
            }
            /* handle special Q case */
            if ((char)lettercounter + 'A' == 'Q')
            {
                tile->setTileLetter("Qu");
                thisRoundLetters << "Qu";
            }
            else
            {
                tile->setTileLetter(QString((char)lettercounter + 'A'));
                thisRoundLetters << QString((char)lettercounter + 'A');
            }
            currentGameCode += ((char)lettercounter + 'A');
            tile->setTileBrush(brushUnsolved);
            int x = index % boardWidth;
            int y = index / boardWidth;
            simpleGridRep[x][y] = (char)lettercounter + 'A';
            index++;
        }

    }
    else
    {

        foreach (Tile* tile, tiles)
        {
            if (gameToLoad[index] == 'Q')
            {
                tile->setTileLetter("Qu");
                thisRoundLetters << "Qu";
            }
            else
            {
                tile->setTileLetter(QString(gameToLoad[index]));
                thisRoundLetters << QString(gameToLoad[index]);
            }
            tile->setTileBrush(brushUnsolved);
            int x = index % boardWidth;
            int y = index / boardWidth;
            simpleGridRep[x][y] = gameToLoad[index].toAscii();
            index++;
        }

        shouldLoadNextNewGame = false;
        currentGameCode = gameToLoad;
    }



    for (int j = 0; j < boardHeight; j++)
    {
        QString line;
        for (int i = 0; i < boardWidth; i++)
        {
            line += simpleGridRep[i][j];
        }
        qDebug() << line;
    }

    ui->lcdNumber->display(timerSecs);
    ui->lcdNumberScore->display(0);
    gameTimer.start();
    ui->listWidget->clear();
    gameGoing = true;
    curScore = 0;
    numSolvedLetters = 0;
    ui->textEdit->append("---------------------------------------------");

    for (int i = 0; i < 16; i++)
        solvedWordsByLength[i] = 0;

    generateFindList();

    //    scene.update();


}

void MainWindow::secPassed()
{
    timerSecs--;
    if (timerSecs <= 0)
    {
        gameTimer.stop();
        gameGoing = false;
        ui->textEdit->append("<font color=red>Time is up!</font>");
        QString summary = "You scored %1 points. Word breakdown by length:";
        summary = summary.arg(curScore);
        for (int i = 0; i < 16; i++)
        {
            if (solvedWordsByLength[i] > 0)
                summary += " <font color=green>" + QString::number(i)
                + "s:</font> " + QString::number(solvedWordsByLength[i]);
        }
        ui->textEdit->append(summary);
    }
    ui->lcdNumber->display(qMax(timerSecs, 0));
}

void MainWindow::on_toolButtonMinusSize_clicked()
{
    curTileWidth -=5;
    if (curTileWidth < 5) curTileWidth = 5;
    foreach (Tile* tile, tiles)
        tile->setWidth(curTileWidth, 1);
//
//    foreach (Tile* tile, bonusTiles)
//        tile->setWidth(curTileWidth, 1);

    setTilesPos();
}

void MainWindow::on_toolButtonPlusSize_clicked()
{
    curTileWidth +=5;
    if (curTileWidth >= 60) curTileWidth = 60;
    foreach (Tile* tile, tiles)
        tile->setWidth(curTileWidth, 1);
//
//    foreach (Tile* tile, bonusTiles)
//        tile->setWidth(curTileWidth, 1);

    setTilesPos();
}

MainWindow::~MainWindow()
{
    delete ui;
    foreach (Tile* tile, tiles)
        tile->deleteLater();
//
//    foreach (Tile* tile, bonusTiles)
//        tile->deleteLater();


}

void MainWindow::generateFindList()
{
    QTime t;
    t.start();
    /* here generate all possible rectangles with words of certain sizes */


    QSet <QString>alphaSet;
    for (int i = 0; i < lastGridSize; i++)
    {
        for (int j = 0; j < lastGridSize; j++)
        {
            // qDebug() << "i j" << i << j;
            generateSingleFindList(minLengthHints, maxLengthHints, i, j, alphaSet);
        }
    }
    qDebug() << "Rectangle generation:" << t.elapsed();

   // ui->textEdit->append(QString("lgs %1 %2").arg(lastGridSize).arg(alphaSet.size()));

    ui->listWidgetWordsToFind->clear();

    foreach (QString alph, alphaSet)
    {
        if (wordStructure->wordStructure.contains(alph))
        {

            ui->listWidgetWordsToFind->insertItem(0, wordStructure->wordStructure.value(alph));

        }
    }
    qDebug() << "All words: " << t.elapsed();
}

QString MainWindow::extractStringsFromRectangle(int TLi, int TLj, int BRi, int BRj, int minLength, int maxLength)
{
    QString word;
    int lengthSoFar = 0;
    for (int i = TLi; i <= BRi; i++)
    {
        for (int j = TLj; j <= BRj; j++)
        {
            char letter = simpleGridRep[i][j];
            if (letter != ' ')
            {
                lengthSoFar++;
                word += letter;
            }
            if (letter == 'Q')
            {
                // special case, handle QU
                lengthSoFar++;
                word += 'U';
            }
            if (lengthSoFar > maxLength) return ""; // return blank string if the length is already too long.
        }
    }
    if (lengthSoFar >= minLength && lengthSoFar <= maxLength) return word;
    else return "";
}

void MainWindow::generateSingleFindList(int minLength, int maxLength,
                                        int TLi, int TLj, QSet<QString>& alphaSet)
{
    //
    //    lengthSoFar++;
    //    if (lengthSoFar > maxLength) return;
    /* try all squares to the right and bottom as the "bottom right corner" */

    QString curStr;
    int lengthSoFar = 0;
    for (int i = TLi; i < lastGridSize; i++)
    {
        for (int j = TLj; j < lastGridSize; j++)
        {
            QString possibleWord = extractStringsFromRectangle(TLi, TLj, i, j, minLength, maxLength);
            if (possibleWord != "") alphaSet.insert(alphagrammize(possibleWord));
        }
    }

}

void MainWindow::finishedLoadingWordStructure()
{
    loadedWordStructure = true;
    ui->textEdit->append("Loaded word structure! You may begin playing.");
}

void MainWindow::on_actionLoad_board_triggered()
{
    QString code = QInputDialog::getText(this, "Enter code", "Enter passcode");
    if (code != "" && isPerfectSquare(code.length()))
    {
        shouldLoadNextNewGame = true;
        gameToLoad = code.simplified().toUpper();
        ui->pushButtonNewGame->animateClick();
    }
    else
    {
        QMessageBox::warning(this, "Code incorrect!", "Code incorrect");
    }
}

void MainWindow::on_actionSave_board_triggered()
{
    if (currentGameCode != "")
    {
        ui->textEdit->append("Use the following code when you click 'Load game':");
        ui->textEdit->append(currentGameCode);
    }
}

/***********************/

void WordgridsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    emit sceneMouseClicked(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
    mouseEvent->ignore();
}


void WordgridsScene::mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent )
{
    emit sceneMouseClicked(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
    mouseEvent->ignore();

}

void WordgridsScene::mouseMoveEvent (QGraphicsSceneMouseEvent * mouseEvent )
{
    emit sceneMouseMoved(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
    mouseEvent->ignore();
}

void WordgridsScene::keyPressEvent ( QKeyEvent * keyEvent )
{
    emit keyPressed(keyEvent->key());
    keyEvent->ignore();
}
