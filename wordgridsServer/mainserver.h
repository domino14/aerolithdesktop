#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QTcpServer>
#include "wordgridstable.h"
#include <QTcpSocket>
#include "ClientSocket.h"

class MainServer : public QTcpServer
{
    Q_OBJECT
public:
    MainServer();
private:
    void incomingConnection(int socketDescriptor); // inherited from QTcpServer
    QHash <QString, ClientSocket*> usernamesHash;
    QList <ClientSocket*> connections;
    QHash <quint16, WordgridsTable*> tables;
    void removePersonFromTable(ClientSocket*, quint16);
    void processCommand(ClientSocket*, QByteArray);
    void processNewTable(QByteArray);
    void doJoinTable(ClientSocket* socket, quint16 tablenum);
    QByteArray tableInfoString(quint16 tablenum);
private slots:
    void removeConnection();
    void receiveMessage();
};

#endif // MAINSERVER_H
