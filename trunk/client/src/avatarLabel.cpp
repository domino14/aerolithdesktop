#include "avatarLabel.h"

avatarLabel::avatarLabel(QWidget *parent) : QLabel(parent)
{
	left_pressed = false;
	right_pressed = false;
	setFixedWidth(40);
	setFixedHeight(40);
}

void avatarLabel::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		left_pressed = true;
	}
	else if (e->button() == Qt::RightButton)
	{
		right_pressed = true;
	}
}

void avatarLabel::mouseReleaseEvent(QMouseEvent *e)
{
	if (left_pressed && e->button() == Qt::LeftButton && inLabel(e->pos()))
		emit leftMouseClicked();
	else if (right_pressed && e->button() == Qt::RightButton && inLabel(e->pos()))
		emit rightMouseClicked();

	left_pressed = false;
	right_pressed = false;

}

bool avatarLabel::inLabel(const QPoint &p)
{
	return rect().contains(p);
}
