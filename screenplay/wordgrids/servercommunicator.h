#ifndef SERVERCOMMUNICATOR_H
#define SERVERCOMMUNICATOR_H

#include <QObject>
#include <QTcpSocket>
class ServerCommunicator : public QObject
{
    Q_OBJECT
public:
    ServerCommunicator(QObject*);
    bool isConnectedToServer();
    void connectToServer(QString server, int port, QString _username);
    void disconnectFromServer();
private:
    QTcpSocket *commsSocket;
    QString username;
    QByteArray curCommand;
    void processCommand(QByteArray cmd);
signals:
    void serverConnect();
    void serverDisconnect();
    void serverConnectionError(QString);
    void showError(QString);
    void newTable(QByteArray);
private slots:
    void readFromServer();
    void handleError(QAbstractSocket::SocketError socketError);
    void socketDisconnected();
    void socketConnected();

};

#endif // SERVERCOMMUNICATOR_H
