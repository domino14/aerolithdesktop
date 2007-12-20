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

void WordRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
  painter->setBrush(QBrush(QColor(220, 220, 220)));
  painter->drawRect(0, 0, 205, 26);

  int fontSize = 25;

  // find best font size

QRectF textBoundingRect;
  do
    {
      fontSize--;
      QFont font(fontFamily, fontSize, 100);  
      painter->setFont(font);
      textBoundingRect = painter->boundingRect(QRectF(20, 0, 185, 26), Qt::AlignCenter, text);
     
    } while (textBoundingRect.width() > 185);


  painter->drawText(QRectF(20, 0, 185, 26), Qt::AlignCenter, text);

}

void WordRectangle::setText(QString text)
{
  this->text = text;
}

void WordRectangle::mousePressEvent ( QGraphicsSceneMouseEvent * event)
{
  emit mousePressed();
}
