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

void Tile::setLexiconForMapping(QString l)
{
    currentLexicon = l;
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
    /*   int fontSize = 25;

        // find best font size

        QRectF textBoundingRect;
        do
        {
            fontSize--;
            QFont font(fontFamily, fontSize, 100);
            //	font.setStyleStrategy(QFont::PreferAntialias);

            painter->setFont(font);

            textBoundingRect = painter->boundingRect(QRectF(20, 0, 185, 26), Qt::AlignCenter, displayedText);

        } while (textBoundingRect.width() > 182 || textBoundingRect.height() > 26);*/

    painter->setFont(tileFont);
    painter->setPen(foregroundPen);
    painter->drawText(QRectF(0, 1, width-1, height-1), Qt::AlignCenter, tileLetter);

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
    if (currentLexicon == "FISE")
    {
        /* special symbols */
        if (tileLetter == "1")
            this->tileLetter = "CH";
        if (tileLetter == "2")
            this->tileLetter = "LL";
        if (tileLetter == "3")
            this->tileLetter = "RR";
        if (tileLetter == "4")
            this->tileLetter = QChar(0x00D1); // capital N-tilde

    }

    int fontSize = 25;

    // find best font size

    QRect textBoundingRect;
    tileFont.setFamily("Courier New");
    tileFont.setWeight(100);


    do
    {
        fontSize--;

        tileFont.setPointSize(fontSize);
        QFontMetrics fm(tileFont);
        //	font.setStyleStrategy(QFont::PreferAntialias);
    //    textBoundingRect = fm.boundingRect(QRect(0, 0, width, height), Qt::AlignCenter, this->tileLetter);
        textBoundingRect = fm.boundingRect(this->tileLetter);
        qDebug() << "Letter: " << this->tileLetter << textBoundingRect;
    } while (textBoundingRect.width() >= width || textBoundingRect.height() >= (double)height*1.3);

}

void Tile::mousePressEvent ( QGraphicsSceneMouseEvent * event)
{
    event->ignore();
    emit mousePressed();
}
