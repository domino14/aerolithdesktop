#ifndef PLAYERLISTWIDGETITEM_H
#define PLAYERLISTWIDGETITEM_H

#include <QListWidgetItem>

class PlayerListWidgetItem : public QListWidgetItem
{
public:
    PlayerListWidgetItem();
    void setScore(int);
    void setUsername(QByteArray);
    QString formattedText();
private:
    QByteArray username;
    int score;
    bool operator< ( const PlayerListWidgetItem & other ) const;
};

#endif // PLAYERLISTWIDGETITEM_H
