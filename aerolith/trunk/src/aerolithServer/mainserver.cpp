
//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Aerolith is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Aerolith is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Aerolith.  If not, see <http://www.gnu.org/licenses/>.

#include "mainserver.h"
#include <QtDebug>
#include <QtSql>
#include "ClientWriter.h"
#include "UnscrambleGame.h"
#include "commonDefs.h"
#include "databasehandler.h"

//QList <QVariant> dummyList;

extern QByteArray block;
extern QDataStream out;
extern const quint16 MAGIC_NUMBER;

const QString incompatibleVersionString = 
        "You are using an outdated version of the Aerolith client.<BR>\
        Please check <a href=""http://www.aerolith.org"">http://www.aerolith.org</a> for the new client.";
                             const QString compatibleButOutdatedVersionString =
                             "You are using an outdated version of the Aerolith client. However, this version will work with the current server, but you will be missing new features. If you would like to upgrade, please check <a href=""http://www.aerolith.org"">http://www.aerolith.org</a> for the new client.";
//const QString thisVersion = "0.4.1";

const quint32 userDailyByteLimit = 2000000;

#define MAX_NUM_TABLES 65535


MainServer::MainServer(QString aerolithVersion, DatabaseHandler* databaseHandler) :
        aerolithVersion(aerolithVersion), dbHandler(databaseHandler)
{
    qDebug("mainserver constructor");

    // connect to existing databases

    QStringList dbList = dbHandler->checkForDatabases();
    dbHandler->connectToDatabases(false, dbList);
    

    QTime midnight(0, 0, 0);
    qsrand(midnight.msecsTo(QTime::currentTime()));
    initializeWriter();
    //  wordDb = QSqlDatabase::addDatabase("QSQLITE");
    //wordDb.setDatabaseName(QDir::homePath() + "/.zyzzyva/lexicons/OWL2+LWL.db");
    //wordDb.open();
    oneMinuteTimer = new QTimer;
    connect(oneMinuteTimer, SIGNAL(timeout()), this, SLOT(checkEveryone()));
    oneMinuteTimer->start(60000);

    midnightTimer = new QTimer;
    connect(midnightTimer, SIGNAL(timeout()), this, SLOT(midnightUpkeep()));

    midnightTimer->start(86400000 + QTime::currentTime().msecsTo(midnight));
    qDebug() << "there are" << 86400000 + QTime::currentTime().msecsTo(midnight) << "msecs to midnight.";
    // but still generate daily challenges right now.
    UnscrambleGame::loadWordLists(dbHandler);
    UnscrambleGame::generateDailyChallenges(dbHandler);
    UnscrambleGame::midnightSwitchoverToggle = true;


    if (QFile::exists("users.db"))
    {
        userDb = QSqlDatabase::addDatabase("QSQLITE", "usersDB");
        userDb.setDatabaseName("users.db");
        userDb.open();
    }
    else
    {
        userDb = QSqlDatabase::addDatabase("QSQLITE", "usersDB");
        userDb.setDatabaseName("users.db");
        userDb.open();
        QSqlQuery query(QSqlDatabase::database("usersDB"));
        query.exec("CREATE TABLE IF NOT EXISTS users(username VARCHAR(16), password VARCHAR(16), avatar INTEGER, "
                   "profile VARCHAR(1000), registeredIP VARCHAR(16), lastIP VARCHAR(16), lastLoggedOut VARCHAR(32), email VARCHAR(40), "
                   "points INTEGER, picture BLOB, playerID INTEGER, coins INTEGER, saveData BLOB)");
        query.exec("CREATE UNIQUE INDEX IF NOT EXISTS usernameIndex on users(username)");
        query.exec("CREATE TABLE IF NOT EXISTS playerID_table(playerID INTEGER)");
        query.exec("INSERT INTO playerID_table(playerID) VALUES(1)");
    }
}

void MainServer::midnightUpkeep()
{
    /* clear today's blacklist */
    todaysBlacklist.clear();
    todaysBandwidthByUser.clear();

    /* set bytes sent today to 0 */

    writeHeaderData();
    out << (quint8) SERVER_RESET_TODAYS_BANDWIDTH;
    fixHeaderLength();

    foreach (ClientSocket* socket, connections)
    {
        socket->connData.numBytesSentToday = 0;
        socket->write(block);
    }

    /* restart midnight timer */
    QTime midnight(0, 0, 0);
    midnightTimer->stop();
    midnightTimer->start(86400000 + QTime::currentTime().msecsTo(midnight));

    /* handle daily challenges for Unscramble game */
    UnscrambleGame::generateDailyChallenges(dbHandler);

}

void MainServer::incomingConnection(int socketDescriptor)
{
    ClientSocket *client = new ClientSocket();
    if (client->setSocketDescriptor(socketDescriptor))
    {
        //      connections.append(client);
        client->connData.loggedIn = false;
        client->connData.numBytesInPacket = 0;
        client->connData.in.setDevice(client);
        client->connData.in.setVersion(QDataStream::Qt_4_2);
        client->connData.tableNum = 0;
        client->connData.isSitting = false;
        client->connData.avatarId = 1;
        client->connData.isActive = true; // assume we responded to the last ping
        client->connData.minutesInactive = 0;

        connect(client, SIGNAL(disconnected()), this, SLOT(removeConnection()));
        connect(client, SIGNAL(readyRead()), this, SLOT(receiveMessage()));

    }
    qDebug() << "Incoming connection: " << client->peerAddress();

}

void MainServer::checkEveryone()
{
    qDebug() << "Called checkEveryone";
    /* if (connections.size() > 0)
    {
      writeHeaderData();      
      out << (quint8) SERVER_PING; // keep alive
      fixHeaderLength();

      foreach (ClientSocket* socket, connections)
        {
          // disconnect if did not respond to last ping
          if (socket->connData.isActive == false)
            {
              qDebug() << socket->connData.userName << "lagged out!";
              socket->disconnectFromHost();
            }
          else
            {

              qint64 retval = socket->write(block);
              socket->flush(); // do we really need this???
              qDebug() << "Pinged " << socket->connData.userName << "ret:" << retval;
              if (retval == -1) qDebug() << socket->errorString();
              socket->connData.isActive = false;
            }
        }
    }
  */
    foreach (ClientSocket* socket, connections)
    {
        socket->connData.minutesInactive++;
        if (socket->connData.minutesInactive >= 60)
        {
            writeToClient(socket, "Kicked for inactivity!", S_ERROR);
            socket->disconnectFromHost();
        }

    }


}

void MainServer::removeConnection()
{
    qDebug("remove connection");
    ClientSocket* socket = static_cast<ClientSocket*> (sender()); // sender violates modularity
    // but many signals are connected to this slot
    //  connectionData* connData = connectionParameters.value(socket);
    QString username = socket->connData.userName;
    if (username != "")
    {
        //      quint16 tablenum = playerDataHash.value(username).tablenum;
        quint16 tableNum = socket->connData.tableNum;
        if (tableNum != 0)
        {
            qDebug() << username << " is at table " << tableNum << " so we have to remove him.";
            removePersonFromTable(socket, tableNum);
        }
        else
            qDebug() << username << " is not at any tables.";

        foreach (ClientSocket* connection, connections)
            writeToClient(connection, username, S_USERLOGGEDOUT);

        usernamesHash.remove(username);

        QSqlQuery query(QSqlDatabase::database("usersDB"));
        query.prepare("UPDATE users SET avatar = :avatar, lastIP = :lastIP, lastLoggedOut = :lastLoggedOut "
                      "WHERE username = :username");
        query.bindValue(":avatar", socket->connData.avatarId);
        query.bindValue(":username", socket->connData.userName.toLower());
        query.bindValue(":lastIP", socket->peerAddress().toString());
        query.bindValue(":lastLoggedOut", QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss"));

        query.exec();


    }
    // will add the user to the hash if it doesn't exist
    todaysBandwidthByUser[socket->connData.userName.toLower()] = socket->connData.numBytesSentToday;

    connections.removeAll(socket);
    socket->deleteLater();

    qDebug("connection removed");
    qDebug() << " Number of connections: " << connections.size();
    qDebug() << " Number of tables: " << tables.size();
}

void MainServer::receiveMessage()
{

    // this seems like it'll be slow, but it'll do for now
    ClientSocket* socket = static_cast<ClientSocket*> (sender());

    if (socket->bytesAvailable() > 16500)
    {
        qDebug() << "too many available bytes.";
        socket->disconnectFromHost();
        return;
    }

    while (socket->bytesAvailable()> 0)
    {
        //QDataStream needs EVERY byte to be available for reading!
        if (socket->connData.numBytesInPacket == 0)
        {
            if (socket->bytesAvailable() < 4)
                return;

            quint16 header;
            quint16 packetlength;

            // there are 4 bytes available. read them in!
            socket->connData.in >> header;
            socket->connData.in >> packetlength;
            // TODO THIS IS OPEN TO ABUSE.  what if the user sends a packet thats > 2^16 bytes? packetlength will wrap around,
            // and reader won't read all bytes
            // or if user sends 20 bytes e.g. and packetlength = 10
            socket->connData.numBytesInPacket = packetlength;
            if (header != (quint16)MAGIC_NUMBER) // magic number
            {
                qDebug() << " wrong magic number: " << header << " packlength " << packetlength;
                writeToClient(socket, incompatibleVersionString, S_ERROR);
                socket->disconnectFromHost();
                return;
            }
        }


        if (socket->bytesAvailable() < socket->connData.numBytesInPacket)
            return;

        socket->connData.numBytesSentToday += socket->connData.numBytesInPacket;

        if (socket->connData.numBytesSentToday > userDailyByteLimit)
        {
            writeToClient(socket, "You have reached your daily bandwidth limit! Please quiz with large files "
                          "locally!", S_ERROR); // the user daily limit should happen very rarely.
            todaysBlacklist.insert(socket->connData.userName.toLower());
            socket->disconnectFromHost();

        }

        // ok, we can now read the WHOLE packet
        // ideally we read the 'case' byte right now, and then call a process function with
        // 'in' (the QDataStream) as a parameter!
        // the process function will set connData->numBytesInPacket to 0 at the end
        quint8 packetType;
        socket->connData.in >> packetType; // this is the case byte!
        qDebug() << "Received from sender " << socket->connData.userName << " packet " << (char)packetType <<
                socket->connData.numBytesInPacket;
        socket->connData.minutesInactive = 0;
        if (socket->connData.numBytesInPacket > 10000)
        {
            socket->disconnectFromHost();
            return;
        }

        if (packetType != CLIENT_LOGIN && !socket->connData.loggedIn)
        {
            socket->disconnectFromHost();
            return; // cannot do anything before logging in!
        }

        switch(packetType)
        {
        case CLIENT_PONG:
            //	  socket->connData.isActive = true;
            qDebug() << "PONG" << socket->connData.userName;
            break;

        case CLIENT_CHAT_ACTION:
            processChatAction(socket);
            break;
        case CLIENT_LOGIN:
            // entered
            processLogin(socket);
            break;
            /*    case 'l':
          // left
          processLogout(socket, connData);
          break;*/ // there is no 'left' packet because server writes it.
        case CLIENT_CHAT:
            // chat
            processChat(socket);
            break;
        case CLIENT_PM:
            // private message
            processPrivateMessage(socket);
            break;
        case CLIENT_NEW_TABLE:
            // created a new table
            processNewTable(socket);
            break;
        case CLIENT_VERSION:
            processVersionNumber(socket);
            break;
        case CLIENT_JOIN_TABLE:
            // joined an existing table
            processJoinTable(socket);
            break;
        case CLIENT_LEAVE_TABLE:
            processLeftTable(socket);
            break;
        case CLIENT_TABLE_COMMAND:
            processTableCommand(socket);
            break;
        case CLIENT_HIGH_SCORE_REQUEST:
            sendHighScores(socket);
            break;

        case CLIENT_REGISTER:
            registerNewName(socket);
            break;


        default:
            socket->disconnectFromHost(); // possibly a malicious packet
            return;
        }

        socket->connData.numBytesInPacket = 0;

    }
}


void MainServer::sendHighScores(ClientSocket* socket)
{
    // sends high scores for unscramble game
    QString challengeName;

    socket->connData.in >> challengeName;
    if (UnscrambleGame::challenges.contains(challengeName))
    {
        //      highScoreData *tmp = &UnscrambleGame::challenges.value(challengeName).highScores;

        if (UnscrambleGame::challenges.value(challengeName).highScores->size() < 0) return;

        QList <highScoreData> tmpList = UnscrambleGame::challenges.value(challengeName).highScores->values();

        if (tmpList.size() == 0) return;
        writeHeaderData();
        out << (quint8) SERVER_HIGH_SCORES; // high scores
        out << challengeName;
        out << tmpList.at(0).numSolutions;
        out << (quint16) tmpList.size();
        qDebug() << challengeName << "#Sol" << tmpList.at(0).numSolutions << "#players" << tmpList.size();
        for (int i = 0; i < tmpList.size(); i++)
        {
            out << tmpList.at(i).userName;
            out << tmpList.at(i).numCorrect;
            out << tmpList.at(i).timeRemaining;
            qDebug() << tmpList.at(i).userName << tmpList.at(i).numCorrect << tmpList.at(i).timeRemaining;
        }

        // 40 bytes per score
        // max ppl allowed = ~1000 or more.

        fixHeaderLength();
        socket->write(block);
    }

}



void MainServer::processChatAction(ClientSocket* socket)
{
    QString username, actionText;
    // i.e. cesar knocks bbstenniz over the head
    username = socket->connData.userName;
    socket->connData.in >> actionText;
    if (actionText.length() > 400)
    {
        socket->disconnectFromHost();
        return;
    }
    foreach (ClientSocket* connection, connections)
        writeToClient(connection, "* " +username + " " + actionText, S_SERVERMESSAGE);
}

void MainServer::processVersionNumber(ClientSocket* socket)
{
    QString version;
    socket->connData.in >> version;

    if (version != aerolithVersion)
        writeToClient(socket, compatibleButOutdatedVersionString , S_SERVERMESSAGE);
}

void MainServer::processTableCommand(ClientSocket* socket)
{
    quint16 tablenum;
    socket->connData.in >> tablenum;
    quint8 subcommand;
    socket->connData.in >> subcommand;

    if (!tables.contains(tablenum))
    {
        qDebug() << "this table does not exist! Return!";
        socket->disconnectFromHost();
        return;

    }
    Table* table = tables.value(tablenum);

    switch (subcommand)
    {
    case CLIENT_TABLE_READY_BEGIN:
        table->tableGame->gameStartRequest(socket);

        break;
        //    case CLIENT_TABLE_GUESS:
        //        // guess from solution box
        //        {
        //            QByteArray guess;
        //            socket->connData.in >> guess;
        //            table->tableGame->guessSent(socket, guess);
        //        }
        //        break;
    case CLIENT_TABLE_CHAT:
        // chat
        {
            QString chat;
            socket->connData.in >> chat;
            if (chat == "/reload" && socket->connData.userName == "cesar")
                UnscrambleGame::loadWordLists(dbHandler);
            if (chat == "/goingdown" && socket->connData.userName == "cesar")
            {
                foreach (ClientSocket* connection, connections)
                    writeToClient(connection, "The server is being restarted in 3 minutes!",  S_SERVERMESSAGE);
            }

            if (chat.length() > 1000)
            {
                socket->disconnectFromHost();
                return;
            }

            qDebug() << " ->" << tablenum << socket->connData.userName << chat;
            table->sendChatSentPacket(socket->connData.userName, chat);
        }
        break;
    case CLIENT_TABLE_GIVEUP:
        // uncle

        // user gave up.
        table->tableGame->gameEndRequest(socket);

        break;
    case CLIENT_TRY_SITTING:
        {
            quint8 seatNumber;
            socket->connData.in >> seatNumber;
            table->trySitting(socket, seatNumber);
        }
        break;
    case CLIENT_TRY_STANDING:
        {
            table->tryStanding(socket);
        }
        break;
    case CLIENT_TABLE_ACTION:
        {
            QString username, actionText;
            // i.e. cesar knocks bbstenniz over the head
            username = socket->connData.userName;
            socket->connData.in >> actionText;
            if (actionText.length() > 400)
            {
                socket->disconnectFromHost();
                return;
            }
            table->sendTableMessage("* " + username + " " + actionText);
        }
        break;
    case CLIENT_TABLE_PRIVACY:
        {
            bool privacy;
            socket->connData.in >> privacy;

            bool success = table->setTablePrivacy(socket, privacy);

            if (success)
            {
                // need to tell everyone that this table is now private or public.
                writeHeaderData();
                out << (quint8) SERVER_TABLE_PRIVACY;
                out << tablenum;
                out << privacy;
                fixHeaderLength();

                foreach (ClientSocket* connection, connections)
                    connection->write(block);
            }

        }
        break;
    case CLIENT_TABLE_AVATAR:
        // avatar ID
        {

            quint8 avatarID;
            socket->connData.in >> avatarID;

            table->processAvatarID(socket, avatarID);

        }

        break;
    case CLIENT_TABLE_INVITE:
        {
            QString username;
            socket->connData.in >> username;

            // to avoid spoofed invite/boot packets
            if (socket->connData.userName == table->host->connData.userName && usernamesHash.contains(username))
            {
                table->inviteList.insert(usernamesHash.value(username));
                writeHeaderData();
                out << (quint8) SERVER_INVITE_TO_TABLE;
                out << tablenum;
                out << socket->connData.userName;
                fixHeaderLength();

                usernamesHash.value(username)->write(block);
            }

        }
        break;

    case CLIENT_TABLE_BOOT:
        {
            QString username;
            socket->connData.in >> username;
            // avoid spoofed invite/boot packets. only the host can boot or invite.
            if (socket->connData.userName == table->host->connData.userName && usernamesHash.contains(username))
            {
                writeHeaderData();
                out << (quint8) SERVER_BOOT_FROM_TABLE;
                out << tablenum;
                out << socket->connData.userName;
                fixHeaderLength();
                usernamesHash.value(username)->write(block);

                // also kick the person out of the table
                removePersonFromTable(usernamesHash.value(username), tablenum);
            }

        }
        break;
    default:
        table->tableGame->handleMiscPacket(socket, subcommand);
        //socket->disconnectFromHost();
    }

}

void MainServer::processLeftTable(ClientSocket* socket)
{
    quint16 tablenum;
    socket->connData.in >> tablenum;

    // the table checking/deletion process must also be done on disconnection!!!

    removePersonFromTable(socket,  tablenum);


}

void MainServer::removePersonFromTable(ClientSocket* socket, quint16 tablenum)
{
    QString username = socket->connData.userName;
    if (socket->connData.tableNum != tablenum)
    {

        qDebug() << "A SERIOUS ERROR OCCURRED " << username << socket->connData.tableNum << tablenum;

    }
    // this functions removes the player from the table
    // additionally, if the table is then empty as a result, it deletes the table!
    if (tables.contains(tablenum))
    {
        Table *tmp = tables.value(tablenum);
        tmp->removePersonFromTable(socket);




        // write to all connections that username has left table  
        writeHeaderData();
        out << (quint8) SERVER_LEFT_TABLE;
        out << tablenum;
        out << username;
        fixHeaderLength();

        foreach (ClientSocket* connection, connections)
            connection->write(block);

        qDebug() << "wrote " << username << " left " << tablenum;
        if (tmp->peopleInTable.size() == 0)
        {
            qDebug() << " need to kill table " << tablenum;
            tables.remove(tablenum);
            tmp->cleanupBeforeDelete();
            delete tmp; // delete this table data structure -- this should also delete the tablegame

            // write to all clients that table has ceased to exist!
            writeHeaderData();
            out << (quint8) SERVER_KILL_TABLE; // kill table
            out << tablenum;
            fixHeaderLength();

            foreach (ClientSocket* connection, connections)
            {
                connection->write(block);
                qDebug() << "wrote that we Killed table " << tablenum << " to " << connection->connData.userName;
            }

        }
        else
        {
            foreach (ClientSocket* thisConn, tmp->peopleInTable)
                thisConn->playerData.readyToPlay = false;
            // TODO: actually send out readytoplay false
        }
    } 
    else
    {
        // an error that shouldn't happen.
        writeToClient(socket, "Error leaving table!", S_ERROR);
        return;

    }

    socket->connData.tableNum = 0;
    socket->connData.isSitting = false;
    socket->playerData.readyToPlay = false;
    socket->playerData.gaveUp = false;
}

void MainServer::processNewTable(ClientSocket* socket)
{
    //

    quint16 tablenum = 0;
    // read in all the data, and let the table class parse it. this is because the table create
    // packet looks different for different games.
    //   int tableDescriptionSize = socket->connData.numBytesInPacket-sizeof(quint8);
    //    char* newTableBytes = new char[tableDescriptionSize];
    //    socket->connData.in.readRawData(newTableBytes, tableDescriptionSize);

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
        writeToClient(socket, "You cannot create any more tables!", S_ERROR);
        canCreateTable = false;
    }

    else if (socket->connData.loggedIn == false)
    {
        qDebug() << "new table? wait.. still logging in!";
        canCreateTable = false;
    }

    else if (socket->connData.tableNum != 0)
    {
        writeToClient(socket, "Please leave the table you are in before creating another table.", S_ERROR);
        canCreateTable = false;
        socket->connData.in.skipRawData(socket->connData.numBytesInPacket-sizeof(quint8));
        // read and ignore table creation bytes
    }

    if (canCreateTable)
    {
        Table *tmp = new Table;
        //   QByteArray tableDescription = QByteArray::fromRawData(newTableBytes, tableDescriptionSize);
        // does not do a deep copy!
        
        QByteArray tableBlock = tmp->initialize(socket, tablenum, dbHandler);
        tables.insert(tablenum, tmp);

        foreach (ClientSocket* connection, connections)
            connection->write(tableBlock);
        doJoinTable(socket, tablenum);
    }
    //   delete [] newTableBytes;



}

void MainServer::doJoinTable(ClientSocket* socket, quint16 tablenum)
{
    // this function actually JOINS the table, and not just processes a command from the client.

    // check if table exists, and if it does (which it should), if it's full

    if (!tables.contains(tablenum))
    {
        writeToClient(socket, "That table doesn't exist!", S_ERROR);
        return;
    }

    Table *table = tables.value(tablenum);
    if (!table->canJoin(socket))
    {
        writeToClient(socket, "You can not join this table!", S_ERROR);
        return;
    }

    if (socket->connData.tableNum != 0)
    {
        writeToClient(socket, "You are already in a table!", S_ERROR);
        return;
    }

    if (socket->connData.loggedIn == false)
    {
        qDebug() << "join? wait.. still logging in!";
        return;
    }

    // got here with no errors, join table!
    qDebug() << "Ok, join table!";

    writeHeaderData();
    out << (quint8) SERVER_JOIN_TABLE;
    out << (quint16) tablenum;
    out << socket->connData.userName;
    fixHeaderLength();
    foreach (ClientSocket* connection, connections)
        connection->write(block);

    table->personJoined(socket);

}

void MainServer::processJoinTable(ClientSocket* socket)
{
    // processes a join table from the client
    quint16 tablenum;
    socket->connData.in >> tablenum;


    doJoinTable(socket, tablenum);
}

void MainServer::processPrivateMessage(ClientSocket* socket)
{
    QString username, message;
    socket->connData.in >> username >> message;
    if (username.length() + message.length() > 400)
    {
        socket->disconnectFromHost();
        return;
    }

    if (usernamesHash.find(username) != usernamesHash.end())
    {
        // the username exists
        ClientSocket* connection = usernamesHash.value(username); // receiver
        writeHeaderData();
        out << (quint8) SERVER_PM;
        out << socket->connData.userName; // sender of message
        out << message;
        fixHeaderLength();
        connection->write(block);
    }


}

void MainServer::registerNewName(ClientSocket* socket)
{
    QString username, password;
    socket->connData.in >> username >> password;
    if (!isValidUsername(username))
    {

        writeToClient(socket, "That is an invalid username.", S_ERROR);
        socket->disconnectFromHost();
        return;
    }
    if (!isValidPassword(password))
    {
        writeToClient(socket, "That password is invalid. Use only alphanumeric characters, and the password must be between 4 and 16 characters long.", S_ERROR);
        socket->disconnectFromHost();
        return;
    }

    QSqlQuery query(QSqlDatabase::database("usersDB"));
    query.exec("SELECT username from users where username = '" + username.toLower() + "'");
    if (query.next())
    {
        // username exists
        writeToClient(socket, "That username already exists in the database. Please select another username.", S_ERROR);
        socket->disconnectFromHost();
        return;
    }


    else
    {
        int playerID;
        query.exec("SELECT playerID from playerID_table");
        while (query.next())
        {
            playerID = query.value(0).toInt();
        }
        QString toExecute;
        toExecute = "INSERT INTO users(username, password, avatar, registeredIP, playerID) VALUES(:username, :password, :avatar, :registeredIP, :playerID)";
        query.prepare(toExecute);
        query.bindValue(":username", username.toLower());
        query.bindValue(":password", password);
        query.bindValue(":avatar", 1);
        query.bindValue(":registeredIP", socket->peerAddress().toString());
        query.bindValue(":playerID", playerID);
        query.exec();
        writeToClient(socket, "The username " + username + " was successfully registered! Please click ""Login Screen"" and connect using this username and password.", S_ERROR);
        socket->disconnectFromHost();

        // increase id by 1
        playerID++;
        query.exec(QString("UPDATE playerID_table SET playerID = %1").arg(playerID));
        return;
    }

}

void MainServer::processLogin(ClientSocket* socket)
{

    QString username, password;
    socket->connData.in >> username >> password;
    qDebug() << "Login: " << username << password;

    if (socket->connData.loggedIn == true)
    {
        qDebug() << "WEIRD";
        writeToClient(socket, "You are already logged in!", S_ERROR);
        socket->disconnectFromHost();
        return;
    }

    if (!isValidUsername(username))
    {
        writeToClient(socket, "That is an invalid username.", S_ERROR);
        socket->disconnectFromHost();
        return;
    }
    if (!isValidPassword(password))
    {
        writeToClient(socket, "Your password is invalid!", S_ERROR);
        socket->disconnectFromHost();
        return;
    }

    // now check the database

    QSqlQuery query(QSqlDatabase::database("usersDB"));
    query.exec("SELECT * from users where username = '" + username.toLower() + "'");

    if (!query.next())
    {
        writeToClient(socket, "Your username does not appear in the database. "
                      "Please click Register, and follow the instructions. "
                      , S_ERROR);
        socket->disconnectFromHost();
        return;
    }
    else
    {
        QString comparePW = query.value(1).toString();
        if (comparePW != password)
        {
            writeToClient(socket, "Your password is incorrect!", S_ERROR);
            socket->disconnectFromHost();
            return;
        }
    }

    foreach (QString thisUsername, usernamesHash.keys())
    {
        if (thisUsername.toLower() == username.toLower())
        {
            qDebug() << "was already logged in!";
            writeToClient(socket, "It appears that you were already logged in... Your previous connection has been logged out! Please try again.", S_ERROR);

            usernamesHash.value(thisUsername)->disconnectFromHost();
            socket->disconnectFromHost();
            return;

        }
    }

    /*
  if (usernamesHash.contains(username))
    {


    }

  */


    // got here with no error

    if (todaysBlacklist.contains(username.toLower()))
    {
        writeToClient(socket, "You cannot log in right now. Please play locally, and try again tomorrow." ,S_ERROR);
        socket->disconnectFromHost();
        return;
    }

    if (todaysBandwidthByUser.contains(username.toLower()))
        socket->connData.numBytesSentToday = todaysBandwidthByUser.value(username.toLower());
    else
        socket->connData.numBytesSentToday = 0;

    writeHeaderData();
    out << (quint8) SERVER_MAX_BANDWIDTH;
    out << userDailyByteLimit;
    fixHeaderLength();
    socket->write(block);


    connections.append(socket);

    int avatarID = query.value(2).toInt();
    socket->connData.avatarId = avatarID;

    usernamesHash.insert(username, socket);
    socket->connData.loggedIn = true;
    socket->connData.userName = username;
    foreach (ClientSocket* connection, connections)
    {
        // write to each socket that "username" has logged in
        writeToClient(connection, username, S_USERLOGGEDIN);
        if (connection != socket)
        {
            // write to THIS socket that every other username has logged in.
            writeToClient(socket, connection->connData.userName, S_USERLOGGEDIN);
        }
    }
    socket->playerData.readyToPlay = false;
    socket->playerData.gaveUp = false;
    socket->playerData.score = 0;

    // send game-specific data
    UnscrambleGame::sendLists(socket);


    // finally, send tables
    QList <Table*> tableList= tables.values();

    foreach(Table* table, tableList)
    {
        socket->write(table->tableInformationArray);

        foreach(ClientSocket* thisSocket, table->peopleInTable)
        {
            writeHeaderData();
            out << (quint8) SERVER_JOIN_TABLE;
            out << (quint16) table->tableNumber;
            out << thisSocket->connData.userName;
            fixHeaderLength();
            socket->write(block);
        }

    }




}

void MainServer::processChat(ClientSocket* socket)
{
    QString username = socket->connData.userName;
    QString chattext;
    socket->connData.in >> chattext;


    if (chattext.length() > 400)
    {
        socket->disconnectFromHost();
        return;
    }

    writeHeaderData();
    out << (quint8) SERVER_CHAT;
    out << username;
    out << chattext;
    fixHeaderLength();
    foreach (ClientSocket* connection, connections)
        connection->write(block);

    qDebug() <<" ->" << username << ":" << chattext;

}

void MainServer::writeToClient(ClientSocket* socket, QString parameter, packetHeaderStatesEnum type)
{
    writeHeaderData();
    QString debugstring;
    switch (type)
    {
    case S_USERLOGGEDIN:

        out << (quint8) SERVER_LOGGED_IN;
        out << parameter;
        debugstring = "logged in.";
        break;
    case S_ERROR:
        out << (quint8) SERVER_ERROR;
        out << parameter;
        debugstring = "error.";
        break;
    case S_USERLOGGEDOUT:
        out << (quint8) SERVER_LOGGED_OUT;
        out << parameter;
        debugstring = "logged out.";
        break;
    case S_SERVERMESSAGE:
        out << (quint8) SERVER_MESSAGE;
        out << parameter;
        break;
    }
    fixHeaderLength();
    socket->write(block);

    //  qDebug() << " wrote to " << connectionParameters.value(socket)->username << " that " << parameter << debugstring;
} 



bool MainServer::isValidUsername(QString username)
{


    if (username.length() > 16 || username.length() < 1) return false;

    char ch = username.at(0).toLatin1();
    if (! ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')))
        return false;

    for (int i = 1; i < username.length(); i++)
    {
        char ch = username.at(i).toLatin1();
        qDebug() << "testing" << ch;
        if ( !((ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z') ||
               (ch >= '0' && ch <= '9') ||
               (ch == '.')))
            return false;

    }

    return true;
}

bool MainServer::isValidPassword(QString password)
{
    if (password.length() > 16 || password.length() < 4) return false;

    for (int i = 0; i < password.length(); i++)
    {
        char ch = password.at(i).toLatin1();
        qDebug() << "testing" << ch;
        if ( !((ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z') ||
               (ch >= '0' && ch <= '9') ||
               (ch == '.')))
            return false;

    }

    return true;
}



