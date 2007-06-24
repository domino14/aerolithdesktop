#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include <QTcpServer>

class MainServer : public QTcpServer
{
Q_OBJECT

public: 
 MainServer(QObject *parent = 0);

protected:
 void incomingConnection(int socketDescriptor);
 

};

#endif
