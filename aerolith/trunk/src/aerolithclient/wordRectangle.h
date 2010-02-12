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

#ifndef _WORDRECTANGLE_H_
#define _WORDRECTANGLE_H_

#include <QtGui>
#include <QtCore>

class WordRectangle : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    WordRectangle();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
    QRectF boundingRect() const;
    void setText(QString);
    void setFixedWidthFont(bool on);
    void showText();
    void hideText();
    void setTransparentBG();
    void setOpaqueBG();
    void alphagrammizeText();
    void shuffleText();
    void setLexiconForMapping(QString);
private:
    QString currentLexicon;
    QString originalText;
    QString text;
    bool opaqueBG;
    bool shouldShowText;
    void mousePressEvent ( QGraphicsSceneMouseEvent * event);
    QString fontFamily;
signals:
    void mousePressed();
};

#endif
