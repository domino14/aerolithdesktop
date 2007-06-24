
#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include <QtNetwork>
#include <QBuffer>
#include <QHash>
#include <QList>

class MainServer : public QTcpServer
{
Q_OBJECT

public: 
 MainServer();
  

private slots:
 void addConnection();
 void removeConnection();
 void receiveMessage();
 
private:
 QList <QTcpSocket*> connections;
 QHash <QTcpSocket*, QBuffer*> buffers;
 quint16 blockSize;
};

#endif
