#ifndef _TABLE_H_
#define _TABLE_H_

#include <QtCore>
#include "ClientSocket.h"
#include "commonDefs.h"

class TableGame;


class tableData
{
public:

    enum tablePacketHeaders
    {
        GAME_STARTED, GAME_ENDED, CHAT_SENT, GUESS_RIGHT, TIMER_VALUE, READY_TO_BEGIN, GAVE_UP
            };

    ~tableData();
    QByteArray initialize(ClientSocket* tableCreator, quint16 tableNumber, QByteArray tableDescription);
    quint16 tableNumber;
    quint8 lexiconIndex;
    QByteArray tableInformationArray;
    //QString wordListDescriptor; // todo: change this to table name
    QString tableName;
    QList <ClientSocket*> playerList;
    quint8 maxPlayers;
    bool canJoin;
    TableGame* tableGame;
    ClientSocket *host;

    void sendChatSentPacket(QString, QString);
    void sendGenericPacket();
    void sendTableMessage(QString);
    // anagrams mode?
    quint8 gameType;
    QByteArray letters; // the letters that are showing on the board
    void removePlayerFromTable(ClientSocket*);

};

#endif
