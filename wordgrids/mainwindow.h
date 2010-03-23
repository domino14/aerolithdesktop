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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui>
#include <QtCore>
#include "Tile.h"

#include "wordstructure.h"
#include "ui_preferences.h"
namespace Ui
{
    class MainWindowClass;
}

class WordgridsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    WordgridsScene(QObject* parent);
private:
    void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
    void keyPressEvent ( QKeyEvent * event );
signals:
    void sceneMouseClicked(double, double);
    void keyPressed(int);

};

#define MIN_GRID_SIZE 3
#define MAX_GRID_SIZE 12

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();



private:
    enum ClickStates
    {
        FIRST_TILE_CLICKED, BOTH_TILES_CLICKED, NO_TILES_CLICKED
            };

    Ui::MainWindowClass *ui;

    QWidget* preferencesWidget;
    Ui::preferencesForm uiPreferences;

    WordgridsScene* scene;
    Tile* curselBonusTile;




    //    Tile* requiredBonusTile;
    bool bonusTilesAllowed;

    QVector <Tile*> tiles;

    QSet <Tile*> highlightedTiles;

    //    QVector <Tile*> bonusTiles;
    Tile *tileRect1, *tileRect2;
    QGraphicsLineItem *line1, *line2, *line3, *line4;
    QGraphicsSimpleTextItem* scoreLabel;
    QGraphicsSimpleTextItem* gameScore;

    QGraphicsSimpleTextItem* bonusTileLabel;
    QGraphicsSimpleTextItem* dashInstructionsLabel;

    QGraphicsSimpleTextItem* thisScoreLabel;

    Tile *bonusTile;

    ClickStates clickState;
    int crossHairsWidth;
    void setTilesPos();
    QTimer gameTimer;
    int timerSecs, lastTimerSecs;
    int curTileWidth;
    int curScore;

    WordStructure* wordStructure;

    bool gameGoing;
    void possibleRectangleCheck();

    int boardWidth, boardHeight;

    int x1, y1, x2, y2;
    QString alphagrammize(QString);
    QList<unsigned char> letterList;
    QList <QString> thisRoundLetters;



    int numSolvedLetters;
    int lastGridSize;
    int solvedWordsByLength[16];
    char simpleGridRep[MAX_GRID_SIZE][MAX_GRID_SIZE];


    void generateSingleFindList(int, int, int, int, QSet<QString>&);
    QString extractStringsFromRectangle(int TLi, int TLj, int BRi, int BRj, int minLength, int maxLength);
    bool loadedWordStructure;

    /* for Word Struck: */
    int bonusTurnoffTiles;

    /* for Word Dash: */
    int minToGenerate;
    int maxToGenerate;
    int curGenerating;
    int curToSolve;
    int curSolved;
    QSet <QString> foundAlphaset;
    QSet <QString> solvedAlphaset;
    void generateFindList();
    bool populateNextFindList();
    void markInWordList(QString str);



    bool shouldLoadNextNewGame;
    QString gameToLoad;
    QString currentGameCode;

    void resetTilesHighlightStatus();
    double mouseX, mouseY;
    void displayScore(int);
    void showSingleScore(int score, int x1, int x2, int y1, int y2,bool penalty = false);
    void closeEvent ( QCloseEvent * event );
    void quit();
    void writeSettings();
    void readSettings();
    QString currentGameDescription;
    QTimer* scoreTimer;
public slots:
    //   void tileMouseCornerClicked(int, int);
private slots:
    void on_actionLoad_board_triggered();
    void on_actionSave_board_triggered();
    void on_actionEdit_game_preferences_triggered();
    void secPassed();
    void sceneMouseClicked(double, double);
    void mouseOverTile();
    void mouseOutOfTile();
    void keyPressed(int);
    void on_pushButtonNewGame_clicked();
    void on_pushButtonRetry_clicked();
    void on_pushButtonGiveUp_clicked();

    void on_toolButtonMinusSize_clicked();
    void on_toolButtonPlusSize_clicked();
    void finishedLoadingWordStructure();
    void on_pushButtonSwitchGame_clicked();

    void hideSingleScore();

};

#endif // MAINWINDOW_H
