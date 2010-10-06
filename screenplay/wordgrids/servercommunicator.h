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
    void sendJoinTable(int);
    void sendToTable(QByteArray);
    void sendLeaveTable();
    void sendCurrentPosition(int xl, int yl, int xh, int yh, int score, QByteArray bonusTile);
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
    void joinTable(QByteArray);
    void chatTable(QByteArray);
    void leftTable(QByteArray);
    void timerVal(QByteArray);
    void curBoard(QByteArray);
    void playerScore(QByteArray);
    void gameOver(QByteArray);

private slots:
    void readFromServer();
    void handleError(QAbstractSocket::SocketError socketError);
    void socketDisconnected();
    void socketConnected();


};

#endif // SERVERCOMMUNICATOR_H
