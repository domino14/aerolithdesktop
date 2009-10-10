#include "resizehandle.h"

SingleResizeRectangle::SingleResizeRectangle(const QRectF & rect, QGraphicsItem * parent) :
        QGraphicsRectItem(rect, parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);


}

void SingleResizeRectangle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsRectItem::mouseMoveEvent(event);
    emit movedRectangle(event->pos());
}

void SingleResizeRectangle::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsRectItem::mouseReleaseEvent(event);
    emit releasedMouse();
}

/*************************/
ResizeHandle::ResizeHandle(QGraphicsItem * parent)
{
    brHandle = new SingleResizeRectangle(QRect(-4, -4, 8, 8));
    tlHandle = new SingleResizeRectangle(QRect(-4, -4, 8, 8));

    brHandle->setPen(QPen(Qt::black));
    tlHandle->setPen(QPen(Qt::black));

    brHandle->setBrush(QBrush(Qt::green));
    tlHandle->setBrush(QBrush(Qt::green));

    connect(brHandle, SIGNAL(movedRectangle(QPointF)), this, SIGNAL(movedBRHandle(QPointF)));
    connect(tlHandle, SIGNAL(movedRectangle(QPointF)), this, SIGNAL(movedTLHandle(QPointF)));

    connect(brHandle, SIGNAL(releasedMouse()), this, SIGNAL(stoppedMovingHandles()));
    connect(tlHandle, SIGNAL(releasedMouse()), this, SIGNAL(stoppedMovingHandles()));

    showHandles(false);

}

ResizeHandle::~ResizeHandle()
{
    brHandle->deleteLater();
    tlHandle->deleteLater();
}

void ResizeHandle::showHandles(bool s)
{
    if (s)
    {
        brHandle->show();
        tlHandle->show();
    }
    else
    {
        brHandle->hide();
        tlHandle->hide();
    }
}

void ResizeHandle::moveHandles(double tlX, double tlY, double brX, double brY)
{
    tlHandle->setPos(tlX, tlY);
    brHandle->setPos(brX, brY);
}

