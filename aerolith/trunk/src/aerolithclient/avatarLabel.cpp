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

#include "avatarLabel.h"

avatarLabel::avatarLabel(QWidget *parent) : QLabel(parent)
{
    left_pressed = false;
    right_pressed = false;
    setFixedWidth(40);
    setFixedHeight(40);
}

void avatarLabel::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        left_pressed = true;
    }
    else if (e->button() == Qt::RightButton)
    {
        right_pressed = true;
    }
}

void avatarLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if (left_pressed && e->button() == Qt::LeftButton && inLabel(e->pos()))
    {
        emit leftMouseClicked();
        qDebug() << "emitted left";
    }
    else if (right_pressed && e->button() == Qt::RightButton && inLabel(e->pos()))
    {
        emit rightMouseClicked();
        qDebug() << "emitted right";
    }
    left_pressed = false;
    right_pressed = false;

}

bool avatarLabel::inLabel(const QPoint &p)
{
    return rect().contains(p);
}
