#ifndef _AVATAR_LABEL_H_
#define _AVATAR_LABEL_H_

#include <QtGui>

class avatarLabel : public QLabel
{
    Q_OBJECT

public:
    avatarLabel(QWidget* parent = 0);
signals:
    void leftMouseClicked();
    void rightMouseClicked();

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool left_pressed, right_pressed;
    bool inLabel(const QPoint &p);
};

#endif
