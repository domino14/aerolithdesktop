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

    rh->moveHandles(this->boundingRect());

}

void FlashcardTextItem::brHandleMoved(QPointF pos)
{
    QPointF mapped = mapFromItem(rh->brHandle, pos);
    prepareGeometryChange();
    setTextWidth(qAbs(mapped.x()));
    update();
}

void FlashcardTextItem::tlHandleMoved(QPointF pos)
{
    QPointF mapped = mapFromItem(rh->tlHandle, pos);
    prepareGeometryChange();
    setTextWidth(qAbs(this->boundingRect().width() - mapped.x()));
}

void FlashcardTextItem::refreshHandles()
{
    qDebug() << this->boundingRect();
    rh->moveHandles(this->boundingRect());
}
