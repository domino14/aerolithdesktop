#include "mainserver.h"
#include <QDateTime>
const int DEFAULT_PORT = 2003;
#define MAX_NUM_TABLES 10000
MainServer::MainServer()
{
    QTcpServer::listen(QHostAddress::Any, DEFAULT_PORT);
    qsrand(QDateTime::currentDateTime().toTime_t());
}

void MainServer::incomingConnection(int socketDescriptor)
{
    ClientSocket* client = new ClientSocket();
    if (client->setSocketDescriptor(socketDescriptor))
    {
        //      connections.append(client);
        client->connectionData.tableNum = 0;
        connect(client, SIGNAL(disconnected()), this, SLOT(removeConnection()));
        connect(client, SIGNAL(readyRead()), this, SLOT(receiveMessage()));
    }
    qDebug() << "Incoming connection: " << client->peerAddress();
}

void MainServer::removeConnection()
{
    qDebug("remove connection");
    ClientSocket* socket = static_cast<ClientSocket*> (sender()); // sender violates modularity
    // but many signals are connected to this slot

    QString username = socket->connectionData.userName;
    if (username != "")
    {
        quint16 tableNum = socket->connectionData.tableNum;
        if (tableNum != 0)
        {
            qDebug() << username << " is at table " << tableNum << " so we have to remove him.";
            removePersonFromTable(socket, tableNum);
        }
        else
            qDebug() << username << " is not at any tables.";

        //        foreach (ClientSocket* connection, connections)
        //            writeToClient(connection, username, S_USERLOGGEDOUT);

        usernamesHash.remove(username);

    }

    connections.removeAll(socket);
    socket->deleteLater();

    qDebug("connection removed");
    qDebug() << " Number of connections: " << connections.size();
    qDebug() << " Number of tables: " << tables.size();
}

void MainServer::receiveMessage()
{

    ClientSocket* socket = static_cast<ClientSocket*> (sender());
    socket->curCommand += socket->readAll();

    if (!socket->curCommand.contains('\n'))
        return;
    bool hasCommand = true;

    while (hasCommand)
    {
        int index = socket->curCommand.indexOf('\n');
        QByteArray thisCmd = socket->curCommand.left(index).trimmed();
        socket->curCommand = socket->curCommand.right(socket->curCommand.size() - index - 1);

        processCommand(socket, thisCmd);


        hasCommand = socket->curCommand.contains('\n');

    }
}

void MainServer::removePersonFromTable(ClientSocket* socket, quint16 tablenum)
{
    QString username = socket->connectionData.userName;
    if (socket->connectionData.tableNum != tablenum)
    {

        qDebug() << "A SERIOUS ERROR OCCURRED " << username << socket->connectionData.tableNum << tablenum;

    }
    // this functions removes the player from the table
    // additionally, if the table is then empty as a result, it deletes the table!
    if (tables.contains(tablenum))
    {
        WordgridsTable *tmp = tables.value(tablenum);
        tmp->removePersonFromTable(socket);

        // write to all connections that username has left table

        foreach (ClientSocket* connection, connections)
            connection->write("LEFTTABLE " + username.toAscii() + " "
                              + QByteArray::number(tablenum) + "\n");

        if (tmp->peopleInTable.size() == 0)
        {
            qDebug() << " need to kill table " << tablenum;
            tables.remove(tablenum);
            tmp->cleanupBeforeDelete();
            tmp->deleteLater(); // delete this table data structure -- this should also delete the tablegame

            // write to all clients that table has ceased to exist!

            foreach (ClientSocket* connection, connections)
            {
                connection->write("KILLTABLE " + QByteArray::number(tablenum) + "\n");
            }

        }

    }


    socket->connectionData.tableNum = 0;

}

void MainServer::processCommand(ClientSocket* socket, QByteArray cmd)
{
    // for right now just echo
    //socket->write(cmd + '\n');

    // client commands are lowercase
    if (cmd == "newtable")
    {
        processNewTable(socket);
    }
    else if (cmd == "leavetable")
    {
        removePersonFromTable(socket, socket->connectionData.tableNum);
    }
    else if (cmd.startsWith("chattable "))
    {
        int tablenum = socket->connectionData.tableNum;
        if (tablenum != 0 && tables.contains(tablenum))
        {
            foreach (ClientSocket* connection, tables.value(tablenum)->peopleInTable)
            {
                connection->write("CHATTABLE " + QByteArray::number(tablenum) +
                                  cmd.mid(10) + "\n");
            }
        }
    }
    else if (cmd.startsWith("acceptpos "))
    {
        int tablenum = socket->connectionData.tableNum;
        if (tablenum != 0 && tables.contains(tablenum))
        {
            QList<QByteArray> params = cmd.split(' ');
            if (params.size() == 6) // xl yl xh yh score bonustile
            {
                int score = tables.value(tablenum)->processMove(socket, params);
                foreach (ClientSocket* connection, tables.value(tablenum)->peopleInTable)
                {
                    connection->write("PLAYERSCORE " + QByteArray::number(tablenum) +
                                      " " + QByteArray::number(score) + " " +
                                      socket->connectionData.userName + "\n");
                }
            }
        }
    }
}

void MainServer::processNewTable(ClientSocket* socket)
{
    //

    quint16 tablenum = 0;

    // check to see if we can actually create a new table.
    bool foundFreeNumber = false;
    bool canCreateTable = true;

    while (!foundFreeNumber && tablenum < MAX_NUM_TABLES)
    {
        tablenum++;
        foundFreeNumber = !tables.contains(tablenum);
    }

    if (!foundFreeNumber)
    {
        canCreateTable = false;
    }

    else if (socket->connData.tableNum != 0)
    {
        canCreateTable = false;
    }

    if (canCreateTable)
    {
        WordgridsTable *tmp = new Table(this);

        tmp->initialize(socket, tablenum);
        tables.insert(tablenum, tmp);

        foreach (ClientSocket* connection, connections)
            connection->write("NEWTABLE " + QByteArray::number(tablenum) +  "\n");
        doJoinTable(socket, tablenum);

    }
}

void MainServer::doJoinTable(ClientSocket* socket, quint16 tablenum)
{
    // this function actually JOINS the table, and not just processes a command from the client.

    // check if table exists, and if it does (which it should), if it's full

    if (!tables.contains(tablenum))
    {
        return;
    }

    WordgridsTable *table = tables.value(tablenum);

    if (socket->connData.tableNum != 0)
    {
        // already in a table
        return;
    }


    // got here with no errors, join table!
    qDebug() << "Ok, join table!";


    foreach (ClientSocket* connection, connections)
        connection->write("JOINTABLE " + socket->connectionData.userName + " "
                          + QByteArray::number(tablenum) + "\n");

    table->personJoined(socket);

}
