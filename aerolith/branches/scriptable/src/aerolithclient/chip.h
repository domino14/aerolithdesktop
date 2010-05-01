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

#ifndef _CHIP_H_
#define _CHIP_H_

#include <QtGui>
#include <QtCore>

class Chip : public QGraphicsItem
{
public:
    Chip();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
    QRectF boundingRect() const;

    void setChipProperties(QBrush& b, QPen& p, QPen &e);
    void setChipNumber(quint8 number);
    void setNumberMode(bool mode);
    void setChipString(QString chipString);
private:
    QBrush chipBrush;
    QPen foregroundPen;
    QPen edgePen;
    quint8 number;
    bool numberMode;
    QString chipString;
};


#endif
