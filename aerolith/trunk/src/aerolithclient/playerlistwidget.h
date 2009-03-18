#ifndef _PLAYER_LIST_WIDGET_H_
#define _PLAYER_LIST_WIDGET_H_

#include <QtGui>

class playerListWidget : public QListWidget
{
    Q_OBJECT
public:
    playerListWidget(QWidget* w) : QListWidget(w)
    {
        contextMenu = new QMenu(this);
        contextMenu->addAction("View Profile");
        contextMenu->addAction("Send PM");
        connect(contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(contextMenuAction(QAction*)));
        connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(shouldSendPM(QListWidgetItem*)));
    }

protected:
    void contextMenuEvent(QContextMenuEvent* event)
    {
        if (itemAt(event->pos()) != NULL)// TODO CHECK IF THIS RETURNS NULL!
        {
            setCurrentItem(itemAt(event->pos()));
            contextMenu->popup(event->globalPos());
        }
    }
signals:
    void viewProfile(QString);
    void sendPM(QString);
private:
    QMenu* contextMenu;

private slots:
    void contextMenuAction(QAction* a)
    {
        if (a->text() == "View Profile") emit viewProfile(currentItem()->text());
        if (a->text() == "Send PM") emit sendPM(currentItem()->text());
    }

    void shouldSendPM(QListWidgetItem* i)
    {
        emit sendPM(i->text());

    }
};

#endif
