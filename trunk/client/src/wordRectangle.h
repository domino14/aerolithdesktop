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
private:
	QString text;
	void mousePressEvent ( QGraphicsSceneMouseEvent * event);
	QString fontFamily;
signals:
	void mousePressed();
};

#endif
