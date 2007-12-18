#include "chip.h"

Chip::Chip()
{


	chipBrush = QBrush(Qt::red);

	foregroundPen = QPen(Qt::black);

	edgePen = QPen(chipBrush.color().darker(200), 0);

	//setFlag(QGraphicsItem::ItemIsMovable);
	numberMode = true;
}

void Chip::setChipProperties(QBrush& b, QPen& p, QPen &e)
{
	chipBrush = b;
	foregroundPen = p;
	edgePen = e;
}

void Chip::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
	Q_UNUSED(widget);


	if (numberMode)
	  {

	    QLinearGradient linearGrad(QPointF(0, 0), QPointF(18, 18));
	    
	    
	    switch(number)
	      {
	      case 1:
		linearGrad.setColorAt(0, QColor(184, 7, 9).lighter(150));			// 7 9 184
		linearGrad.setColorAt(1, QColor(175, 55, 75).lighter(150));			// 55 75 175
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(255, 0, 0).darker(200), 1);
		break;
	      case 2:
		linearGrad.setColorAt(0, QColor(7, 9, 184).lighter(150));
		linearGrad.setColorAt(1, QColor(55, 75, 175).lighter(150));
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(0, 0, 255).darker(200), 1);
		break;
	      case 3:
		linearGrad.setColorAt(0, QColor(255, 193, 37));			
		linearGrad.setColorAt(1, QColor(205, 155, 29));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(205, 155, 29).darker(200), 1);
		break;
	      case 4:
		linearGrad.setColorAt(0, QColor(255, 235, 205));			
		linearGrad.setColorAt(1, QColor(205, 179, 139));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(205, 179, 139).darker(200), 1);
		break;
	      case 5:
		linearGrad.setColorAt(0, QColor(210, 105, 30));			
		linearGrad.setColorAt(1, QColor(139, 69, 19));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(139, 69, 19).darker(200), 1);
		break;
	      case 6:
		linearGrad.setColorAt(0, QColor(250, 128, 114));			
		linearGrad.setColorAt(1, QColor(255, 99, 71));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(255, 99, 71).darker(200), 1);
		break;
	      case 7:
		linearGrad.setColorAt(0, QColor(113, 198, 113));			
		linearGrad.setColorAt(1, QColor(124, 205, 124));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(124, 205, 124).darker(200), 1);
		break;
	      case 8:
		linearGrad.setColorAt(0, QColor(127, 255, 212));			
		linearGrad.setColorAt(1, QColor(102, 205, 170));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::black);
		edgePen = QPen(QColor(102, 205, 170).darker(200), 1);
		break;
	      case 9:
		linearGrad.setColorAt(0, QColor(155, 48, 255));			
		linearGrad.setColorAt(1, QColor(85, 26, 139));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::white);
		edgePen = QPen(QColor(85, 26, 139).darker(200), 1);
		break;
	      default:
		linearGrad.setColorAt(0, QColor(41, 36, 33));			
		linearGrad.setColorAt(1, QColor(0, 0, 0));			
		chipBrush = QBrush(linearGrad);
		foregroundPen = QPen(Qt::white);
		edgePen = QPen(QColor(41, 36, 33), 1);
		
		
	      }

	  }

	painter->setPen(QPen(chipBrush, 0));
	painter->setBrush(chipBrush);
	painter->drawEllipse(QRectF(0.0, 0.0, 18.0, 18.0));
	
	// draw shadow
	painter->setPen(edgePen);
	painter->drawArc(QRectF(0.0, 0.0, 18.0, 18.0), 0.0, -150*16);
	// Draw text
	
	// draw "spokes"

	//QPainterPath path;
	//path.moveTo(9.5, 9.5);

	painter->setPen(QPen(Qt::white, 1));
	for (int i = 0; i < 360; i+=45)
	  painter->drawArc(QRectF(1.0, 1.0, 15.0, 15.0), i * 16, 10 * 16);


	QFont font("Courier", 16, 70);
	font.setStyleStrategy(QFont::PreferAntialias);
	painter->setFont(font);
	painter->setPen(foregroundPen);

	if (numberMode)
	  painter->drawText(QRectF(1, 2, 17, 17), Qt::AlignCenter, QString("%1").arg(number));
	else
	  painter->drawText(QRectF(1, 2, 17, 17), Qt::AlignCenter, chipString);

}

void Chip::setNumberMode(bool mode)
{
  numberMode = mode;
}

QRectF Chip::boundingRect() const
{
	return QRectF(0, 0, 19, 19);
}

void Chip::setChipNumber(quint8 number)
{
	this->number = number;
}

void Chip::setChipString(QString chipString)
{
  this->chipString = chipString;
  numberMode = false;

}
