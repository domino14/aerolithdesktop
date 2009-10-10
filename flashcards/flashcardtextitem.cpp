#include "flashcardtextitem.h"


 FlashcardTextItem::FlashcardTextItem(QGraphicsItem *parent)
     : QGraphicsTextItem(parent)
 {
     setFlag(QGraphicsItem::ItemIsMovable);
     setFlag(QGraphicsItem::ItemIsSelectable);

     rh = new ResizeHandle(this);

     connect(rh, SIGNAL(movedBRHandle(QPointF)), this, SLOT(brHandleMoved(QPointF)));
     connect(rh, SIGNAL(movedTLHandle(QPointF)), this, SLOT(tlHandleMoved(QPointF)));

     connect(rh, SIGNAL(stoppedMovingHandles()), this, SLOT(refreshHandles()));
     lastTopX = 0;
     lastTopY = 0;
     lastWidth = 300;
 }

 QVariant FlashcardTextItem::itemChange(GraphicsItemChange change,
                      const QVariant &value)
 {
     if (change == QGraphicsItem::ItemSelectedHasChanged)
         emit selectedChange(this);
     return value;
 }

 void FlashcardTextItem::focusOutEvent(QFocusEvent *event)
 {
     setTextInteractionFlags(Qt::NoTextInteraction);
     emit lostFocus(this);
     QGraphicsTextItem::focusOutEvent(event);
 }

 void FlashcardTextItem::focusInEvent(QFocusEvent *event)
 {
     QGraphicsTextItem::focusInEvent(event);
 }


 void FlashcardTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
 {
     if (textInteractionFlags() == Qt::NoTextInteraction)
         setTextInteractionFlags(Qt::TextEditorInteraction);
     QGraphicsTextItem::mouseDoubleClickEvent(event);
 }

void FlashcardTextItem::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsTextItem::mousePressEvent(event);
}

void FlashcardTextItem::setText(QString text)
{
    this->setTextWidth(300);
    this->setHtml(text);

    rh->moveHandles(lastTopX, lastTopY, lastTopX + 300, lastTopY + this->boundingRect().height());
    lastWidth = this->boundingRect().width();
    lastTopX = this->boundingRect().x();
    lastTopY = this->boundingRect().y();

}

void FlashcardTextItem::brHandleMoved(QPointF pos)
{
    QPointF mapped = mapFromItem(rh->brHandle, pos);
    prepareGeometryChange();
    setTextWidth(qAbs(mapped.x()));
    lastWidth = this->textWidth();
    update();
}

void FlashcardTextItem::tlHandleMoved(QPointF pos)
{
    QPointF mapped = mapFromItem(rh->tlHandle, pos);
    prepareGeometryChange();
////    setTextWidth(qAbs(lastWidth - mapped.x()));
////    lastTopX = mapped.x();
////    lastTopY = mapped.y();
////    lastWidth = this->textWidth();
//
    QPointF toParent = mapToParent(mapped);

    this->setPos(toParent); // setPos is also moving the handles because they're children! how do i stop this?


    qDebug() << pos << mapped << toParent;

    update();
}

void FlashcardTextItem::refreshHandles()
{
    qDebug() << "refresh handles";
    rh->moveHandles(lastTopX, lastTopY, lastTopX + lastWidth, lastTopY + this->boundingRect().height());
    lastWidth = this->boundingRect().width();
    lastTopX = this->boundingRect().x();
    lastTopY = this->boundingRect().y();
}
