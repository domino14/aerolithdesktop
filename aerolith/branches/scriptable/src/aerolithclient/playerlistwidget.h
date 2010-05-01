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

#ifndef _PLAYER_LIST_WIDGET_H_
#define _PLAYER_LIST_WIDGET_H_

#include <QtGui>

class playerListWidget : public QListWidget
{
    Q_OBJECT
public:
    playerListWidget(QWidget* w) : QListWidget(w)
    {
        contextMenu = new QMenu(this);
        contextMenu->addAction("View Profile");
        contextMenu->addAction("Send PM");
        connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuAction(QAction*)));
        connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(shouldSendPM(QListWidgetItem*)));
    }

protected:
    void contextMenuEvent(QContextMenuEvent* event)
    {
        if (itemAt(event->pos()) != NULL)// TODO CHECK IF THIS RETURNS NULL!
        {
            setCurrentItem(itemAt(event->pos()));
            contextMenu->popup(event->globalPos());
        }
    }
signals:
    void viewProfile(QString);
    void sendPM(QString);
private:
    QMenu* contextMenu;

private slots:
    void contextMenuAction(QAction* a)
    {
        if (a->text() == "View Profile") emit viewProfile(currentItem()->text());
        if (a->text() == "Send PM") emit sendPM(currentItem()->text());
    }

    void shouldSendPM(QListWidgetItem* i)
    {
        emit sendPM(i->text());

    }
};

#endif
