#ifndef WORDGRIDSTABLE_H
#define WORDGRIDSTABLE_H

#include <QObject>
#include "ClientSocket.h"
#include <QTimer>
class WordgridsTable : public QObject
{
    Q_OBJECT
public:
    enum GameType
    {
        GAME_STRUCK, GAME_DASH
    };
    WordgridsTable(QObject* parent);
    void initialize(quint16, int, int, GameType);
    void removePersonFromTable(ClientSocket*);
    QList <ClientSocket*> peopleInTable;
    void cleanupBeforeDelete();
    void personJoined(ClientSocket*);
    QHash <ClientSocket*, QList<QByteArray> >movesHash;
    void processMove(ClientSocket* socket, QList<QByteArray> params);
    int boardSize;
    int gameTimerValue;
    GameType gameType;
    int btTurnoff;

private:
    enum GameTimerModes
    {
        MODE_BEGINNING, MODE_INGAME, MODE_BREAK
    };



    QByteArray curBoard;
    quint16 tableNum;

    QTimer* gameTimer;
    GameTimerModes timerModeGame;

    int curTimerValue;


    void sendMessageToTable(QByteArray);
    void sendMessageToPlayer(ClientSocket*, QByteArray);
    void generateAndSendNewBoard();
    void writeScoreToAll(ClientSocket* socket);
    void startGame();
signals:

private slots:
    void timeout();
};

#endif // WORDGRIDSTABLE_H
