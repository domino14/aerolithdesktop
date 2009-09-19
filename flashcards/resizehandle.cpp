#include "resizehandle.h"

SingleResizeRectangle::SingleResizeRectangle(const QRectF & rect, QGraphicsItem * parent) :
        QGraphicsRectItem(rect, parent)
{

}








/*************************/
ResizeHandle::ResizeHandle(QGraphicsItem * parent)
{
    brHandle = new SingleResizeRectangle(QRect(-4, -4, 8, 8), parent);
    tlHandle = new SingleResizeRectangle(QRect(-4, -4, 8, 8), parent);

    brHandle->setPen(QPen(Qt::black));
    tlHandle->setPen(QPen(Qt::black));

    brHandle->setBrush(QBrush(Qt::green));
    tlHandle->setBrush(QBrush(Qt::green));


}

void ResizeHandle::moveHandles(QRectF boundingRect)
{

    double brPtx, brPty;
    brPtx = boundingRect.x() + boundingRect.width();
    brPty = boundingRect.y() + boundingRect.height();

    brHandle->setRect(brPtx-4, brPty-4, 8, 8);
}
