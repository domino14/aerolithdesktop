#ifndef _SERVERTHREAD_H_
#define _SERVERTHREAD_H_

#include <QTcpSocket>
#include <QThread>

class ServerThread : public QThread
{
Q_OBJECT


public:

  ServerThread(int socketDescriptor, QObject *parent);
  void run(); 
 
signals:
  void error(QTcpSocket::SocketError socketError);

private:
  int socketDescriptor;

};

#endif
