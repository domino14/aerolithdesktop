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
	QString tileLetter;
	void setTileProperties(QBrush& b, QPen& p);
	void setTileLetter(QString tileLetter);
private:
	QBrush tileBrush;
	QPen foregroundPen;

};

#endif
