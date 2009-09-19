#ifndef FLASHCARDPICTUREITEM_H
#define FLASHCARDPICTUREITEM_H

#include <QGraphicsPixmapItem>
#include "resizehandle.h"

class FlashcardPictureItem : public QGraphicsPixmapItem
{
public:
    FlashcardPictureItem(QGraphicsItem *parent = 0);
    void addPic(QPixmap, double);
private:
    double maxHeight, maxWidth;
};

#endif // FLASHCARDPICTUREITEM_H
