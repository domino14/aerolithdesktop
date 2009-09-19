#include "flashcardpictureitem.h"
#include <QFileDialog>
#include <QtDebug>

FlashcardPictureItem::FlashcardPictureItem(QGraphicsItem *parent) : QGraphicsPixmapItem(parent)
{
    maxHeight = parent->boundingRect().height();
    maxWidth = parent->boundingRect().width();
}


void FlashcardPictureItem::addPic(QPixmap pix, double scaleFactor)
{
    this->setPixmap(pix);
    this->scale(scaleFactor, scaleFactor);

}
