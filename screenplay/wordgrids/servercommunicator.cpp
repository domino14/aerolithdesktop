#include "servercommunicator.h"

ServerCommunicator::ServerCommunicator(QObject* parent) : QObject(parent)
{
    commsSocket = new QTcpSocket(this);
    connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleError(QAbstractSocket::SocketError)));
    connect(commsSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(commsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

}

bool ServerCommunicator::isConnectedToServer()
{
    return commsSocket->state() == QAbstractSocket::ConnectedState;
}

void ServerCommunicator::readFromServer()
{
    curCommand += commsSocket->readAll();

    if (!curCommand.contains('\n'))
        return;
    bool hasCommand = true;

    while (hasCommand)
    {
        int index = curCommand.indexOf('\n');
        QByteArray thisCmd = curCommand.left(index).trimmed();
        curCommand = curCommand.right(curCommand.size() - index - 1);

        processCommand(thisCmd);


        hasCommand = curCommand.contains('\n');

    }
}

void ServerCommunicator::processCommand(QByteArray cmd)
{
    cmd = cmd.trimmed();
    if (cmd.startsWith("ERROR "))
    {
        emit showError(cmd.mid(6));
    }
    else if (cmd.startsWith("NEWTABLE "))
    {
        emit newTable(cmd.mid(9));
    }
}

void ServerCommunicator::handleError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
        {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            emit serverConnectionError("The host was not found. Please check the "
                                       "host name and port settings.");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            emit serverConnectionError("The connection was refused by the peer. "
                                       "Make sure the Aerolith server is running, "
                                       "and check that the host name and port "
                                       "settings are correct.");
            break;
        default:
            emit serverConnectionError("The following error occurred: " + commsSocket->errorString());
        }

}

void ServerCommunicator::socketConnected()
{
   // here write username to server.
    commsSocket->write("login " + username.toAscii() + "\n");
    emit serverConnect();
}

void ServerCommunicator::socketDisconnected()
{
    emit serverDisconnect();
}

void ServerCommunicator::connectToServer(QString server, int port, QString _username)
{
    username = _username.trimmed();

    commsSocket->abort();
    commsSocket->connectToHost(server, port);
}

void ServerCommunicator::disconnectFromServer()
{
    commsSocket->disconnectFromHost();
}

