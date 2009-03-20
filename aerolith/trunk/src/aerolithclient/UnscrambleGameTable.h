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

#ifndef _UNSCRAMBLE_GAME_TABLE_H_
#define _UNSCRAMBLE_GAME_TABLE_H_

#include <QtCore>
#include <QtGui>
#include <QtSql>
#include "ui_tableForm.h"
#include "ui_playerInfoForm.h"
#include "ui_solutionsForm.h"
#include "ui_tableCustomizationForm.h"
#include "tile.h"
#include "chip.h"
#include "wordRectangle.h"

#define NUM_AVATAR_IDS 73 

class GameTable : public QWidget
{

    Q_OBJECT
public:
    GameTable(QWidget* parent = 0, Qt::WindowFlags f = 0, int gamePlayers = 6);
    virtual ~GameTable() = 0;
    void setMyUsername(QString);

    void setAvatar(QString, quint8);
    virtual void setReadyIndicator(QString) = 0;
    virtual void clearReadyIndicators() = 0;
    virtual void setupForGameStart() = 0;
    void addToPlayerList(QString, QString);
    virtual void setDatabase(QString name) = 0;

signals:
    void avatarChange(quint8);
protected:
    QString myUsername;

    // most of these have to do with the player widgets.
    QHash <QString, int> seats;
    QList <Ui::playerInfoForm> playerUis;
    QList <QWidget*> playerWidgets;
    int numPlayers;
    void clearAndHidePlayers(bool hide);
    void playerLeaveTable();


    void addPlayersToWidgets(QStringList playerList);
    void removePlayerFromWidgets(QString, bool);
    void addPlayerToWidgets(QString, bool);

private slots:

    void possibleAvatarChangeLeft();
    void possibleAvatarChangeRight();
};


class UnscrambleGameTable : public GameTable
{
    Q_OBJECT

public:
    UnscrambleGameTable(QWidget* parent, Qt::WindowFlags f);
    ~UnscrambleGameTable();
    void resetTable(quint16, QString, QString);
    void leaveTable();
    void addPlayer(QString, bool);
    void removePlayer(QString, bool);
    void addPlayers(QStringList);

    void setupForGameStart();
    void gotChat(QString);
    void gotTimerValue(quint16 timerval);
    void gotWordListInfo(QString);

    void clearSolutionsDialog();
    void populateSolutionsTable();

    void addNewWord(int, QString, QStringList, quint8);
    void clearAllWordTiles();
    void answeredCorrectly(int index, QString username, QString answer);

    void setReadyIndicator(QString);
    void clearReadyIndicators();
    void setDatabase(QString name);
private:
    QSqlDatabase zyzzyvaDb;
    QGraphicsScene gfxScene;
    Ui::tableForm tableUi;
    int currentWordLength;
    QList <Tile*> tiles;
    QList <Chip*> chips;
    QList <Chip*> readyChips;
    QList <WordRectangle*> wordRectangles;

    QGraphicsPixmapItem* tableItem;
    QDialog* solutionsDialog;
    Ui::solutionsForm uiSolutions;

    QWidget* preferencesWidget;
    Ui::tableCustomizationForm uiPreferences;


    struct wordQuestion
    {
        wordQuestion(QString a, QStringList s, quint8 n)
        {
            alphagram = a;
            solutions = s;
            numNotYetSolved = n;
        };

        QString alphagram;
        QStringList solutions;
        quint8 numNotYetSolved;
        Chip* chip;
        QList <Tile*> tiles;
    };

    QList <wordQuestion> wordQuestions;
    QSet <QString> rightAnswers;
    double verticalVariation;
    double heightScale;
    void loadUserPreferences();
    void swapXPos(Tile*, Tile*);
    int getTileWidth(int wordLength);
    void getBasePosition(int index, double& x, double& y, int tileWidth);

protected:
    virtual void closeEvent(QCloseEvent*);
signals:
    void giveUp();
    void sendStartRequest();
    void guessSubmitted(QString);
    void chatTable(QString);
    void sendPM(QString);
    void exitThisTable();
    void viewProfile(QString);
        private slots:
    void enteredGuess();
    void enteredChat();

    void alphagrammizeWords();
    void shuffleWords();
    void tileWasClicked();
    void rectangleWasClicked();
    void setZoom(int);
    void changeTileColors(int);
    void changeFontColors(int);
    void changeTableStyle(int);
    void changeTileBorderStyle(bool);
    void changeVerticalVariation(bool);
    void changeBackground(int index);
    void drawWordBorders(bool);

    void changeTileAspectRatio(bool);
    void changeUseTiles(bool);
    void useFixedWidthFontForRectangles(bool);
    void pushedFontToggleButton();

    void saveUserPreferences();


public:


};



#endif
