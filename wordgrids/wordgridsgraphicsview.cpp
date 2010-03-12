#include "wordgridsgraphicsview.h"
#include <QKeyEvent>
#include <QtDebug>
WordgridsGraphicsView::WordgridsGraphicsView(QWidget* parent) : QGraphicsView(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    qDebug() << "After setFocusPolicy";
}

//void WordgridsGraphicsView::keyPressEvent ( QKeyEvent * keyEvent )
//{
//    qDebug() << "Key pressed: " << keyEvent->key();
//    emit keyPressed(keyEvent->key());
////    keyEvent->ignore();
//}

