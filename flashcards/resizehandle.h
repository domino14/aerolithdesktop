#ifndef RESIZEHANDLE_H
#define RESIZEHANDLE_H

#include <QGraphicsRectItem>
#include <QtGui>

class SingleResizeRectangle : public QObject, public QGraphicsRectItem
{
Q_OBJECT
public:
    SingleResizeRectangle( const QRectF & rect, QGraphicsItem * parent = 0);
private:
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent (QGraphicsSceneMouseEvent *event);
signals:
    void movedRectangle(QPointF newPos);
    void releasedMouse();
};


class ResizeHandle : public QObject
{
Q_OBJECT
public:
    ResizeHandle(QGraphicsItem * parent = 0);
    void moveHandles(QRectF);

    SingleResizeRectangle* brHandle;
    SingleResizeRectangle* tlHandle;
signals:
    void movedTLHandle(QPointF newPos);
    void movedBRHandle(QPointF newPos);
    void stoppedMovingHandles();
};

#endif // RESIZEHANDLE_H
