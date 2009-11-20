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

#ifndef GAMETABLE_H
#define GAMETABLE_H

#include <QtGui>
#include "databasehandler.h"
#include "ui_playerInfoForm.h"

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
    //virtual void setDatabase(QString name) = 0;
    void setLexicon(QString name)
    {
        lexiconName = name;
        wordDb = dbHandler->lexiconMap.value(lexiconName).db;
    }

signals:
    void avatarChange(quint8);
protected:
    QSqlDatabase wordDb;
    DatabaseHandler* dbHandler;
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
    QString lexiconName;

private slots:

    void possibleAvatarChangeLeft();
    void possibleAvatarChangeRight();
};


#endif // GAMETABLE_H
