#include "flashcard.h"

Flashcard::Flashcard(qreal x, qreal y, qreal width, qreal height) : QGraphicsRectItem(x, y, width, height)
{
    qDebug() << "Mypos" << this->pos();
    qDebug() << "Myscenepos" << this->scenePos();
    for (int i = 0; i < MAX_TEXT_ITEMS; i++)
    {
        textItems[i] = new FlashcardTextItem(this);
        textItems[i]->hide();
        textItems[i]->setPos(0, 0);
    }
    for (int i = 0; i < MAX_PICTURE_ITEMS; i++)
    {
        picItems[i] = new FlashcardPictureItem(this);
        picItems[i]->hide();
        picItems[i]->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        picItems[i]->setPos(0, 0);
    }

}

void Flashcard::addText(QString text)
{
        qDebug() << "Mypos" << this->pos();
    qDebug() << "Myscenepos" << this->scenePos();
    int openSlot = -1;
    for (int i = 0; i < MAX_TEXT_ITEMS; i++)
    {
        if (!textItems[i]->isVisible())
        {
            openSlot = i;
            break;
        }
    }
    if (openSlot == -1)
    {
        // could not find an open slot!
        // output some sort of error message.
    }
    else
    {
        textItems[openSlot]->setText(text);
        textItems[openSlot]->show();

    }
}

void Flashcard::addPicture(QPixmap pix, double scale)
{
    // search for first open slot
    int openSlot = -1;
    for (int i = 0; i < MAX_PICTURE_ITEMS; i++)
    {
        if (!picItems[i]->isVisible())
        {
            openSlot = i;
            break;
        }
    }
    if (openSlot == -1)
    {
        // could not find an open slot!
        // output some sort of error message.
    }
    else
    {
        picItems[openSlot]->addPic(pix, scale);
        picItems[openSlot]->show();


    }
}
