#include "playerlistwidgetitem.h"

PlayerListWidgetItem::PlayerListWidgetItem()
{
}

QString PlayerListWidgetItem::formattedText()
{
    return (QString("(%1) %2").arg(score).arg(QString(username)));
}

void PlayerListWidgetItem::setUsername(QByteArray u)
{
    username = u;
}

void PlayerListWidgetItem::setScore(int s)
{
    score = s;
}

bool PlayerListWidgetItem::operator< ( const PlayerListWidgetItem & other ) const
{
    return this->score < other.score;
}
