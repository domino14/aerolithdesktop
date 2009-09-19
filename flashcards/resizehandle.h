#ifndef RESIZEHANDLE_H
#define RESIZEHANDLE_H

#include <QGraphicsRectItem>
#include <QtGui>

class SingleResizeRectangle : public QGraphicsRectItem
{
    public:
    SingleResizeRectangle( const QRectF & rect, QGraphicsItem * parent = 0);
};


class ResizeHandle
{
public:
    ResizeHandle(QGraphicsItem * parent = 0);
    void moveHandles(QRectF);
private:
    SingleResizeRectangle* brHandle;
    SingleResizeRectangle* tlHandle;
};

#endif // RESIZEHANDLE_H
