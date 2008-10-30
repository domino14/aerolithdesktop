#ifndef _CLIENT_SOCKET_H_
#define _CLIENT_SOCKET_H_

#include <QtCore>
#include <QtNetwork>

class ClientSocket : public QTcpSocket
{
  Q_OBJECT

public:
  struct ConnectionData
  {
    QDataStream in;
    bool loggedIn;
    QString userName;
    quint16 numBytesInPacket;
    quint16 tableNum; // adding here because table number is pretty essential to the workings of this particular server
    quint8 avatarId;
    bool isActive;
    quint16 minutesInactive;
  };

  // this struct will grow for other games. either we will have a separate struct for other games or all in this struct. this is still better than a hash table lookup and having to delete this struct.
  struct PlayerData
  {
    bool readyToPlay;
    bool gaveUp;
    quint16 score;
  };
  

  
  ConnectionData connData;
  PlayerData playerData;




};

#endif
