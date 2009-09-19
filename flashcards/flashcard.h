#ifndef FLASHCARD_H
#define FLASHCARD_H

#include <QGraphicsRectItem>
#include "flashcardtextitem.h"
#include "flashcardpictureitem.h"

#define MAX_TEXT_ITEMS 16
#define MAX_PICTURE_ITEMS 16

class Flashcard : public QGraphicsRectItem
{
public:
    Flashcard(qreal, qreal, qreal, qreal);

    void addText(QString);
    void addPicture(QPixmap, double);
private:
   FlashcardTextItem* textItems[MAX_TEXT_ITEMS];
   FlashcardPictureItem* picItems[MAX_PICTURE_ITEMS];
};

#endif // FLASHCARD_H
