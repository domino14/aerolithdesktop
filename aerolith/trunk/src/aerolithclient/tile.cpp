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

#include "tile.h"

Tile::Tile()
{
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(18, 18));
    linearGrad.setColorAt(0, QColor(7, 9, 184).lighter(200));			// 7 9 184
    linearGrad.setColorAt(1, QColor(55, 75, 175).lighter(200));			// 55 75 175

    //linearGrad.setColorAt(0, QColor(255, 255, 255));
    //linearGrad.setColorAt(1, QColor(255, 255, 255));

    tileBrush = QBrush(linearGrad);

    //foregroundPen = QPen(Qt::white);
    foregroundPen = QPen(Qt::black);

    edgePen = QPen(Qt::black, 1);
    //edgePen = QPen(Qt::white, 2);
    width = 17;
    height = (double)width;
    //setFlag(QGraphicsItem::ItemIsMovable);
}

int Tile::getWidth()
{
    return width;
}

void Tile::setWidth(int w, double hscale)
{
    prepareGeometryChange();
    width = w;
    height = (double)width* hscale;

}

QRectF Tile::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void Tile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    painter->setPen(QPen(tileBrush, 0));
    painter->setBrush(tileBrush);
    painter->drawRect(0, 0, width-1, height-1);

    // draw black shadow
    painter->setPen(edgePen);
    painter->drawLine(1, height-1, width-1, height-1);
    painter->drawLine(width-1, height-1, width-1, 1);
    // Draw text

    int baseFontSize;
    if (width == height) baseFontSize = 0;
    else baseFontSize = 2;
#ifdef Q_WS_MAC
    QFont font("Courier New", width + 4 + baseFontSize, 75);
#else
    QFont font("Courier New", width + baseFontSize, 100);
#endif
    //	font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(foregroundPen);
#ifdef Q_WS_MAC
    painter->drawText(QRectF(0, 1, width-1, height-1), Qt::AlignCenter, tileLetter);
#else
    painter->drawText(QRectF(0, 1, width-1, height-1), Qt::AlignCenter, tileLetter);
#endif   
}

void Tile::setTileProperties(QBrush& b, QPen& p, QPen &e)
{
    tileBrush = b;
    foregroundPen = p;
    edgePen = e;
}

void Tile::setTileBrush(QBrush& b)
{
    tileBrush = b;
}

QBrush Tile::getTileBrush()
{
    return tileBrush;
}

void Tile::setForegroundPen(QPen& p)
{
    foregroundPen = p;
}

void Tile::setEdgePen(QPen &e)
{
    edgePen = e;
}

void Tile::setTileLetter(QString tileLetter)
{
    this->tileLetter = tileLetter;

}

void Tile::mousePressEvent ( QGraphicsSceneMouseEvent * event)
{
    event->ignore();
    emit mousePressed();
}
