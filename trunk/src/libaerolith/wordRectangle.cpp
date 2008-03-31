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
      int fontSize = 25;
      
  // find best font size
      
      QRectF textBoundingRect;
      do
	{
	  fontSize--;
	  QFont font(fontFamily, fontSize, 100);  
//	font.setStyleStrategy(QFont::PreferAntialias);

	  painter->setFont(font);

	  textBoundingRect = painter->boundingRect(QRectF(20, 0, 185, 26), Qt::AlignCenter, text);
	  
	} while (textBoundingRect.width() > 182 || textBoundingRect.height() > 26);
      
      
      painter->drawText(QRectF(20, 0, 185, 26), Qt::AlignCenter, text);
      
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
