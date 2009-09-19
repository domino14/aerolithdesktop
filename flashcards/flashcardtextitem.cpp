#include "flashcardtextitem.h"


 FlashcardTextItem::FlashcardTextItem(QGraphicsItem *parent)
     : QGraphicsTextItem(parent)
 {
     setFlag(QGraphicsItem::ItemIsMovable);
     setFlag(QGraphicsItem::ItemIsSelectable);

     rh = new ResizeHandle(this);
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
