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

#include "wordRectangle.h"

WordRectangle::WordRectangle()
{
    fontFamily = "Arial";
}

QRectF WordRectangle::boundingRect() const
{
    return QRectF(0, 0, 205, 26);

}

void WordRectangle::setFixedWidthFont(bool on)
{
    if (on)
    {
        fontFamily = "Courier New";
    }
    else
    {
        fontFamily = "Arial";
    }
}

void WordRectangle::hideText()
{
    shouldShowText = false;
}

void WordRectangle::showText()
{
    shouldShowText = true;
}

void WordRectangle::setTransparentBG()
{
    opaqueBG = false;
}

void WordRectangle::setOpaqueBG()
{
    opaqueBG = true;
}

void WordRectangle::setLexiconForMapping(QString l)
{
    currentLexicon = l;
}

void WordRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{

    if (opaqueBG)
        painter->setBrush(QBrush(QColor(230, 230, 230)));
    else
        painter->setBrush(QBrush(QColor(230, 230, 230, 50)));

    painter->drawRect(0, 0, 205, 26);
    painter->setRenderHint(QPainter::TextAntialiasing);

    if (shouldShowText)
    {
        QString displayedText = text;
        if (currentLexicon == "FISE")
        {
            /* the text has special symbols */
            displayedText = displayedText.replace('1', "CH");
            displayedText = displayedText.replace('2', "LL");
            displayedText = displayedText.replace('3', "RR");
            displayedText = displayedText.replace('4', QChar(0x00D1));  // a capital N-tilde character.

        }
        int fontSize = 25;

        // find best font size

        QRectF textBoundingRect;
        do
        {
            fontSize--;
            QFont font(fontFamily, fontSize, 100);
            //	font.setStyleStrategy(QFont::PreferAntialias);

            painter->setFont(font);

            textBoundingRect = painter->boundingRect(QRectF(20, 0, 185, 26), Qt::AlignCenter, displayedText);

        } while (textBoundingRect.width() > 182 || textBoundingRect.height() > 26);


        painter->drawText(QRectF(20, 0, 185, 26), Qt::AlignCenter, displayedText);

    }
}

void WordRectangle::setText(QString text)
{
    this->text = text;
    originalText = text;
}

void WordRectangle::mousePressEvent ( QGraphicsSceneMouseEvent * event)
{
    emit mousePressed();
}

void WordRectangle::alphagrammizeText()
{
    text = originalText;
}

void WordRectangle::shuffleText()
{
    for (int i = 0; i < text.length(); i++)
    {
        int j = qrand() % text.length();
        QChar tmp;
        tmp = text[i];
        text[i] = text[j];
        text[j] = tmp;
    }
}
