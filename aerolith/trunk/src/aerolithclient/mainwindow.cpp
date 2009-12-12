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

#include "mainwindow.h"
#include "commonDefs.h"
#include "databasehandler.h"
const quint16 MAGIC_NUMBER = 25349;

bool highScoresLessThan(const tempHighScoresStruct& a, const tempHighScoresStruct& b)
{
    if (a.numCorrect == b.numCorrect) return (a.timeRemaining > b.timeRemaining);
    else return (a.numCorrect > b.numCorrect);
}

MainWindow::MainWindow(QString aerolithVersion, DatabaseHandler* databaseHandler) :
        aerolithVersion(aerolithVersion), dbHandler(databaseHandler), PLAYERLIST_ROLE(Qt::UserRole),
        out(&block, QIODevice::WriteOnly)
{



    createTableDialog = new QDialog(this);
    uiTable.setupUi(createTableDialog);


    uiMainWindow.setupUi(this);
    uiMainWindow.roomTableWidget->verticalHeader()->hide();
    uiMainWindow.roomTableWidget->setColumnWidth(0, 30);
    uiMainWindow.roomTableWidget->setColumnWidth(1, 40);
    uiMainWindow.roomTableWidget->setColumnWidth(2, 200);
    uiMainWindow.roomTableWidget->setColumnWidth(3, 350);
    uiMainWindow.roomTableWidget->setColumnWidth(4, 75);

    uiMainWindow.roomTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);

    WindowTitle = "Aerolith " + aerolithVersion;
    setWindowTitle(WindowTitle);

    challengesMenu = new QMenu;

    connect(challengesMenu, SIGNAL(triggered(QAction*)), this, SLOT(dailyChallengeSelected(QAction*)));
    uiMainWindow.pushButtonChallenges->setMenu(challengesMenu);

    connect(uiMainWindow.pushButtonNewTable, SIGNAL(clicked()), this, SLOT(createNewRoom()));
    uiMainWindow.chatText->setOpenExternalLinks(true);
    connect(uiMainWindow.listWidgetPeopleConnected, SIGNAL(sendPM(QString)), this, SLOT(sendPM(QString)));
    connect(uiMainWindow.listWidgetPeopleConnected, SIGNAL(viewProfile(QString)), this, SLOT(viewProfile(QString)));

    uiMainWindow.chatText->document()->setMaximumBlockCount(5000);  // at most 5000 newlines.

    connect(uiMainWindow.lineEditChat, SIGNAL(returnPressed()), this, SLOT(submitChatLEContents()));

    // commsSocket

    commsSocket = new QTcpSocket;
    connect(commsSocket, SIGNAL(readyRead()), this, SLOT(readFromServer()));
    connect(commsSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    connect(commsSocket, SIGNAL(connected()), this, SLOT(connectedToServer()));

    connect(commsSocket, SIGNAL(disconnected()), this, SLOT(serverDisconnection()));

    connect(uiTable.buttonBox, SIGNAL(accepted()), SLOT(createUnscrambleGameTable()));
    connect(uiTable.buttonBox, SIGNAL(rejected()), createTableDialog, SLOT(hide()));

    connect(uiTable.spinBoxWL, SIGNAL(valueChanged(int)), SLOT(spinBoxWordLengthChange(int)));

    scoresDialog = new QDialog(this);
    uiScores.setupUi(scoresDialog);
    uiScores.scoresTableWidget->verticalHeader()->hide();
    connect(uiScores.pushButtonGetScores, SIGNAL(clicked()), this, SLOT(getScores()));

    loginDialog = new QDialog(this);
    uiLogin.setupUi(loginDialog);
    //  loginDialog->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    connect(uiLogin.loginPushButton, SIGNAL(clicked()), this, SLOT(toggleConnectToServer()));

    connect(uiLogin.registerPushButton, SIGNAL(clicked()), this, SLOT(showRegisterPage()));
    connect(uiLogin.cancelRegPushButton, SIGNAL(clicked()), this, SLOT(showLoginPage()));

    connect(uiLogin.submitRegPushButton, SIGNAL(clicked()), this, SLOT(registerName()));
    connect(uiLogin.pushButtonStartOwnServer, SIGNAL(clicked()), this, SLOT(startOwnServer()));


    scoresDialog->setAttribute(Qt::WA_QuitOnClose, false);
    loginDialog->setAttribute(Qt::WA_QuitOnClose, false);


    gameStarted = false;
    blockSize = 0;

    currentTablenum = 0;
    uiLogin.stackedWidget->setCurrentIndex(0);


    gameBoardWidget = new UnscrambleGameTable(0, Qt::Window, dbHandler);
    gameBoardWidget->setWindowTitle("Table");
    gameBoardWidget->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    gameBoardWidget->setAttribute(Qt::WA_QuitOnClose, false);

    connect(gameBoardWidget, SIGNAL(giveUp()), this, SLOT(giveUpOnThisGame()));
    connect(gameBoardWidget, SIGNAL(sendStartRequest()), this, SLOT(submitReady()));
    connect(gameBoardWidget, SIGNAL(avatarChange(quint8)), this, SLOT(changeMyAvatar(quint8)));
    connect(gameBoardWidget, SIGNAL(correctAnswerSubmitted(quint8, quint8)), this, SLOT(submitCorrectAnswer(quint8, quint8)));
    connect(gameBoardWidget, SIGNAL(chatTable(QString)), this, SLOT(chatTable(QString)));

    connect(gameBoardWidget, SIGNAL(viewProfile(QString)), this, SLOT(viewProfile(QString)));
    connect(gameBoardWidget, SIGNAL(sendPM(QString)), this, SLOT(sendPM(QString)));
    connect(gameBoardWidget, SIGNAL(exitThisTable()), this, SLOT(leaveThisTable()));


    out.setVersion(QDataStream::Qt_4_2);

    QTime midnight(0, 0, 0);
    qsrand(midnight.msecsTo(QTime::currentTime()));

    connect(uiMainWindow.actionAerolith_Help, SIGNAL(triggered()), this, SLOT(aerolithHelpDialog()));
    connect(uiMainWindow.actionConnect_to_Aerolith, SIGNAL(triggered()), loginDialog, SLOT(raise()));
    connect(uiMainWindow.actionConnect_to_Aerolith, SIGNAL(triggered()), loginDialog, SLOT(show()));
    connect(uiMainWindow.actionAcknowledgements, SIGNAL(triggered()), this, SLOT(aerolithAcknowledgementsDialog()));
    connect(uiMainWindow.actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));


    gameTimer = new QTimer();
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGameTimer()));

    readWindowSettings();

    show();
    loginDialog->show();
    //  loginDialog->activateWindow();
    loginDialog->raise();
    uiLogin.usernameLE->setFocus(Qt::OtherFocusReason);
    setWindowIcon(QIcon(":images/aerolith.png"));


    setProfileWidget = new QWidget(this, Qt::Window);
    uiSetProfile.setupUi(setProfileWidget);

    getProfileWidget = new QWidget(this, Qt::Window);
    uiGetProfile.setupUi(getProfileWidget);

    databaseDialog = new QDialog(this);
    uiDatabase.setupUi(databaseDialog);

    connect(uiMainWindow.actionCreate_databases, SIGNAL(triggered()), databaseDialog, SLOT(show()));

    connect(uiDatabase.pushButtonCreateDatabases, SIGNAL(clicked()), SLOT(createDatabasesOKClicked()));

    connect(dbHandler, SIGNAL(setProgressMessage(QString)), uiDatabase.labelProgress, SLOT(setText(QString)));
    connect(dbHandler, SIGNAL(setProgressValue(int)), uiDatabase.progressBar, SLOT(setValue(int)));
    connect(dbHandler, SIGNAL(setProgressRange(int, int)), uiDatabase.progressBar, SLOT(setRange(int, int)));
    connect(dbHandler, SIGNAL(enableClose(bool)), SLOT(dbDialogEnableClose(bool)));
    connect(dbHandler, SIGNAL(createdDatabase(QString)), SLOT(databaseCreated(QString)));

    //   connect(uiTable.pushButtonUseOwnList, SIGNAL(clicked()), SLOT(uploadOwnList()));



    ///////
    // set game icons
    unscrambleGameIcon.addFile(":images/unscrambleGameSmall.png");

    QStringList dbList = dbHandler->checkForDatabases();    // move this to the database handler.. duh

    if (dbList.size() == 0)
    {

        if (QMessageBox::question(this, "Create databases?", "You do not seem to have any lexicon databases created. "
                                  "Creating a lexicon database is necessary to use Aerolith!",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            databaseDialog->show();
    }
    else
    {
        QMenu* rebuildDbMenu = new QMenu(this);
        foreach (QString str, dbList)
        {
            setCheckbox(str);
            rebuildDbMenu->addAction(str);
        }
        uiDatabase.pushButtonRebuildDatabase->setMenu(rebuildDbMenu);
        connect(rebuildDbMenu, SIGNAL(triggered(QAction*)), SLOT(rebuildDatabaseAction(QAction*)));
        dbHandler->connectToDatabases(true, dbList);
    }

    uiTable.tableWidgetMyLists->setSelectionMode(QAbstractItemView::SingleSelection);
    uiTable.tableWidgetMyLists->setSelectionBehavior(QAbstractItemView::SelectRows);
    uiTable.tableWidgetMyLists->setEditTriggers(QAbstractItemView::NoEditTriggers);

    savingGameAllowable = false;
}

void MainWindow::dbDialogEnableClose(bool e)
{
    if (!e)
        uiMainWindow.chatText->append("<font color=red>Databases are being created.</font>");
    if (e)
    {
        uiMainWindow.chatText->append("<font color=red>Lexicon databases were successfully created!</font>");
        QStringList dbList = dbHandler->checkForDatabases();
        dbHandler->connectToDatabases(true, dbList);
    }
}

void MainWindow::databaseCreated(QString lexiconName)
{
    setCheckbox(lexiconName);
}

void MainWindow::rebuildDatabaseAction(QAction* action)
{
    if (QMessageBox::question(this, "Rebuild?", "Would you really like to rebuild the database '" + action->text() + "'?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        dbHandler->createLexiconDatabases(QStringList(action->text()));
}

void MainWindow::setCheckbox(QString lexiconName)
{
    if (lexiconName == "OWL2+LWL")
    {
        uiDatabase.checkBoxOWL2->setChecked(true);
        uiDatabase.checkBoxOWL2->setEnabled(false);
    }
    else if (lexiconName == "CSW")
    {
        uiDatabase.checkBoxCSW->setChecked(true);
        uiDatabase.checkBoxCSW->setEnabled(false);
    }
    else if (lexiconName == "FISE")
    {
        uiDatabase.checkBoxFISE->setChecked(true);
        uiDatabase.checkBoxFISE->setEnabled(false);
    }
    else if (lexiconName == "Volost")
    {
        uiDatabase.checkBoxVolost->setChecked(true);
        uiDatabase.checkBoxVolost->setEnabled(false);
    }
}

void MainWindow::createDatabasesOKClicked()
{
    QStringList databasesToCreate;


    if (uiDatabase.checkBoxOWL2->isEnabled() && uiDatabase.checkBoxOWL2->isChecked())
        databasesToCreate << "OWL2+LWL";
    if (uiDatabase.checkBoxCSW->isEnabled() && uiDatabase.checkBoxCSW->isChecked())
        databasesToCreate << "CSW";
    if (uiDatabase.checkBoxFISE->isEnabled() && uiDatabase.checkBoxFISE->isChecked())
        databasesToCreate << "FISE";
    if (uiDatabase.checkBoxVolost->isEnabled() && uiDatabase.checkBoxVolost->isChecked())
        databasesToCreate << "Volost";
    dbHandler->createLexiconDatabases(databasesToCreate);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeWindowSettings();
    event->accept();
}

void MainWindow::writeWindowSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "CesarWare", "Aerolith");
    settings.beginGroup("MainWindow");
    settings.setValue("pos", pos());
    settings.setValue("username", uiLogin.usernameLE->text());
    if (uiLogin.checkBoxSavePassword->isChecked())
    {
        settings.setValue("password", uiLogin.passwordLE->text());
    }
    else
    {
        settings.setValue("password", "");
    }
    settings.setValue("savePassword", uiLogin.checkBoxSavePassword->isChecked());
    settings.endGroup();

}

void MainWindow::readWindowSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       "CesarWare", "Aerolith");
    settings.beginGroup("MainWindow");
    move(settings.value("pos", QPoint(100, 100)).toPoint());
    uiLogin.usernameLE->setText(settings.value("username", "").toString());
    uiLogin.passwordLE->setText(settings.value("password", "").toString());
    uiLogin.checkBoxSavePassword->setChecked(settings.value("savePassword", true).toBool());
    settings.endGroup();
}

void MainWindow::writeHeaderData()
{
    out.device()->seek(0);
    block.clear();
    out << (quint16)MAGIC_NUMBER;	// magic number
    out << (quint16)0; // length
}

void MainWindow::fixHeaderLength()
{
    out.device()->seek(sizeof(quint16));
    out << (quint16)(block.size() - sizeof(quint16) - sizeof(quint16));
}

void MainWindow::submitChatLEContents()
{

    /* else if (chatLE->text().indexOf("/me ") == 0)
        {
        writeHeaderData();
        out << (quint8)'a';
        out << chatLE->text().mid(4);
        fixHeaderLength();
        commsSocket->write(block);
        chatLE->clear();
        }*/

    writeHeaderData();
    out << (quint8)CLIENT_CHAT;
    out << uiMainWindow.lineEditChat->text();
    fixHeaderLength();
    commsSocket->write(block);
    uiMainWindow.lineEditChat->clear();
}


void MainWindow::chatTable(QString textToSend)
{
    if (textToSend.indexOf("/me ") == 0)
    {
        writeHeaderData();
        out << (quint8)CLIENT_TABLE_COMMAND;
        out << (quint16)currentTablenum;
        out << (quint8)CLIENT_TABLE_ACTION;
        out << textToSend.mid(4);
        fixHeaderLength();
        commsSocket->write(block);
    }
    else
    {


        writeHeaderData();
        out << (quint8)CLIENT_TABLE_COMMAND;
        out << (quint16)currentTablenum;
        out << (quint8)CLIENT_TABLE_CHAT;
        out << textToSend;
        fixHeaderLength();
        commsSocket->write(block);
    }

}

void MainWindow::submitCorrectAnswer(quint8 space, quint8 specificAnswer)
{
    // chatText->append(QString("From solution: ") + solutionLE->text());
    // solutionLE->clear();

    writeHeaderData();
    out << (quint8) CLIENT_TABLE_COMMAND;
    out << (quint16) currentTablenum;
    out << (quint8) CLIENT_TABLE_UNSCRAMBLEGAME_CORRECT_ANSWER;
    out << space << specificAnswer;
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::readFromServer()
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
            if (header != (quint16)MAGIC_NUMBER) // magic number
            {
#ifdef Q_WS_MAC
                QMessageBox::critical(this, "Wrong version of Aerolith", "You seem to be using an outdated "
                                      "version of Aerolith. Please go to www.aerolith.org and download the newest "
                                      " update.");
#endif
#ifdef Q_WS_WIN
                // call an updater program
                // QDesktopServices::openUrl(QUrl("http://www.aerolith.org"));
                // QCoreApplication::quit();
                QMessageBox::critical(this, "Wrong version of Aerolith", "You seem to be using an outdated "
                                      "version of Aerolith. Please go to www.aerolith.org and download the newest "
                                      " update.");
#endif
#ifdef Q_WS_X11
                QMessageBox::critical(this, "Wrong version of Aerolith", "You seem to be using an outdated "
                                      "version of Aerolith. If you compiled this from source, please check out the new "
                                      "version of Aerolith by typing into a terminal window: <BR>"
                                      "svn co svn://www.aerolith.org/aerolith/tags/1.0 aerolith<BR>"
                                      "and then go to this directory, type in qmake, then make."
                                      " You must have Qt 4.5 on your system.");
#endif



                //uiMainWindow.chatText->append("You have the wrong version of the client. Please check http://www.aerolith.org");
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
                writeHeaderData();
                out << (quint8)CLIENT_PONG;
                fixHeaderLength();
                commsSocket->write(block);
            }
            break;

        case SERVER_LOGGED_IN:	// logged in (entered)
            {
                QString username;
                in >> username;
                QListWidgetItem *it = new QListWidgetItem(username, uiMainWindow.listWidgetPeopleConnected);
                if (username == currentUsername)
                {
                    // QSound::play("sounds/enter.wav");

                    uiLogin.connectStatusLabel->setText("You have connected!");
                    loginDialog->hide();
                    setWindowTitle(QString(WindowTitle + " - logged in as ") + username);
                    sendClientVersion();
                    gameBoardWidget->setMyUsername(username);
                    currentTablenum = 0;
                }
                if (username.toLower() == "cesar")
                {
                    it->setForeground(QBrush(Qt::blue));

                }
            }
            break;
        case SERVER_LOGGED_OUT:	// logged out
            {
                QString username;
                in >> username;
                for (int i = 0; i < uiMainWindow.listWidgetPeopleConnected->count(); i++)
                    if (uiMainWindow.listWidgetPeopleConnected->item(i)->text() == username)
                    {
                    QListWidgetItem *it = uiMainWindow.listWidgetPeopleConnected->takeItem(i);
                    delete it;
                }

            }
            break;

        case SERVER_ERROR:	// error
            {
                QString errorString;
                in >> errorString;
                QMessageBox::information(loginDialog, "Aerolith client", errorString);

                uiMainWindow.chatText->append("<font color=red>" + errorString + "</font>");
            }
            break;
        case SERVER_CHAT:	// chat
            {
                QString username;
                in >> username;
                QString text;
                in >> text;
                uiMainWindow.chatText->moveCursor(QTextCursor::End);
                uiMainWindow.chatText->insertHtml(QString("[")+username+"] " + text);
                uiMainWindow.chatText->append("");
            }
            break;
        case SERVER_PM:	// PM
            {
                QString username, message;
                in >> username >> message;
                receivedPM(username, message);
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
                //		QStringList

                in >> tablenum >> gameType >> lexiconName >> tableName >> maxPlayers;
                // create table
                handleCreateTable(tablenum, gameType, lexiconName, tableName, maxPlayers);
                /* TODO genericize this as well (like in the server) to take in a table number and type,
                           then read different amount of info for each type */
            }
            break;
        case SERVER_JOIN_TABLE:	// player joined table
            {
                quint16 tablenum;
                QString playerName;

                in >> tablenum >> playerName; // this will also black out the corresponding button for can join
                qDebug() << playerName << "joined" << tablenum;
                handleAddToTable(tablenum, playerName);
                if (playerName == currentUsername)
                {
                    currentTablenum = tablenum;
                    if (!tables.contains(tablenum))
                        break;
                    tableRepresenter* t = tables.value(tablenum);

                    QString wList = t->descriptorItem->text();
                    qDebug() << "I joined table!";
                    gameBoardWidget->resetTable(tablenum, wList, playerName);
                    gameBoardWidget->show();
                    uiMainWindow.comboBoxLexicon->setEnabled(false);

                }
                if (currentTablenum == tablenum)
                    modifyPlayerLists(tablenum, playerName, 1);
                //  chatText->append(QString("%1 has entered %2").arg(playerName).arg(tablenum));


            }
            break;
        case SERVER_LEFT_TABLE:
            {
                // player left table
                quint16 tablenum;
                QString playerName;
                in >> tablenum >> playerName;

                if (currentTablenum == tablenum)
                {//i love shoe
                    modifyPlayerLists(tablenum, playerName, -1);
                }

                if (playerName == currentUsername)
                {
                    currentTablenum = 0;
                    //gameStackedWidget->setCurrentIndex(0);
                    gameBoardWidget->hide();
                    uiMainWindow.comboBoxLexicon->setEnabled(true);
                }

                // chatText->append(QString("%1 has left %2").arg(playerName).arg(tablenum));
                handleLeaveTable(tablenum, playerName);

            }

            break;
        case SERVER_KILL_TABLE:
            {
                // table has ceased to exist
                quint16 tablenum;
                in >> tablenum;
                //	    chatText->append(QString("%1 has ceasd to exist").arg(tablenum));
                handleDeleteTable(tablenum);
            }
            break;
        case SERVER_WORD_LISTS:

            // word lists
            handleWordlistsMessage();


            break;
        case SERVER_TABLE_COMMAND:
            // table command
            // an additional byte is needed
            {
                quint16 tablenum;
                in >> tablenum;
                if (tablenum != currentTablenum)
                {
                    qDebug() << "HORRIBLE ERROR" << tablenum << currentTablenum;
                    QMessageBox::critical(this, "Aerolith client", "Critical error 10003");
                    exit(0);
                }
                quint8 addByte;
                in >> addByte;

                handleTableCommand(tablenum, addByte);

            }
            break;
        case SERVER_MESSAGE:
            // server message
            {
                QString serverMessage;
                in >> serverMessage;

                uiMainWindow.chatText->append("<font color=red>" + serverMessage + "</font>");
                gameBoardWidget->gotChat("<font color=red>" + serverMessage + "</font>");

            }
            break;
        case SERVER_HIGH_SCORES:
            // high scores!
            {
                displayHighScores();

            }


            break;
        case SERVER_AVATAR_CHANGE:
            // avatar id
            {
                QString username;
                quint8 avatarID;
                in >> username >> avatarID;
                // username changed his avatar to avatarID. if we want to display this avatar, display it
                // i.e. if we are in a table. in the future consider changing this to just a table packet but do the check now
                // just in case.
                if (currentTablenum != 0)
                {
                    // we are in a table
                    gameBoardWidget->setAvatar(username, avatarID);

                }
                // then here we can do something like chatwidget->setavatar( etc). but this requires the server
                // to send avatars to more than just the table. so if we want to do this, we need to change the server behavior!
                // this way we can just send everyone's avatar on login. consider changing this!
            }
            break;
        default:
            QMessageBox::critical(this, "Aerolith client", "Don't understand this packet!");
            commsSocket->disconnectFromHost();
        }

        blockSize = 0;
    }
}


void MainWindow::displayError(QAbstractSocket::SocketError socketError)
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

void MainWindow::toggleConnectToServer()
{
    if (commsSocket->state() != QAbstractSocket::ConnectedState)
    {
        connectionMode = MODE_LOGIN;
        commsSocket->abort();

        commsSocket->connectToHost(uiLogin.serverLE->text(), uiLogin.portLE->text().toInt());
        uiLogin.connectStatusLabel->setText("Connecting to server...");
        uiLogin.loginPushButton->setText("Disconnect");


        uiMainWindow.roomTableWidget->clearContents();
        uiMainWindow.roomTableWidget->setRowCount(0);

        QList <tableRepresenter*> tableStructs = tables.values();
        tables.clear();
        foreach (tableRepresenter* t, tableStructs)
            delete t;
    }
    else
    {
        uiLogin.connectStatusLabel->setText("Disconnected from server");
        commsSocket->disconnectFromHost();
        uiLogin.loginPushButton->setText("Connect");
        //gameStackedWidget->setCurrentIndex(2);
        //centralWidget->hide();
        loginDialog->show();
        loginDialog->raise();
        //gameBoardWidget->hide();
    }

}

void MainWindow::serverDisconnection()
{
    savingGameAllowable = false;
    uiLogin.connectStatusLabel->setText("You are disconnected.");
    uiMainWindow.listWidgetPeopleConnected->clear();
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

void MainWindow::connectedToServer()
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
    savingGameAllowable = false;
}

void MainWindow::sendPM(QString person)
{
    QString hashString = person.toLower();
    if (pmWindows.contains(hashString))
    {
        PMWidget* w = pmWindows.value(hashString);
        w->show();
        w->activateWindow();
    }
    else
    {
        PMWidget *w = new PMWidget(0, currentUsername, person);
        w->setAttribute(Qt::WA_QuitOnClose, false);
        connect(w, SIGNAL(sendPM(QString, QString)), this, SLOT(sendPM(QString, QString)));
        w->show();

        pmWindows.insert(hashString, w);
    }

}

void MainWindow::sendPM(QString username, QString message)
{
    if (message.simplified() == "")
    {
        return;
    }

    writeHeaderData();
    out << (quint8)CLIENT_PM;
    out << username;
    out << message;
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::receivedPM(QString username, QString message)
{
    QString hashString = username.toLower();
    if (pmWindows.contains(hashString))
    {
        PMWidget* w = pmWindows.value(hashString);
        w->appendText("[" + username + "] " + message);
        w->show();
        //w->activateWindow();
    }
    else
    {
        PMWidget *w = new PMWidget(0, currentUsername, username);
        w->setAttribute(Qt::WA_QuitOnClose, false);
        connect(w, SIGNAL(sendPM(QString, QString)), this, SLOT(sendPM(QString, QString)));
        w->appendText("[" + username + "] " + message);
        w->show();
        pmWindows.insert(hashString, w);
    }
    //QSound::play("sounds/inbound.wav");

}

void MainWindow::createUnscrambleGameTable()
{
    if (uiTable.radioButtonMyLists->isChecked() && uiTable.tableWidgetMyLists->selectedItems().size() == 0) return;

    writeHeaderData();
    out << (quint8)CLIENT_NEW_TABLE;
    out << (quint8)GAME_TYPE_UNSCRAMBLE;

    quint8 numPlayers = uiTable.playersSpinBox->value();

    out << numPlayers;

    if (numPlayers == 1)
    {
        // we are creating a 1-player table. these are the only conditions in which it is allowable to
        // save the game.
        savingGameAllowable = true;

    }
    else
        savingGameAllowable = false;


    if (uiTable.radioButtonOtherLists->isChecked() && uiTable.listWidgetTopLevelList->currentItem())
    {

        out << (quint8)LIST_TYPE_NAMED_LIST;
        out << uiTable.listWidgetTopLevelList->currentItem()->text();
    }
    else if (uiTable.radioButtonProbability->isChecked())
    {
        if (!uiTable.checkBoxAll->isChecked())
        {
            out << (quint8)LIST_TYPE_INDEX_RANGE_BY_WORD_LENGTH;
            if (uiTable.spinBoxProb2->value() <= uiTable.spinBoxProb1->value()) return;   // don't send any data, this table is invalid

            out << (quint8)uiTable.spinBoxWL->value() <<
                    (quint32)uiTable.spinBoxProb1->value() << (quint32)uiTable.spinBoxProb2->value();
        }
        else
        {
            out << (quint8)LIST_TYPE_ALL_WORD_LENGTH;
            out << (quint8)uiTable.spinBoxWL->value();   // special values mean the entire range.
        }
    }
    else if (uiTable.radioButtonMyLists->isChecked())
    {
        out << (quint8)LIST_TYPE_MULTIPLE_INDICES;
        QList<QTableWidgetItem*> si = uiTable.tableWidgetMyLists->selectedItems();
        if (si.size() != 5) return;

        QList <quint32> qindices;
        QList <quint32> mindices;

        DatabaseHandler::UserListQuizModes mode;
        bool seenWholeList;

        if (uiTable.radioButtonContinueListQuiz->isChecked()) mode = DatabaseHandler::MODE_CONTINUE;
        else if (uiTable.radioButtonRestartListQuiz->isChecked()) mode = DatabaseHandler::MODE_RESTART;
        else if (uiTable.radioButtonQuizFirstMissed->isChecked()) mode = DatabaseHandler::MODE_FIRSTMISSED;
        dbHandler->getProbIndicesFromSavedList(currentLexicon,si[0]->text(), qindices, mindices, mode, seenWholeList);

        out << si[0]->text().left(32);
        out << qindices << mindices;
    }


    out << uiMainWindow.comboBoxLexicon->currentText();

    if (uiTable.cycleRbo->isChecked()) out << (quint8)TABLE_TYPE_CYCLE_MODE;
    else if (uiTable.endlessRbo->isChecked()) out << (quint8)TABLE_TYPE_MARATHON_MODE;
    //else if (uiTable.randomRbo->isChecked()) out << (quint8)TABLE_TYPE_RANDOM_MODE;

    out << (quint8)uiTable.timerSpinBox->value();
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::createBonusGameTable()
{
    writeHeaderData();
    out << (quint8)CLIENT_NEW_TABLE;

    out << (quint8)GAME_TYPE_BONUS;
    out << QString("Bonus squares.");
    out << (quint8)uiTable.playersSpinBox->value();

    out << (quint8)uiMainWindow.comboBoxLexicon->currentIndex();
    fixHeaderLength();
    commsSocket->write(block);

}

void MainWindow::createNewRoom()
{

    // reset dialog to defaults first.
    uiTable.cycleRbo->setChecked(true);

    uiTable.playersSpinBox->setValue(1);
    uiTable.timerSpinBox->setValue(4);

    createTableDialog->show();
}

void MainWindow::joinTable()
{
    QPushButton* buttonThatSent = static_cast<QPushButton*> (sender());
    QVariant tn = buttonThatSent->property("tablenum");
    quint16 tablenum = (quint16)tn.toInt();

    writeHeaderData();
    out << (quint8)CLIENT_JOIN_TABLE;
    out << (quint16) tablenum;
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::leaveThisTable()
{
    writeHeaderData();
    out << (quint8)CLIENT_LEAVE_TABLE;
    out << (quint16)currentTablenum;
    fixHeaderLength();
    commsSocket->write(block);
    savingGameAllowable = false;
}


void MainWindow::handleTableCommand(quint16 tablenum, quint8 commandByte)
{
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
            QString username;
            in >> username;
            gameBoardWidget->setReadyIndicator(username);
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

    case SERVER_TABLE_QUESTIONS:
        // alphagrams!!!
        {
            QTime t;
            t.start();
            quint8 numRacks;
            in >> numRacks;
            for (int i = 0; i < numRacks; i++)
            {
                quint32 probIndex;
                in >> probIndex;
                quint8 numSolutionsNotYetSolved;
                in >> numSolutionsNotYetSolved;
                QSet <quint8> notSolved;

                quint8 temp;
                for (int j = 0; j < numSolutionsNotYetSolved; j++)
                {
                    in >> temp;
                    notSolved.insert(temp);
                }
                gameBoardWidget->addNewWord(i, probIndex, numSolutionsNotYetSolved, notSolved);
            }
            gameBoardWidget->clearSolutionsDialog();

        }
        break;

    case SERVER_TABLE_NUM_QUESTIONS:
        // word list info

        {
            quint16 numRacksSeen;
            quint16 numTotalRacks;
            in >> numRacksSeen >> numTotalRacks;
            gameBoardWidget->gotWordListInfo(QString("%1 / %2").arg(numRacksSeen).arg(numTotalRacks));
            break;
        }

    case SERVER_TABLE_GIVEUP:
        // someone cried uncle
        {
            QString username;
            in >> username;
            gameBoardWidget->gotChat("<font color=red>" + username + " gave up! </font>");
        }
        break;

    case SERVER_TABLE_CORRECT_ANSWER:
        {
            QString username;
            quint8 space, specificAnswer;
            in >> username >> space >> specificAnswer;
            QString wordAnswer = gameBoardWidget->answeredCorrectly(username, space, specificAnswer);
            gameBoardWidget->addToPlayerList(username, wordAnswer);

        }
        break;
    }


}

void MainWindow::handleWordlistsMessage()
{

    quint8 numLexica;
    in >> numLexica;
    disconnect(uiMainWindow.comboBoxLexicon, SIGNAL(currentIndexChanged(int)), 0, 0);
    uiMainWindow.comboBoxLexicon->clear();
    lexiconLists.clear();
    qDebug() << "Got" << numLexica << "lexica.";
    for (int i = 0; i < numLexica; i++)
    {
        QByteArray lexicon;
        in >> lexicon;
        uiMainWindow.comboBoxLexicon->addItem(lexicon);
        LexiconLists dummyLists;
        dummyLists.lexicon = lexicon;
        lexiconLists << dummyLists;
        qDebug() << dummyLists.lexicon;
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

                    for (int k = 0; k < numLists; k++)
                    {
                        QByteArray listTitle;
                        in >> listTitle;
                        lexiconLists[lexiconIndex].regularWordLists << listTitle;
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

                    for (int k = 0; k < numLists; k++)
                    {
                        QByteArray listTitle;
                        in >> listTitle;
                        lexiconLists[lexiconIndex].dailyWordLists << listTitle;
                    }
                }
            }
            break;

        }
    }
    uiMainWindow.comboBoxLexicon->setCurrentIndex(0);
    lexiconComboBoxIndexChanged(0);

    /* we connect the signals here instead of earlier in the constructor for some reason having to do with the
       above two lines. the 'disconnect' is earlier in this function */


    connect(uiMainWindow.comboBoxLexicon, SIGNAL(currentIndexChanged(int)),
            SLOT(lexiconComboBoxIndexChanged(int)));

    spinBoxWordLengthChange(uiTable.spinBoxWL->value());


}

void MainWindow::lexiconComboBoxIndexChanged(int index)
{
    qDebug() << "Changed lexicon to" << index;

    qDebug() << lexiconLists.at(index).regularWordLists.size();
    qDebug() << lexiconLists.at(index).dailyWordLists.size();
    uiTable.listWidgetTopLevelList->clear();
    challengesMenu->clear();
    uiScores.comboBoxChallenges->clear();

    for (int i = 0; i < lexiconLists.at(index).regularWordLists.size(); i++)
    {
        uiTable.listWidgetTopLevelList->addItem(lexiconLists.at(index).regularWordLists.at(i));
    }
    for (int i = 0; i < lexiconLists.at(index).dailyWordLists.size(); i++)
    {
        challengesMenu->addAction(lexiconLists.at(index).dailyWordLists.at(i));
        uiScores.comboBoxChallenges->addItem(lexiconLists.at(index).dailyWordLists.at(i));
    }
    challengesMenu->addAction("Get today's scores");
    // gameBoardWidget->setDatabase(lexiconLists.at(index).lexicon);
    gameBoardWidget->setLexicon(lexiconLists.at(index).lexicon);
    currentLexicon = lexiconLists.at(index).lexicon;
    uiTable.labelLexiconName->setText(currentLexicon);
    repopulateMyListsTable();

}


void MainWindow::handleCreateTable(quint16 tablenum, quint8 gameType, QString lexiconName,
                                   QString tableName, quint8 maxPlayers)
{
    tableRepresenter* t = new tableRepresenter;
    t->tableNumItem = new QTableWidgetItem(QString("%1").arg(tablenum));
    t->descriptorItem = new QTableWidgetItem("(" + lexiconName +
                                             ") " + tableName);
    t->playersItem = new QTableWidgetItem("");
    t->maxPlayers = maxPlayers;
    switch (gameType)
    {
    case GAME_TYPE_UNSCRAMBLE:

        t->typeIcon = new QTableWidgetItem(unscrambleGameIcon, "");
        t->typeIcon->setSizeHint(QSize(32, 32));
        break;
    case GAME_TYPE_BONUS:
        t->typeIcon = new QTableWidgetItem(QIcon(":/images/BonusGameSmall.png"), "");
        t->typeIcon->setSizeHint(QSize(32, 32));
        break;
    }
    t->buttonItem = new QPushButton("Join");
    t->buttonItem->setProperty("tablenum", QVariant((quint16)tablenum));
    t->tableNum = tablenum;
    connect(t->buttonItem, SIGNAL(clicked()), this, SLOT(joinTable()));
    t->buttonItem->setEnabled(false);

    //  QStringList playerList;
    //t->playersItem->setData(PLAYERLIST_ROLE, QVariant(playerList));

    int rc = uiMainWindow.roomTableWidget->rowCount();

    uiMainWindow.roomTableWidget->insertRow(rc);
    uiMainWindow.roomTableWidget->setItem(rc, 0, t->tableNumItem);
    uiMainWindow.roomTableWidget->setItem(rc, 1, t->typeIcon);
    uiMainWindow.roomTableWidget->setItem(rc, 2, t->descriptorItem);
    uiMainWindow.roomTableWidget->setItem(rc, 3, t->playersItem);
    uiMainWindow.roomTableWidget->setCellWidget(rc, 4, t->buttonItem);
    uiMainWindow.roomTableWidget->setRowHeight(rc, 40);

    //   uiMainWindow.roomTableWidget->resizeColumnsToContents();
    tables.insert(tablenum, t);



}

void MainWindow::modifyPlayerLists(quint16 tablenum, QString player, int modification)
{
    // modifies the player lists INSIDE a table

    if (!tables.contains(tablenum))
        return;
    tableRepresenter* t = tables.value(tablenum);
    if (player == currentUsername)	// I joined. therefore, populate the list from the beginning
    {
        if (modification == -1)
        {
            gameBoardWidget->leaveTable();

            return; // the widget will be hid anyway, so we don't need to hide the individual lists
            //however, we hide when adding when we join down below
        }
        else
        {
            gameBoardWidget->addPlayers(t->playerList);
            // add all players including self
            return;
        }
    }

    // if we are here then a player has joined/left a table that we were already in

    // modification = -1 remove
    // or 1 add

    if (modification == 1)
        // player has been added
        // find a spot
        gameBoardWidget->addPlayer(player, gameStarted);

    else if (modification == -1)
        gameBoardWidget->removePlayer(player, gameStarted);


}



void MainWindow::handleDeleteTable(quint16 tablenum)
{

    if (!tables.contains(tablenum))
        return;

    tableRepresenter* t = tables.value(tablenum);
    uiMainWindow.roomTableWidget->removeRow(t->tableNumItem->row()); // delete the whole row (this function deletes the elements)
    delete tables.take(tablenum); // delete the tableRepresenter object after removing it from the hash.
}

void MainWindow::handleLeaveTable(quint16 tablenum, QString player)
{


    if (!tables.contains(tablenum))
        return;

    tableRepresenter* t = tables.value(tablenum);

    t->playerList.removeAll(player);
    QString textToSet="";
    foreach(QString thisplayer, t->playerList)
        textToSet += thisplayer + " ";

    int numPlayers = t->playerList.size();
    textToSet += "(" + QString::number(numPlayers) + "/" + QString::number(t->maxPlayers) + ")";
    t->playersItem->setText(textToSet);

    if (numPlayers >= t->maxPlayers)
        t->buttonItem->setEnabled(false);
    else
        t->buttonItem->setEnabled(true);
}

void MainWindow::handleAddToTable(quint16 tablenum, QString player)
{
    // this will change button state as well

    if (!tables.contains(tablenum))
        return;

    tableRepresenter* t = tables.value(tablenum);
    t->playerList << player;
    QString textToSet = "";
    foreach(QString thisplayer, t->playerList)
        textToSet += thisplayer + " ";

    quint8 numPlayers = t->playerList.size();
    quint8 maxPlayers = t->maxPlayers;

    textToSet += "(" + QString::number(numPlayers) + "/" + QString::number(maxPlayers) + ")";

    t->playersItem->setText(textToSet);

    if (numPlayers >= maxPlayers)
    {
        t->buttonItem->setEnabled(false);
    }
    else
    {
        t->buttonItem->setEnabled(true);
    }
}


void MainWindow::giveUpOnThisGame()
{
    writeHeaderData();
    out << (quint8)CLIENT_TABLE_COMMAND;
    out << (quint16)currentTablenum;
    out << (quint8)CLIENT_TABLE_GIVEUP;
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::submitReady()
{
    writeHeaderData();
    out << (quint8)CLIENT_TABLE_COMMAND;
    out << (quint16)currentTablenum;
    out << (quint8)CLIENT_TABLE_READY_BEGIN;
    fixHeaderLength();
    commsSocket->write(block);

}

void MainWindow::aerolithHelpDialog()
{
    QString infoText;
    infoText += "- Cycle mode allows you to go through all the words in a list, and at the end, keep going through the missed words.<BR>";
    infoText += "- Tables are very customizable. To customize the colors and tiles in a table, please enter a table, and click the ""Preferences"" button. It may help to click start to see what the tiles will look like. Click Save when you are done.<BR>";
    QMessageBox::information(this, "Aerolith how-to", infoText);
}
void MainWindow::aerolithAcknowledgementsDialog()
{
    QFile file(":acknowledgments.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString infoText = file.readAll();
    QMessageBox::information(this, "Acknowledgements", infoText);
    file.close();
}

void MainWindow::showAboutQt()
{
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::displayHighScores()
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


void MainWindow::sendClientVersion()
{
    writeHeaderData();
    out << (quint8)CLIENT_VERSION;
    out << aerolithVersion;
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::changeMyAvatar(quint8 avatarID)
{

    writeHeaderData();
    out << (quint8) CLIENT_AVATAR << avatarID;
    fixHeaderLength();
    commsSocket->write(block);
}

void MainWindow::updateGameTimer()
{
    //if (gameBoardWidget->timerDial->value() == 0) return;

    //gameBoardWidget->timerDial->display(gameBoardWidget->timerDial->value() - 1);

}

void MainWindow::dailyChallengeSelected(QAction* challengeAction)
{
    if (challengeAction->text() == "Get today's scores")
    {
        uiScores.scoresTableWidget->clearContents();
        uiScores.scoresTableWidget->setRowCount(0);
        scoresDialog->show();
    }
    else
    {
        writeHeaderData();
        out << (quint8)CLIENT_NEW_TABLE;
        out << (quint8)GAME_TYPE_UNSCRAMBLE;
        out << (quint8)1; // 1 player
        out << (quint8)LIST_TYPE_DAILY_CHALLENGE;
        out << challengeAction->text();
        out << uiMainWindow.comboBoxLexicon->currentText(); // TODO this is kind of kludgy, should already know what lexicon
        // I'm on.
        out << (quint8)TABLE_TYPE_DAILY_CHALLENGES;
        out << (quint8)0;	// server should decide time for daily challenge

        fixHeaderLength();
        commsSocket->write(block);
    }
}

void MainWindow::getScores()
{
    uiScores.scoresTableWidget->clearContents();
    uiScores.scoresTableWidget->setRowCount(0);
    writeHeaderData();
    out << (quint8)CLIENT_HIGH_SCORE_REQUEST;
    out << uiScores.comboBoxChallenges->currentText();
    fixHeaderLength();
    commsSocket->write(block);


}

void MainWindow::registerName()
{

    //  int retval = registerDialog->exec();
    //if (retval == QDialog::Rejected) return;
    // validate password
    if (uiLogin.desiredFirstPasswordLE->text() != uiLogin.desiredSecondPasswordLE->text())
    {
        QMessageBox::warning(this, "Aerolith", "Your passwords do not match! Please try again");
        uiLogin.desiredFirstPasswordLE->clear();
        uiLogin.desiredSecondPasswordLE->clear();
        return;

    }



    //

    connectionMode = MODE_REGISTER;
    commsSocket->abort();
    commsSocket->connectToHost(uiLogin.serverLE->text(), uiLogin.portLE->text().toInt());
    uiLogin.connectStatusLabel->setText("Connecting to server...");
    uiLogin.loginPushButton->setText("Disconnect");
}

void MainWindow::showRegisterPage()
{
    uiLogin.stackedWidget->setCurrentIndex(1);
}

void MainWindow::showLoginPage()
{
    uiLogin.stackedWidget->setCurrentIndex(0);
}

void MainWindow::viewProfile(QString username)
{
    uiGetProfile.lineEditUsername->setText(username);
    getProfileWidget->show();

}

void MainWindow::spinBoxWordLengthChange(int length)
{
    int max = dbHandler->getNumWordsByLength(currentLexicon, length);
    if (max != 0)
        uiTable.spinBoxProb2->setMaximum(max);
}

void MainWindow::startOwnServer()
{
    // start a server thread
    if (uiLogin.pushButtonStartOwnServer->text() == "Start Own Server")
    {
        emit startServerThread();
        uiLogin.loginPushButton->setEnabled(false);
        uiLogin.pushButtonStartOwnServer->setEnabled(false);
        uiLogin.connectStatusLabel->setText("Please wait a few seconds while Aerolith Server loads.");
    }
    else
    {

        emit stopServerThread();
    }

}

void MainWindow::serverThreadHasStarted()
{
    uiLogin.pushButtonStartOwnServer->setEnabled(true);
    uiLogin.loginPushButton->setEnabled(true);
    uiLogin.connectStatusLabel->setText("Server thread has started! Log in now.");
    uiLogin.pushButtonStartOwnServer->setText("Stop Server");
    uiLogin.serverLE->setText("localhost");
    uiLogin.portLE->setText("1988");
}

void MainWindow::serverThreadHasFinished()
{

    uiLogin.connectStatusLabel->setText("Server thread has stopped!");
    uiLogin.pushButtonStartOwnServer->setText("Start Own Server");

    uiLogin.serverLE->setText("aerolith.org");
    uiLogin.portLE->setText("1988");
    uiLogin.loginPushButton->setEnabled(true);

}

void MainWindow::on_pushButtonImportList_clicked()
{
    QString listName = QInputDialog::getText(this, "List name", "Please enter a name for this list");
    if (listName == "") return;
    QString filename = QFileDialog::getOpenFileName(this, "Select a list of words or alphagrams");

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QStringList words;
    while (!in.atEnd())
    {
        QString line = in.readLine().simplified();
        if (line == line.section(" ", 0, 0))
        {
            /* basically, just ensuring that the line just has the word */
            words << line;

        }
    }

    QList <quint32> probIndices;

    bool success = dbHandler->getProbIndices(words, currentLexicon, probIndices);

    if (!success)
    {
        QMessageBox::warning(this, "Error", "You must first create a database for this lexicon!");
        return;
    }

    if (probIndices.size() > 500)
    {
        QMessageBox::warning(this, "List too big", "Your list had more than 500 alphagrams. You can only quiz on 500 "
                             "at a time. The list has been split up into several parts; you can see these below.");
    }

    success = dbHandler->saveNewLists(currentLexicon, listName, probIndices);
    if (!success)
    {
        QMessageBox::critical(this, "Error", "Was unable to connect to userlists database! Please inform "
                              "delsolar@gmail.com about this.");
        return;
    }
    repopulateMyListsTable();

}

void MainWindow::repopulateMyListsTable()
{
    uiTable.tableWidgetMyLists->clearContents();
    uiTable.tableWidgetMyLists->setRowCount(0);
    QList <QStringList> myListsTableLabels = dbHandler->getListLabels(currentLexicon);

    for (int i = 0; i < myListsTableLabels.size(); i++)
    {

        uiTable.tableWidgetMyLists->insertRow(0);
        for (int j = 0; j < myListsTableLabels.at(i).size(); j++)
            uiTable.tableWidgetMyLists->setItem(0, j, new QTableWidgetItem(myListsTableLabels[i][j]));

    }
    uiTable.tableWidgetMyLists->resizeColumnsToContents();
}

void MainWindow::on_radioButtonProbability_clicked()
{
    uiTable.stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_radioButtonOtherLists_clicked()
{
    uiTable.stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_radioButtonMyLists_clicked()
{
    uiTable.stackedWidget->setCurrentIndex(2);
    uiTable.radioButtonContinueListQuiz->setChecked(true);

}

void MainWindow::on_pushButtonDeleteList_clicked()
{
    QList<QTableWidgetItem*> si = uiTable.tableWidgetMyLists->selectedItems();
    if (si.size() != 5) return;

    if (QMessageBox::Yes == QMessageBox::warning(this, "Are you sure?", "Are you sure you wish to delete '" +
                         si[0]->text() + "'?",
                         QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
    {
        dbHandler->deleteUserList(currentLexicon, si[0]->text());
        repopulateMyListsTable();


    }

}

/////////////////////////////////////////////////////

PMWidget::PMWidget(QWidget* parent, QString senderUsername, QString receiverUsername) :
        QWidget(parent), senderUsername(senderUsername), receiverUsername(receiverUsername)
{

    uiPm.setupUi(this);
    connect(uiPm.lineEdit, SIGNAL(returnPressed()), this, SLOT(readAndSendLEContents()));
    setWindowTitle("Conversation with " + receiverUsername);
}

void PMWidget::readAndSendLEContents()
{
    emit sendPM(receiverUsername, uiPm.lineEdit->text());
    uiPm.textEdit->append("[" + senderUsername + "] " + uiPm.lineEdit->text());

    uiPm.lineEdit->clear();
    QSound::play("sounds/outbound.wav");
}

void PMWidget::appendText(QString text)
{
    uiPm.textEdit->append(text);
}
