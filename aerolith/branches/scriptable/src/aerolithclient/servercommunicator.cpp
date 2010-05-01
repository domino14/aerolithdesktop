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


ServerCommunicator::ServerCommunicator(QObject* parent) : QObject(parent),
                                                            out(&block, QIODevice::WriteOnly)
{
    blockSize = 0;
    commsSocket = new QTcpSocket(this);
    connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(commsSocket, SIGNAL(connected()), this, SLOT(connectedToServer()));

    connect(commsSocket, SIGNAL(disconnected()), this, SLOT(serverDisconnection()));
//    connect(commsSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(socketWroteBytes(qint64)));
}

void ServerCommunicator::sendPacket(QByteArray unprocessedPacket)
{
    // possibly add encryption to packet at some point?
    out.device()->seek(0);
    block.clear();
    out << MAGIC_NUMBER;
    out << (quint16)unprocessedPacket.length();
    out.writeRawData(unprocessedPacket.constData(), unprocessedPacket.length());

    // and send block
    commsSocket->write(block);

}


void ServerCommunicator::sendChatAll(QString chat)
{
    QByteArray packet;
    QDataStream o(&packet, QIODevice::WriteOnly);
    o << (quint8)CLIENT_CHAT << chat;
    sendPacket(packet);
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
        qDebug() << "Packet type " << (char)packetType << "block length" << blockSize;
        switch(packetType)
        {
        case SERVER_PING:
            {
                // keep alive
                QByteArray packet;
                QDataStream o(&packet, QIODevice::WriteOnly);
                o << (quint8)CLIENT_PONG;
                sendPacket(packet);
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

        case SERVER_UNSCRAMBLEGAME_LISTDATA_CLEARALL:
            {
                emit clearAllUnscramblegameListData();

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
                emit clearUnscramblegameListData(lexicon, listname);

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


void ServerCommunicator::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        uiMainWindow.chatText->append("<font color=red>The host was not found. Please check the "
                                      "host name and port settings.</font>");
        break;
    case QAbstractSocket::ConnectionRefusedError:
        uiMainWindow.chatText->append("<font color=red>The connection was refused by the peer. "
                                      "Make sure the Aerolith server is running, "
                                      "and check that the host name and port "
                                      "settings are correct.</font>");
        break;
    default:
        uiMainWindow.chatText->append(QString("<font color=red>The following error occurred: %1.</font>")
                                      .arg(commsSocket->errorString()));
    }

    uiLogin.loginPushButton->setText("Connect");
    uiLogin.connectStatusLabel->setText("Disconnected.");
}

void ServerCommunicator::serverDisconnection()
{
    uiLogin.connectStatusLabel->setText("You are disconnected.");
    uiMainWindow.listWidgetPeopleConnected->clear();
    peopleLoggedIn.clear();
    //	QMessageBox::information(this, "Aerolith Client", "You have been disconnected.");
    uiLogin.loginPushButton->setText("Connect");
    //centralWidget->hide();
    loginDialog->show();
    loginDialog->raise();
    uiMainWindow.roomTableWidget->clearContents();
    uiMainWindow.roomTableWidget->setRowCount(0);

    QList <tableRepresenter*> tableStructs = tables.values();
    tables.clear();
    foreach (tableRepresenter* t, tableStructs)
        delete t;


    setWindowTitle("Aerolith - disconnected");
    gameBoardWidget->hide();
    uiMainWindow.comboBoxLexicon->setEnabled(true);
}

void ServerCommunicator::connectedToServer()
{
    // only after connecting!
    blockSize = 0;
    in.setDevice(commsSocket);
    in.setVersion(QDataStream::Qt_4_2);


    // here we see if we are registering a name, or if we are connecting with an existing
    // name/password

    if (connectionMode == MODE_LOGIN)
    {

        currentUsername = uiLogin.usernameLE->text();


        writeHeaderData();
        out << (quint8)CLIENT_LOGIN;
        out << currentUsername;
        out << uiLogin.passwordLE->text();
        fixHeaderLength();
        commsSocket->write(block);
    }
    else if (connectionMode == MODE_REGISTER)
    {
        writeHeaderData();
        out << (quint8)CLIENT_REGISTER;
        out << uiLogin.desiredUsernameLE->text();
        out << uiLogin.desiredFirstPasswordLE->text();
        fixHeaderLength();
        commsSocket->write(block);
    }
}

/*
void MainWindow::handleWordlistsMessage()
{

    quint8 numLexica;
    in >> numLexica;
    disconnect(uiMainWindow.comboBoxLexicon, SIGNAL(currentIndexChanged(QString)), 0, 0);
    uiMainWindow.comboBoxLexicon->clear();
    lexiconListsHash.clear();
    qDebug() << "Got" << numLexica << "lexica.";

    QHash <int, QString> localLexHash;

    for (int i = 0; i < numLexica; i++)
    {
        QByteArray lexicon;
        in >> lexicon;
        localLexHash.insert(i, QString(lexicon));

        if (existingLocalDBList.contains(lexicon))
            uiMainWindow.comboBoxLexicon->addItem(lexicon);
        LexiconLists dummyLists;
        dummyLists.lexicon = lexicon;
        lexiconListsHash.insert(QString(lexicon), dummyLists);

    }


    quint8 numTypes;
    in >> numTypes;
    for (int i = 0; i < numTypes; i++)
    {
        quint8 type;
        in >> type;
        switch(type)
        {

        case 'R':
            {
                for (int j = 0; j < numLexica; j++)
                {
                    quint8 lexiconIndex;
                    quint16 numLists;
                    in >> lexiconIndex >> numLists;

                    QString lexicon = localLexHash.value(lexiconIndex);
                    for (int k = 0; k < numLists; k++)
                    {
                        QByteArray listTitle;
                        in >> listTitle;
                        lexiconListsHash[lexicon].regularWordLists << listTitle;
                    }
                }
            }
            break;


        case 'D':
            {
                for (int j = 0; j < numLexica; j++)
                {
                    quint8 lexiconIndex;
                    quint16 numLists;
                    in >> lexiconIndex >> numLists;

                    QString lexicon = localLexHash.value(lexiconIndex);

                    for (int k = 0; k < numLists; k++)
                    {
                        QByteArray listTitle;
                        in >> listTitle;
                        lexiconListsHash[lexicon].dailyWordLists << listTitle;
                    }
                }
            }
            break;

        }
    }

    if (uiMainWindow.comboBoxLexicon->count() > 0)
    {
        uiMainWindow.comboBoxLexicon->setCurrentIndex(0);
        lexiconComboBoxIndexChanged(uiMainWindow.comboBoxLexicon->currentText());
    }
    else
    {
        QMessageBox::critical(this, "No Lexicon Databases!", "You have no lexicon databases built. You will not be"
                              " able to play Aerolith without building a lexicon database. Please select the 'Lexica'"
                              " option from the menu and build at least one lexicon database, then reconnect to Aerolith.");


    }
    // we connect the signals here instead of earlier in the constructor for some reason having to do with the
    //   above two lines. the 'disconnect' is earlier in this function


    connect(uiMainWindow.comboBoxLexicon, SIGNAL(currentIndexChanged(QString)),
            SLOT(lexiconComboBoxIndexChanged(QString)));

    spinBoxWordLengthChange(uiCreateScrambleTable.spinBoxWL->value());


}
*/


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
            gameBoardWidget->gotChat("<font color=green>" + message + "</font>");


        }
        break;
    case SERVER_TABLE_TIMER_VALUE:
        // a timer byte
        {
            quint16 timerval;
            in >> timerval;
            gameBoardWidget->gotTimerValue(timerval);


        }

        break;
    case SERVER_TABLE_READY_BEGIN:
        // a request for beginning the game from a player
        // refresh ready light for each player.
        {
            quint8 seat;
            in >> seat;
            gameBoardWidget->setReadyIndicator(seat);
        }
        break;
    case SERVER_TABLE_GAME_START:
        // the game has started
        {
            gameBoardWidget->setupForGameStart();
            gameBoardWidget->gotChat("<font color=red>The game has started!</font>");
            gameStarted = true;

            //gameTimer->start(1000);
        }

        break;

    case SERVER_TABLE_AVATAR_CHANGE:
        // avatar id
        {
            quint8 seatNumber;
            quint8 avatarID;
            in >> seatNumber >> avatarID;

            gameBoardWidget->setAvatar(seatNumber, avatarID);

            // then here we can do something like chatwidget->setavatar( etc). but this requires the server
            // to send avatars to more than just the table. so if we want to do this, we need to change the server behavior!
            // this way we can just send everyone's avatar on login. consider changing this!
        }
        break;
    case SERVER_TABLE_GAME_END:
        // the game has ended

        gameBoardWidget->gotChat("<font color=red>This round is over.</font>");
        gameStarted = false;
        gameBoardWidget->populateSolutionsTable();
        ///gameTimer->stop();
        gameBoardWidget->clearAllWordTiles();
        break;


    case SERVER_TABLE_CHAT:
        // chat
        {
            QString username, chat;
            in >> username;
            in >> chat;
            gameBoardWidget->gotChat("[" + username + "] " + chat);
        }
        break;
    case SERVER_TABLE_HOST:
        {
            QString host;
            in >> host;
            gameBoardWidget->setHost(host);
        }
        break;

    case SERVER_TABLE_SUCCESSFUL_STAND:
        {
            QString username;
            quint8 seatNumber;

            in >> username >> seatNumber;
            gameBoardWidget->standup(username, seatNumber);
        }
        break;

    case SERVER_TABLE_SUCCESSFUL_SIT:
        {
            QString username;
            quint8 seatNumber;
            in >> username >> seatNumber;
            gameBoardWidget->sitdown(username, seatNumber);
            qDebug() << "Got saet packet" << username << seatNumber;

        }
        break;

    default:

        /* any other packets are specific to a specific game, so these should be handled by the relevant
           game script ---
         read packetlength-4 bytes from the stream and pass them to a dedicated function. */
        char byteArray[] = new char[packetlength - 4];  // -4 because of quint8, quint16 tablenum, quint8 cmd
        in.readRawData(byteArray, packetlength - 4);
        QByteArray ba(byteArray);

        emit specificTableCommand(ba, tablenum, commandByte);

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
    uiScores.scoresTableWidget->clearContents();
    uiScores.scoresTableWidget->setRowCount(0);


    QList <tempHighScoresStruct> temp;
    for (int i = 0; i < numEntries; i++)
    {
        in >> username >> numCorrect >> timeRemaining;
        temp << tempHighScoresStruct(username, numCorrect, timeRemaining);
    }
    qSort(temp.begin(), temp.end(), highScoresLessThan);

    for (int i = 0; i < numEntries; i++)
    {

        QTableWidgetItem* rankItem = new QTableWidgetItem(QString("%1").arg(i+1));
        QTableWidgetItem* usernameItem = new QTableWidgetItem(temp.at(i).username);
        QTableWidgetItem* correctItem = new QTableWidgetItem(QString("%1%").arg(100.0*(double)temp.at(i).numCorrect/(double)numSolutions, 0, 'f', 1));
        QTableWidgetItem* timeItem = new QTableWidgetItem(QString("%1 s").arg(temp.at(i).timeRemaining));
        uiScores.scoresTableWidget->insertRow(uiScores.scoresTableWidget->rowCount());
        uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 0, rankItem);
        uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 1, usernameItem);
        uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 2, correctItem);
        uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 3, timeItem);
    }

    uiScores.scoresTableWidget->resizeColumnsToContents();
}
