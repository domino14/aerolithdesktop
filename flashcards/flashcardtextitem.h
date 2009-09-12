#ifndef FLASHCARDTEXTITEM_H
#define FLASHCARDTEXTITEM_H

#include <QGraphicsTextItem>
#include <QtGui>
class FlashcardTextItem : public QGraphicsTextItem
{
    Q_OBJECT
public:
    FlashcardTextItem(QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

signals:
    void lostFocus(FlashcardTextItem *item);
    void selectedChange(QGraphicsItem *item);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void focusOutEvent(QFocusEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
};

#endif // FLASHCARDTEXTITEM_H
