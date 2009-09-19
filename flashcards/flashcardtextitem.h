#ifndef FLASHCARDTEXTITEM_H
#define FLASHCARDTEXTITEM_H

#include <QGraphicsTextItem>
#include <QtGui>
#include "resizehandle.h"

class FlashcardTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    FlashcardTextItem(QGraphicsItem *parent = 0);
    void setText(QString);


signals:
    void lostFocus(FlashcardTextItem *item);
    void selectedChange(QGraphicsItem *item);


protected:
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void focusOutEvent(QFocusEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void focusInEvent(QFocusEvent *event);
private:
    ResizeHandle* rh;

};

#endif // FLASHCARDTEXTITEM_H
