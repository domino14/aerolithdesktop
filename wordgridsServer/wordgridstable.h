#ifndef WORDGRIDSTABLE_H
#define WORDGRIDSTABLE_H

#include <QObject>
#include "ClientSocket.h"
#include <QTimer>
class WordgridsTable : public QObject
{
    Q_OBJECT
public:
    WordgridsTable(QObject* parent);
    void initialize(ClientSocket*, quint16, int, int, bool);
    void removePersonFromTable(ClientSocket*);
    QList <ClientSocket*> peopleInTable;
    void cleanupBeforeDelete();
    void personJoined(ClientSocket*);
    QHash <ClientSocket*, QList<QByteArray> >movesHash;
    int processMove(ClientSocket* socket, QList<QByteArray> params);
private:
    enum GameTimerModes
    {
        MODE_BEGINNING, MODE_INGAME, MODE_BREAK
    };

    quint16 tableNum;
    int boardSize;
    QTimer* gameTimer;
    bool timerModeGame;
    int curTimerValue;
    int gameTimerValue;
    bool allowBonusTiles;
    void sendMessageToTable(QByteArray);
    void sendMessageToPlayer(ClientSocket*, QByteArray);
signals:

private slots:
    void timeout();
};

#endif // WORDGRIDSTABLE_H
