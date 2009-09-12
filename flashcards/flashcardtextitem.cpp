#include "flashcardtextitem.h"


 FlashcardTextItem::FlashcardTextItem(QGraphicsItem *parent, QGraphicsScene *scene)
     : QGraphicsTextItem(parent, scene)
 {
     setFlag(QGraphicsItem::ItemIsMovable);
     setFlag(QGraphicsItem::ItemIsSelectable);
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

 void FlashcardTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
 {
     if (textInteractionFlags() == Qt::NoTextInteraction)
         setTextInteractionFlags(Qt::TextEditorInteraction);
     QGraphicsTextItem::mouseDoubleClickEvent(event);
 }
