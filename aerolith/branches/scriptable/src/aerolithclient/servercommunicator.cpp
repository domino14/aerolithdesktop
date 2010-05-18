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

#include "servercommunicator.h"


const quint16 MAGIC_NUMBER = 25349;

bool highScoresLessThan(const tempHighScoresStruct& a, const tempHighScoresStruct& b)
{
    if (a.numCorrect == b.numCorrect) return (a.timeRemaining > b.timeRemaining);
    else return (a.numCorrect > b.numCorrect);
}

ServerCommunicator::ServerCommunicator(QObject* parent) : QObject(parent)
{
    blockSize = 0;
    commsSocket = new QTcpSocket(this);
    connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(handleError(QAbstractSocket::SocketError)));
    connect(commsSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(commsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    //    connect(commsSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketWroteBytes(qint64)));

    pb = new PacketBuilder();
}

ServerCommunicator::~ServerCommunicator()
{
    delete pb;
}

void ServerCommunicator::sendPacket(QByteArray unprocessedPacket)
{
    pb->resetPacket();
    pb->processRawPacketForSending(unprocessedPacket);
    commsSocket->write(pb->getPacket());
}

void ServerCommunicator::sendPreloadedPacket()
{
    // and send block
    pb->processForSending();
    commsSocket->write(pb->getPacket());
}


void ServerCommunicator::sendChatAll(QString chat)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_CHAT << chat;
    sendPreloadedPacket();
}


void ServerCommunicator::readFromServer()
{
    // same structure as server's read

    //QDataStream needs EVERY byte to be available for reading!
    while (commsSocket->bytesAvailable() > 0)
    {
        if (blockSize == 0)
        {
            if (commsSocket->bytesAvailable() < 4)
                return;

            quint16 header;
            quint16 packetlength;

            // there are 4 bytes available. read them in!

            in >> header;
            in >> packetlength;
            if (header != MAGIC_NUMBER) // magic number
            {
                emit badMagicNumber();

                commsSocket->disconnectFromHost();
                return;
            }
            blockSize = packetlength;

        }

        if (commsSocket->bytesAvailable() < blockSize)
            return;

        // ok, we can now read the WHOLE packet
        // ideally we read the 'case' byte right now, and then call a process function with
        // 'in' (the QDataStream) as a parameter!
        // the process function will set blocksize to 0 at the end
        quint8 packetType;
        in >> packetType; // this is the case byte!
        qDebug() << "Client received: Packet type " << (char)packetType << "block length" << blockSize;
        switch(packetType)
        {
        case SERVER_PING:
            {
                // keep alive
                pb->resetPacket();
                pb->o << (quint8)CLIENT_PONG;
                sendPreloadedPacket();
            }
            break;
        case SERVER_MAX_BANDWIDTH:
            {
                quint32 maxBandwidth;
                in >> maxBandwidth;
                //     uiMainWindow.progressBarBandwidthUsage->setRange(0, maxBandwidth);
                //    uiMainWindow.progressBarBandwidthUsage->setValue(0);

            }
            break;
        case SERVER_RESET_TODAYS_BANDWIDTH:
            {
                //  uiMainWindow.progressBarBandwidthUsage->setValue(0);
            }
            break;

        case SERVER_LOGGED_IN:	// logged in (entered)
            {
                QString username;
                in >> username;
                emit userLoggedIn(username);

            }
            break;
        case SERVER_LOGGED_OUT:	// logged out
            {
                QString username;
                in >> username;
                emit userLoggedOut(username);
            }
            break;

        case SERVER_ERROR:	// error
            {
                QString errorString;
                in >> errorString;
                emit errorFromServer(errorString);

            }
            break;
        case SERVER_CHAT:	// chat
            {
                QString username;
                in >> username;
                QString text;
                in >> text;
                emit chatReceived(username, text);

            }
            break;
        case SERVER_PM:	// PM
            {
                QString username, message;
                in >> username >> message;
                emit pmReceived(username, message);

            }
            break;
        case SERVER_NEW_TABLE:	// New table
            {
                // there is a new table

                // static information
                quint16 tablenum;
                quint8 gameType;
                QString lexiconName;
                QString tableName;
                quint8 maxPlayers;
                bool isPrivate;
                //		QStringList

                in >> tablenum >> gameType >> lexiconName >> tableName >> maxPlayers >> isPrivate;
                /* TODO genericize this as well (like in the server) to take in a table number and type,
                           then read different amount of info for each type */
                emit newTableInfoReceived(tablenum, gameType, lexiconName, tableName, maxPlayers, isPrivate);
            }
            break;
        case SERVER_JOIN_TABLE:	// player joined table
            {
                quint16 tablenum;
                QString playerName;

                in >> tablenum >> playerName;
                emit playerJoinedTable(tablenum, playerName);

            }
            break;
        case SERVER_TABLE_PRIVACY:
            {
                quint16 tablenum;
                bool privacy;

                in >> tablenum >> privacy;
                emit tablePrivacyChange(tablenum, privacy);


            }
            break;

        case SERVER_INVITE_TO_TABLE:
            {
                quint16 tablenum;
                QString username;
                in >> tablenum >> username;
                emit receivedTableInvite(tablenum, username);
            }
            break;
        case SERVER_BOOT_FROM_TABLE:
            {
                quint16 tablenum;
                QString username;
                in >> tablenum >> username;
                emit bootedFromTable(tablenum, username);
            }
            break;
        case SERVER_LEFT_TABLE:
            {
                // player left table
                quint16 tablenum;
                QString playerName;
                in >> tablenum >> playerName;

                emit playerLeftTable(tablenum, playerName);
            }

            break;
        case SERVER_KILL_TABLE:
            {
                // table has ceased to exist
                quint16 tablenum;
                in >> tablenum;
                emit tableDeleted(tablenum);
            }
            break;
        case SERVER_WORD_LISTS:

            // word lists
            handleWordlistsMessage();   // this function takes care of emitting the appropriate signals too
            break;
        case SERVER_TABLE_COMMAND:
            // table command
            // an additional byte is needed
            {


                handleTableCommand();

            }
            break;
        case SERVER_MESSAGE:
            // server message
            {
                QString serverMessage;
                in >> serverMessage;

                emit gotServerMessage(serverMessage);

            }
            break;
        case SERVER_HIGH_SCORES:
            // high scores!
            {
                processHighScores();

            }
            break;

        case SERVER_UNSCRAMBLEGAME_LISTDATA_BEGIN:
            {
                emit beginUnscramblegameListData();

            }
            break;
        case SERVER_UNSCRAMBLEGAME_LISTDATA_ADDONE:
            {
                QString lexicon;
                QStringList labels;
                in >> lexicon >> labels;
                emit addUnscramblegameListData(lexicon, labels);


            }
            break;
        case SERVER_UNSCRAMBLEGAME_LISTDATA_DONE:
            {
                emit doneUnscramblegameListData();

            }
            break;
        case SERVER_UNSCRAMBLEGAME_LISTDATA_CLEARONE:
            {
                QString lexicon, listname;
                in >> lexicon >> listname;
                emit clearSingleUnscramblegameListData(lexicon, listname);

            }
            break;
        case SERVER_UNSCRAMBLEGAME_LISTSPACEUSAGE:
            {
                quint32 usage, max;
                in >> usage >> max;
                emit unscramblegameListSpaceUsage(usage, max);

            }
            break;
        default:
            emit dontUnderstandPacket();
            commsSocket->disconnectFromHost();
        }

        blockSize = 0;
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

void ServerCommunicator::socketDisconnected()
{
    emit serverDisconnect();
}

void ServerCommunicator::socketConnected()
{
    // only after connecting!
    blockSize = 0;
    in.setDevice(commsSocket);
    in.setVersion(QDataStream::Qt_4_2);


    // here we see if we are registering a name, or if we are connecting with an existing
    // name/password
    pb->resetPacket();

    if (connectionMode == MODE_LOGIN)
    {
        emit sentLogin();
        pb->o << (quint8)CLIENT_LOGIN;
    }
    else if (connectionMode == MODE_REGISTER)
    {
        pb->o << (quint8)CLIENT_REGISTER;
    }
    pb->o << username << password;
    sendPreloadedPacket();
}

bool ServerCommunicator::isConnectedToServer()
{
    return commsSocket->state() == QAbstractSocket::ConnectedState;
}

void ServerCommunicator::connectToServer(QString server, int port, QString _username, QString _password,
                                         ConnectionModes _connectionMode)
{
    connectionMode = _connectionMode;
    username = _username;
    password = _password;

    commsSocket->abort();
    commsSocket->connectToHost(server, port);
}

void ServerCommunicator::disconnectFromServer()
{
    commsSocket->disconnectFromHost();
}


void ServerCommunicator::handleWordlistsMessage()
{

    quint8 numLexica;
    in >> numLexica;

    emit gettingLexiconAndListInfo();

    QList<QByteArray> serverLexica;
    for (int i = 0; i < numLexica; i++)
    {
        QByteArray lexicon;
        in >> lexicon;
        emit gotLexicon(lexicon);

        serverLexica << lexicon;
    }



    quint8 numTypes;
    in >> numTypes;
    for (int i = 0; i < numTypes; i++)
    {
        quint8 type;
        in >> type;
        switch(type)
        {

        case SERVER_WORD_LIST_REGULAR:
            {
                for (int j = 0; j < numLexica; j++)
                {
                    quint8 lexiconIndex;
                    quint16 numLists;
                    in >> lexiconIndex >> numLists;

                    for (int k = 0; k < numLists; k++)
                    {
                        QByteArray listTitle;
                        in >> listTitle;
                        emit addWordList(serverLexica[lexiconIndex], listTitle, (char)SERVER_WORD_LIST_REGULAR);
                    }
                }
            }
            break;


        case SERVER_WORD_LIST_CHALLENGE:
            {
                for (int j = 0; j < numLexica; j++)
                {
                    quint8 lexiconIndex;
                    quint16 numLists;
                    in >> lexiconIndex >> numLists;

                    for (int k = 0; k < numLists; k++)
                    {
                        QByteArray listTitle;
                        in >> listTitle;
                        emit addWordList(serverLexica[lexiconIndex], listTitle, (char)SERVER_WORD_LIST_CHALLENGE);
                    }
                }
            }
            break;

        }
    }

    emit doneGettingLexAndListInfo();
}



void ServerCommunicator::handleTableCommand()
{
    quint16 tablenum;
    in >> tablenum;
    quint8 commandByte;
    in >> commandByte;

    switch (commandByte)
    {
    case SERVER_TABLE_MESSAGE:
        // a message
        {
            QString message;
            in >> message;
            emit serverTableMessage(tablenum, message);
        }
        break;
    case SERVER_TABLE_TIMER_VALUE:
        // a timer byte
        {
            quint16 timerval;
            in >> timerval;
            emit serverTableTimerValue(tablenum, timerval);
        }

        break;
    case SERVER_TABLE_READY_BEGIN:
        // a request for beginning the game from a player
        // refresh ready light for each player.
        {
            quint8 seat;
            in >> seat;
            emit serverTableReadyBegin(tablenum, seat);
        }
        break;
    case SERVER_TABLE_GAME_START:
        // the game has started
        {
          emit serverTableGameStart(tablenum);
        }

        break;

    case SERVER_TABLE_AVATAR_CHANGE:
        // avatar id
        {
            quint8 seatNumber;
            quint8 avatarID;
            in >> seatNumber >> avatarID;

            emit serverTableAvatarChange(tablenum, seatNumber, avatarID);

        }
        break;
    case SERVER_TABLE_GAME_END:
        // the game has ended

        emit serverTableGameEnd(tablenum);

        break;


    case SERVER_TABLE_CHAT:
        // chat
        {
            QString username, chat;
            in >> username;
            in >> chat;
            emit serverTableChat(tablenum, username, chat);
        }
        break;
    case SERVER_TABLE_HOST:
        {
            QString host;
            in >> host;
            emit serverTableHost(tablenum, host);
        }
        break;

    case SERVER_TABLE_SUCCESSFUL_STAND:
        {
            QString username;
            quint8 seatNumber;

            in >> username >> seatNumber;
            emit serverTableSuccessfulStand(tablenum, username, seatNumber);

        }
        break;

    case SERVER_TABLE_SUCCESSFUL_SIT:
        {
            QString username;
            quint8 seatNumber;
            in >> username >> seatNumber;
            emit serverTableSuccessfulSit(tablenum, username, seatNumber);

        }
        break;
    case SERVER_TABLE_GIVEUP:
        // someone cried uncle
        {
            QString username;
            in >> username;
            emit serverTableGameEndRequest(tablenum, username);
        }
        break;

    default:

        /* any other packets are specific to a specific game, so these should be handled by the relevant
           game script ---
         read packetlength-4 bytes from the stream and pass them to a dedicated function. */
        char* byteArray = new char[blockSize - 4];  // -4 because of quint8, quint16 tablenum, quint8 cmd
        in.readRawData(byteArray, blockSize - 4);
        QByteArray ba(byteArray);

        emit specificTableCommand(tablenum, commandByte, ba);

        delete [] byteArray;



    }


}
void ServerCommunicator::processHighScores()
{
    QString challengeName;
    quint16 numSolutions;
    quint16 numEntries;
    in >> challengeName >> numSolutions >> numEntries;
    QString username;
    quint16 numCorrect;
    quint16 timeRemaining;



    QList <tempHighScoresStruct> temp;
    for (int i = 0; i < numEntries; i++)
    {
        in >> username >> numCorrect >> timeRemaining;
        temp << tempHighScoresStruct(username, numCorrect, timeRemaining);
    }
    qSort(temp.begin(), temp.end(), highScoresLessThan);

    emit clearHighScoresTable();

    for (int i = 0; i < numEntries; i++)        
    {
        emit newHighScore(i+1, temp.at(i).username,
                          100.0*(double)temp.at(i).numCorrect/(double)numSolutions, temp.at(i).timeRemaining);

    }

    emit endHighScoresTable();

}

void ServerCommunicator::chatTable(QString textToSend, quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum;
    if (textToSend.indexOf("/me ") == 0)
    {
        pb->o << (quint8)CLIENT_TABLE_ACTION << textToSend.mid(4);
    }
    else
    {
        pb->o << (quint8)CLIENT_TABLE_CHAT << textToSend;
    }
    sendPreloadedPacket();

}

void ServerCommunicator::changeMyAvatar(quint8 avatarID, quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8) CLIENT_TABLE_AVATAR << avatarID;

    sendPreloadedPacket();
}


void ServerCommunicator::sendPM(QString username, QString message)
{
    if (message.simplified() == "")
    {
        return;
    }

    pb->resetPacket();
    pb->o << (quint8)CLIENT_PM << username << message;
    sendPreloadedPacket();
}

void ServerCommunicator::joinTable(quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_JOIN_TABLE << tablenum;
    sendPreloadedPacket();
}

void ServerCommunicator::leaveTable(quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_LEAVE_TABLE << tablenum;
    sendPreloadedPacket();
}

void ServerCommunicator::trySitting(quint8 seat, quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TRY_SITTING << seat;
    sendPreloadedPacket();
}

void ServerCommunicator::standUp(quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TRY_STANDING;
    sendPreloadedPacket();
}

void ServerCommunicator::trySetTablePrivate(quint16 tablenum, bool priv)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TABLE_PRIVACY << priv;
    sendPreloadedPacket();

}

void ServerCommunicator::sendReady(quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TABLE_READY_BEGIN;
    sendPreloadedPacket();

}

void ServerCommunicator::sendGiveup(quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TABLE_GIVEUP;
    sendPreloadedPacket();
}

void ServerCommunicator::sendClientVersion(QString version)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_VERSION << version;
    sendPreloadedPacket();
}

void ServerCommunicator::requestHighScores(QString challenge)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_HIGH_SCORE_REQUEST << challenge;
    sendPreloadedPacket();

}

void ServerCommunicator::uploadWordList(QString lexicon, QList<quint32> &probIndices, QString listName)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_UNSCRAMBLEGAME_LIST_UPLOAD;
    pb->o << (quint8)2;   // this means what follows is the size of the list
    pb->o << (quint32)probIndices.size();
    pb->o << lexicon;
    pb->o << listName.mid(0, 64);

    sendPreloadedPacket();

    do
    {
        pb->resetPacket();

        pb->o << (quint8)CLIENT_UNSCRAMBLEGAME_LIST_UPLOAD;
        pb->o << (quint8)1;   // this means that this list is CONTINUED (i.e. tell the server to wait for more of these packets)
        pb->o << probIndices.mid(0, 2000);

        sendPreloadedPacket();

        probIndices = probIndices.mid(2000);
    } while (probIndices.size() > 2000);

    pb->resetPacket();

    pb->o << (quint8)CLIENT_UNSCRAMBLEGAME_LIST_UPLOAD;
    pb->o << (quint8)0;   // this means that this list is DONE
    pb->o << probIndices;

    sendPreloadedPacket();

}

void ServerCommunicator::requestSavedWordListInfo(QString lexicon)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_UNSCRAMBLEGAME_LISTINFO_REQUEST << lexicon;
    sendPreloadedPacket();
}

void ServerCommunicator::invitePlayerToTable(quint16 tablenum, QString playerToInvite)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TABLE_INVITE << playerToInvite;
    sendPreloadedPacket();
}

void ServerCommunicator::bootFromTable(quint16 tablenum, QString playerToBoot)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TABLE_BOOT << playerToBoot;
    sendPreloadedPacket();
}

void ServerCommunicator::saveGame(quint16 tablenum)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_TABLE_COMMAND << tablenum << (quint8)CLIENT_TABLE_UNSCRAMBLEGAME_SAVE_REQUEST;
    sendPreloadedPacket();
}

void ServerCommunicator::deleteList(QString lexicon, QString listname)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_UNSCRAMBLEGAME_DELETE_LIST << lexicon << listname;
    sendPreloadedPacket();
}

void ServerCommunicator::sendSuggestionOrBugReport(QString suggestion)
{
    pb->resetPacket();
    pb->o << (quint8)CLIENT_SUGGESTION_OR_BUG_REPORT << suggestion;
    sendPreloadedPacket();
}

