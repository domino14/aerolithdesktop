#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QTcpSocket>
class ClientSocket : public QTcpSocket
{
    Q_OBJECT

public:
    struct ConnectionData
    {
        QString userName;
        quint16 tableNum;
        // adding here because table number is pretty essential to the workings of this particular server
    };

    // this struct will grow for other games. either we will have a separate struct for other games or all in this struct. this is still better than a hash table lookup and having to delete this struct.
    struct GameData
    {
        bool readyToPlay;
        quint16 score;
    };
    QByteArray curCommand;
    ConnectionData connectionData;
    GameData gameData;
};
#endif // CLIENTSOCKET_H
