#include "tile.h"

Tile::Tile()
{
	QLinearGradient linearGrad(QPointF(0, 0), QPointF(18, 18));
	linearGrad.setColorAt(0, QColor(7, 9, 184));
	linearGrad.setColorAt(1, QColor(55, 75, 175));
	tileBrush = QBrush(linearGrad);

	foregroundPen = QPen(Qt::white);
	setFlag(QGraphicsItem::ItemIsMovable);
}

QRectF Tile::boundingRect() const
{
	return QRectF(0, 0, 19, 19);
}

void Tile::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

	painter->setPen(QPen(tileBrush, 0));
	painter->setBrush(tileBrush);
    painter->drawRect(0, 0, 18, 18);

    QPen oldPen = painter->pen();
    QPen pen = oldPen;
	
	// draw black shadow
	painter->setPen(QPen(Qt::black, 2));
	painter->drawLine(1, 18, 18, 18);
	painter->drawLine(18, 18, 18, 1);
    // Draw text
   
	QFont font("Courier New", 18, 70);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);
	painter->setPen(foregroundPen);
    painter->drawText(2, 16, tileLetter);
   
}

void Tile::setTileProperties(QBrush& b, QPen& p)
{
	tileBrush = b;
	foregroundPen = p;
}

void Tile::setTileLetter(QString tileLetter)
{
	this->tileLetter = tileLetter;

}