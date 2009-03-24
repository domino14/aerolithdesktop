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

#include "GameTable.h"

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
        playerUis.at(indexOfPlayer).labelAddInfo->setText(QString("%1").
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
        for (int i = 0; i < numPlayers; i++)
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