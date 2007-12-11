#ifndef _TILE_H_
#define _TILE_H_

#include <QtGui>
#include <QtCore>

class Tile : public QGraphicsItem
{
public:
	Tile();
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
	QRectF boundingRect() const;
	void setTileProperties(QBrush& b, QPen& p, QPen &e);
	void setTileLetter(QString tileLetter);

private:
	QBrush tileBrush;
	QPen foregroundPen;
	QPen edgePen;
	QString tileLetter;
};

#endif
