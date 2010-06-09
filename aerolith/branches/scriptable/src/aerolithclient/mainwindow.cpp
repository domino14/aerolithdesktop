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





MainWindow::MainWindow(QString aerolithVersion, DatabaseHandler* databaseHandler) :
        aerolithVersion(aerolithVersion), dbHandler(databaseHandler)
{



    createScrambleTableDialog = new QDialog(this);
    uiCreateScrambleTable.setupUi(createScrambleTableDialog);

    createTaxesTableDialog = new QDialog(this);
    uiCreateTaxesTable.setupUi(createTaxesTableDialog);


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



    connect(uiCreateScrambleTable.buttonBox, SIGNAL(accepted()), SLOT(createUnscrambleGameTable()));
    connect(uiCreateScrambleTable.buttonBox, SIGNAL(rejected()), createScrambleTableDialog, SLOT(hide()));

    connect(uiCreateScrambleTable.spinBoxWL, SIGNAL(valueChanged(int)), SLOT(spinBoxWordLengthChange(int)));

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

    currentTablenum = 0;
    uiLogin.stackedWidget->setCurrentIndex(0);






    QTime midnight(0, 0, 0);
    qsrand(midnight.msecsTo(QTime::currentTime()));



    connect(uiMainWindow.actionAerolith_Help, SIGNAL(triggered()), this, SLOT(aerolithHelpDialog()));
    connect(uiMainWindow.actionConnect_to_Aerolith, SIGNAL(triggered()), loginDialog, SLOT(raise()));
    connect(uiMainWindow.actionConnect_to_Aerolith, SIGNAL(triggered()), loginDialog, SLOT(show()));
    connect(uiMainWindow.actionAcknowledgements, SIGNAL(triggered()), this, SLOT(aerolithAcknowledgementsDialog()));
    connect(uiMainWindow.actionAbout_Qt, SIGNAL(triggered()), this, SLOT(showAboutQt()));
    connect(uiMainWindow.actionPaypal_Donation, SIGNAL(triggered()), this, SLOT(showDonationPage()));


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


    wordFilter = new WordFilter(this, Qt::Window);
    connect(uiMainWindow.actionWord_Filter, SIGNAL(triggered()), wordFilter, SLOT(showWidget()));

    wordFilter->setDbHandler(dbHandler);




    ///////
    // set game icons
    unscrambleGameIcon.addFile(":images/unscrambleGameSmall.png");

    existingLocalDBList = dbHandler->checkForDatabases();

    if (existingLocalDBList.size() == 0)
    {
        uiLogin.pushButtonStartOwnServer->setEnabled(false);
        if (QMessageBox::question(this, "Create databases?", "You do not seem to have any lexicon databases created. "
                                  "Creating a lexicon database is necessary to use Aerolith!",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            databaseDialog->show();

    }
    else
    {
        QMenu* rebuildDbMenu = new QMenu(this);
        foreach (QString str, existingLocalDBList)
        {
            setCheckbox(str);
            rebuildDbMenu->addAction(str);
        }
        uiDatabase.pushButtonRebuildDatabase->setMenu(rebuildDbMenu);
        connect(rebuildDbMenu, SIGNAL(triggered(QAction*)), SLOT(rebuildDatabaseAction(QAction*)));
        dbHandler->connectToDatabases(true, existingLocalDBList);
    }

    uiCreateScrambleTable.tableWidgetMyLists->setSelectionMode(QAbstractItemView::SingleSelection);
    uiCreateScrambleTable.tableWidgetMyLists->setSelectionBehavior(QAbstractItemView::SelectRows);
    uiCreateScrambleTable.tableWidgetMyLists->setEditTriggers(QAbstractItemView::NoEditTriggers);


    /* server communicator */
    serverCommunicator = new ServerCommunicator(this);
    connect(serverCommunicator, SIGNAL(badMagicNumber()), this, SLOT(badMagicNumber()));
    connect(serverCommunicator, SIGNAL(userLoggedIn(QString)), this, SLOT(userLoggedIn(QString)));
    connect(serverCommunicator, SIGNAL(userLoggedOut(QString)), this, SLOT(userLoggedOut(QString)));
    connect(serverCommunicator, SIGNAL(sentLogin()), this, SLOT(sentLogin()));
    connect(serverCommunicator, SIGNAL(chatReceived(QString,QString)), this, SLOT(chatReceived(QString,QString)));
    connect(serverCommunicator, SIGNAL(errorFromServer(QString)), this, SLOT(errorFromServer(QString)));
    connect(serverCommunicator, SIGNAL(serverConnectionError(QString)), this, SLOT(serverConnectionError(QString)));
    connect(serverCommunicator, SIGNAL(serverDisconnect()), this, SLOT(serverDisconnected()));

    connect(serverCommunicator, SIGNAL(pmReceived(QString,QString)), this, SLOT(receivedPM(QString,QString)));
    connect(serverCommunicator, SIGNAL(newTableInfoReceived(quint16,quint8,QString,QString,quint8,bool)), this,
            SLOT(handleCreateTable(quint16,quint8,QString,QString,quint8,bool)));
    connect(serverCommunicator, SIGNAL(playerJoinedTable(quint16,QString)), this, SLOT(playerJoinedTable(quint16,QString)));
    connect(serverCommunicator, SIGNAL(playerLeftTable(quint16,QString)), this, SLOT(playerLeftTable(quint16,QString)));
    connect(serverCommunicator, SIGNAL(tablePrivacyChange(quint16,bool)), this, SLOT(tablePrivacyChange(quint16,bool)));
    connect(serverCommunicator, SIGNAL(receivedTableInvite(quint16,QString)), this, SLOT(tableInvite(quint16,QString)));
    connect(serverCommunicator, SIGNAL(bootedFromTable(quint16,QString)), this, SLOT(bootedFromTable(quint16,QString)));
    connect(serverCommunicator, SIGNAL(tableDeleted(quint16)), this, SLOT(handleDeleteTable(quint16)));
    connect(serverCommunicator, SIGNAL(gotServerMessage(QString)), this, SLOT(gotServerMessage(QString)));

    connect(serverCommunicator, SIGNAL(beginUnscramblegameListData()), this, SLOT(beginUnscramblegameListData()));
    connect(serverCommunicator, SIGNAL(addUnscramblegameListData(QString,QStringList)),
            this, SLOT(addUnscramblegameListData(QString,QStringList)));
    connect(serverCommunicator, SIGNAL(doneUnscramblegameListData()), this, SLOT(doneUnscramblegameListData()));
    connect(serverCommunicator, SIGNAL(clearSingleUnscramblegameListData(QString,QString)), this,
            SLOT(clearSingleUnscramblegameListData(QString,QString)));
    connect(serverCommunicator, SIGNAL(unscramblegameListSpaceUsage(quint32,quint32)), this,
            SLOT(unscramblegameListSpaceUsage(quint32,quint32)));

    connect(serverCommunicator, SIGNAL(gettingLexiconAndListInfo()), this, SLOT(gettingLexiconAndListInfo()));
    connect(serverCommunicator, SIGNAL(gotLexicon(QByteArray)), this, SLOT(gotLexicon(QByteArray)));
    connect(serverCommunicator, SIGNAL(doneGettingLexAndListInfo()), this, SLOT(doneGettingLexAndListInfo()));
    connect(serverCommunicator, SIGNAL(addWordList(QByteArray,QByteArray,char)), this, SLOT(addWordList(QByteArray,QByteArray,char)));

    connect(serverCommunicator, SIGNAL(clearHighScoresTable()), this, SLOT(clearHighScoresTable()));
    connect(serverCommunicator, SIGNAL(newHighScore(int,QString,double,int)), this,
            SLOT(newHighScore(int,QString,double,int)));
    connect(serverCommunicator, SIGNAL(endHighScoresTable()), this, SLOT(endHighScores()));

    /* TODO this idea of a single gameboard widget is temporary; fix soon */

    gameBoardWidget = new UnscrambleGameTable(0, Qt::Window, dbHandler);
    gameBoardWidget->setWindowTitle("Table");
    gameBoardWidget->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    gameBoardWidget->setAttribute(Qt::WA_QuitOnClose, false);

    connect(gameBoardWidget, SIGNAL(giveUp()), this, SLOT(giveUpOnThisGame()));
    connect(gameBoardWidget, SIGNAL(sendStartRequest()), this, SLOT(submitReady()));
    connect(gameBoardWidget, SIGNAL(avatarChange(quint8)), this, SLOT(changeMyAvatar(quint8)));
    connect(gameBoardWidget, SIGNAL(chatTable(QString)), this, SLOT(chatTable(QString)));

    connect(gameBoardWidget, SIGNAL(viewProfile(QString)), this, SLOT(viewProfile(QString)));
    connect(gameBoardWidget, SIGNAL(sendPM(QString)), this, SLOT(sendPM(QString)));
    connect(gameBoardWidget, SIGNAL(exitThisTable()), this, SLOT(leaveThisTable()));

    connect(gameBoardWidget, SIGNAL(saveCurrentGame()),
            SLOT(saveGame()));

    connect(gameBoardWidget, SIGNAL(sitDown(quint8)), SLOT(trySitting(quint8)));
    connect(gameBoardWidget, SIGNAL(standUp()), SLOT(standUp()));
    connect(gameBoardWidget, SIGNAL(setTablePrivate(bool)), SLOT(trySetTablePrivate(bool)));
    connect(gameBoardWidget, SIGNAL(showInviteDialog()), SLOT(showInviteDialog()));
    connect(gameBoardWidget, SIGNAL(bootFromTable(QString)), SLOT(bootFromTable(QString)));




    connect(gameBoardWidget, SIGNAL(sendSpecificGamePacket(QByteArray)), serverCommunicator, SLOT(sendPacket(QByteArray)));


    connect(serverCommunicator, SIGNAL(serverTableChat(quint16,QString,QString)),
            this, SLOT(gotTableChat(quint16, QString, QString)));
    connect(serverCommunicator, SIGNAL(serverTableMessage(quint16,QString)),
            this, SLOT(gotTableMessage(quint16, QString)));
    connect(serverCommunicator, SIGNAL(serverTableTimerValue(quint16,quint16)), this,
            SLOT(gotServerTableTimerValue(quint16,quint16)));
    connect(serverCommunicator, SIGNAL(serverTableReadyBegin(quint16,quint8)), this,
            SLOT(gotServerTableReadyBegin(quint16,quint8)));
    connect(serverCommunicator, SIGNAL(serverTableGameStart(quint16)), this,
            SLOT(gotServerTableGameStart(quint16)));
    connect(serverCommunicator, SIGNAL(serverTableAvatarChange(quint16,quint8,quint8)), this,
            SLOT(gotServerTableAvatarChange(quint16,quint8,quint8)));
    connect(serverCommunicator, SIGNAL(serverTableGameEnd(quint16)), this, SLOT(gotServerTableGameEnd(quint16)));
    connect(serverCommunicator, SIGNAL(serverTableHost(quint16,QString)), this,
            SLOT(gotServerTableHost(quint16,QString)));
    connect(serverCommunicator, SIGNAL(serverTableSuccessfulStand(quint16,QString,quint8)), this,
            SLOT(gotServerTableSuccessfulStand(quint16,QString,quint8)));
    connect(serverCommunicator, SIGNAL(serverTableSuccessfulSit(quint16,QString,quint8)), this,
            SLOT(gotServerTableSuccessfulSit(quint16,QString,quint8)));
    connect(serverCommunicator, SIGNAL(specificTableCommand(quint16,quint8,QByteArray)), this,
            SLOT(gotSpecificTableCommand(quint16,quint8,QByteArray)));


    testFunction(); // used for debugging

    uiLogin.portLE->setText(QString::number(DEFAULT_PORT));

    unscramblegameUserlistData_clearHash = false;



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
        uiLogin.pushButtonStartOwnServer->setEnabled(true);
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
    if (!existingLocalDBList.contains(lexiconName))
        existingLocalDBList.append(lexiconName);
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

    settings.setValue("ignoreMsgs", uiMainWindow.checkBoxIgnoreMsgs->isChecked());
    settings.setValue("ignoreInvites", uiMainWindow.checkBoxIgnoreTableInvites->isChecked());
    settings.setValue("soundsEnabled", uiMainWindow.checkBoxSounds->isChecked());

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

    uiMainWindow.checkBoxIgnoreMsgs->setChecked(settings.value("ignoreMsgs", false).toBool());
    uiMainWindow.checkBoxIgnoreTableInvites->setChecked(settings.value("ignoreInvites", false).toBool());
    uiMainWindow.checkBoxSounds->setChecked(settings.value("soundsEnabled", true).toBool());
    settings.endGroup();
}



void MainWindow::submitChatLEContents()
{

    serverCommunicator->sendChatAll(uiMainWindow.lineEditChat->text());
    uiMainWindow.lineEditChat->clear();
}


void MainWindow::changeMyAvatar(quint8 avatarID)
{
    serverCommunicator->changeMyAvatar(avatarID, currentTablenum);
}

void MainWindow::chatTable(QString chat)
{
    serverCommunicator->chatTable(chat, currentTablenum);
}



void MainWindow::toggleConnectToServer()
{
    if (!serverCommunicator->isConnectedToServer())
    {

        serverCommunicator->connectToServer(uiLogin.serverLE->text(), uiLogin.portLE->text().toInt(),
                                            uiLogin.usernameLE->text(), uiLogin.passwordLE->text(),
                                            ServerCommunicator::MODE_LOGIN);

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
        serverCommunicator->disconnectFromServer();

        uiLogin.connectStatusLabel->setText("Disconnected from server");

        uiLogin.loginPushButton->setText("Connect");
        loginDialog->show();
        loginDialog->raise();

    }

}



void MainWindow::sendPM(QString person)
{
    QString hashString = person.toLower();
    if (pmWindows.contains(hashString))
    {
        PMWidget* w = pmWindows.value(hashString);
        w->show();
        w->activateWindow();
        w->raise();
    }
    else
    {
        PMWidget *w = new PMWidget(0, currentUsername, person);
        w->setAttribute(Qt::WA_QuitOnClose, false);
        connect(w, SIGNAL(sendPM(QString, QString)), serverCommunicator, SLOT(sendPM(QString, QString)));
        connect(w, SIGNAL(shouldDelete()), this, SLOT(shouldDeletePMWidget()));
        w->show();

        pmWindows.insert(hashString, w);
    }

}


void MainWindow::receivedPM(QString username, QString message)
{
    if (uiMainWindow.checkBoxIgnoreMsgs->isChecked() == false)
    {
        QString hashString = username.toLower();
        if (pmWindows.contains(hashString))
        {
            PMWidget* w = pmWindows.value(hashString);
            w->appendText("[" + username + "] " + message);
            w->show();
            w->activateWindow();
            w->raise();

        }
        else
        {
            PMWidget *w = new PMWidget(0, currentUsername, username);
            w->setAttribute(Qt::WA_QuitOnClose, false);
            connect(w, SIGNAL(sendPM(QString, QString)), serverCommunicator, SLOT(sendPM(QString, QString)));
            connect(w, SIGNAL(shouldDelete()), this, SLOT(shouldDeletePMWidget()));
            w->appendText("[" + username + "] " + message);
            w->show();
            pmWindows.insert(hashString, w);
        }
        if (uiMainWindow.checkBoxSounds->isChecked())
            QSound::play("sounds/Sword1.wav");
    }
    else
    {
        // TODO handle reject pm message
    }
}

void MainWindow::shouldDeletePMWidget()
{
    PMWidget* w = static_cast<PMWidget*> (sender());
    pmWindows.remove(w->getRemoteUsername().toLower());
    w->deleteLater();

}

void MainWindow::createUnscrambleGameTable()
{
    if (uiCreateScrambleTable.radioButtonMyLists->isChecked() &&
        uiCreateScrambleTable.tableWidgetMyLists->selectedItems().size() == 0) return;

    /* creating tables is a very specialized thing; it makes sense to write the specially coded byte array in this function
       and then just use serverCommunicator to send it; rather than send serverCommunicator a bunch of function calls to specially
       format this data */
    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_2);

    out << (quint8)CLIENT_NEW_TABLE;
    out << (quint8)GAME_TYPE_UNSCRAMBLE;

    quint8 numPlayers = uiCreateScrambleTable.playersSpinBox->value();

    out << numPlayers;


    if (uiCreateScrambleTable.radioButtonOtherLists->isChecked() && uiCreateScrambleTable.listWidgetTopLevelList->currentItem())
    {
        QString listname = uiCreateScrambleTable.listWidgetTopLevelList->currentItem()->text();

        out << (quint8)LIST_TYPE_NAMED_LIST;
        out << listname;

        gameBoardWidget->setUnmodifiedListName(listname);

    }
    else if (uiCreateScrambleTable.radioButtonProbability->isChecked())
    {
        if (!uiCreateScrambleTable.checkBoxAll->isChecked())
        {
            out << (quint8)LIST_TYPE_INDEX_RANGE_BY_WORD_LENGTH;
            if (uiCreateScrambleTable.spinBoxProb2->value() <= uiCreateScrambleTable.spinBoxProb1->value()) return;   // don't send any data, this table is invalid

            quint8 wl;
            quint32 low, high;

            wl = uiCreateScrambleTable.spinBoxWL->value();
            low = uiCreateScrambleTable.spinBoxProb1->value();
            high = uiCreateScrambleTable.spinBoxProb2->value();

            out << wl << low << high;

            gameBoardWidget->setUnmodifiedListName(QString("%1s -- %2 to %3").arg(wl).arg(low).arg(high));

        }
        else
        {
            out << (quint8)LIST_TYPE_ALL_WORD_LENGTH;

            quint8 wl;
            quint32 low, high;

            wl = uiCreateScrambleTable.spinBoxWL->value();

            out << wl;   // special values mean the entire range.

            low = 1;    // one
            high = dbHandler->getNumWordsByLength(currentLexicon, wl);

            gameBoardWidget->setUnmodifiedListName(QString("%1s -- %2 to %3").arg(wl).arg(low).arg(high));

        }
    }
    else if (uiCreateScrambleTable.radioButtonMyLists->isChecked())
    {
        out << (quint8)LIST_TYPE_USER_LIST;
        QList<QTableWidgetItem*> si = uiCreateScrambleTable.tableWidgetMyLists->selectedItems();
        if (si.size() != 5) return;

        quint8 mode;

        if (uiCreateScrambleTable.radioButtonContinueListQuiz->isChecked()) mode = UNSCRAMBLEGAME_USERLIST_MODE_CONTINUE;
        else if (uiCreateScrambleTable.radioButtonRestartListQuiz->isChecked()) mode = UNSCRAMBLEGAME_USERLIST_MODE_RESTART;
        else if (uiCreateScrambleTable.radioButtonQuizFirstMissed->isChecked()) mode = UNSCRAMBLEGAME_USERLIST_MODE_FIRSTMISSED;


        out << mode;
        out << si[0]->text();   // list name -- must match list on server



        gameBoardWidget->setUnmodifiedListName(si[0]->text());
    }

    out << uiMainWindow.comboBoxLexicon->currentText();

    if (uiCreateScrambleTable.cycleRbo->isChecked()) out << (quint8)TABLE_TYPE_CYCLE_MODE;
    else if (uiCreateScrambleTable.endlessRbo->isChecked()) out << (quint8)TABLE_TYPE_MARATHON_MODE;
    //else if (uiTable.randomRbo->isChecked()) out << (quint8)TABLE_TYPE_RANDOM_MODE;

    out << (quint8)uiCreateScrambleTable.timerSpinBox->value();

    serverCommunicator->sendPacket(packet);
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

        /* creating tables is a very specialized thing; it makes sense to write the specially coded byte array in this function
           and then just use serverCommunicator to send it; rather than send serverCommunicator a bunch of function calls to specially
           format this data */
        QByteArray packet;
        QDataStream out(&packet, QIODevice::WriteOnly);

        out << (quint8)CLIENT_NEW_TABLE;
        out << (quint8)GAME_TYPE_UNSCRAMBLE;
        out << (quint8)1; // 1 player
        out << (quint8)LIST_TYPE_DAILY_CHALLENGE;
        out << challengeAction->text();
        out << uiMainWindow.comboBoxLexicon->currentText(); // TODO this is kind of kludgy, should already know what lexicon
        // I'm on.
        out << (quint8)TABLE_TYPE_DAILY_CHALLENGES;
        out << (quint8)0;	// server should decide time for daily challenge

        serverCommunicator->sendPacket(packet);
    }
}

void MainWindow::createBonusGameTable()
{
    //        writeHeaderData();
    //        out << (quint8)CLIENT_NEW_TABLE;
    //
    //        out << (quint8)GAME_TYPE_BONUS;
    //   //     out << (quint8)uiTable.playersSpinBox->value();
    //            out << uiMainWindow.comboBoxLexicon->currentText();
    //    //
    //    //    out << (quint8)uiMainWindow.comboBoxLexicon->currentIndex();
    //    //    fixHeaderLength();
    //    //    commsSocket->write(block);

}

void MainWindow::createNewRoom()
{

    // reset dialog to defaults first.
    uiCreateScrambleTable.cycleRbo->setChecked(true);

    uiCreateScrambleTable.playersSpinBox->setValue(1);
    uiCreateScrambleTable.timerSpinBox->setValue(4);

    createScrambleTableDialog->show();
}

void MainWindow::joinTable()
{
    QPushButton* buttonThatSent = static_cast<QPushButton*> (sender());
    QVariant tn = buttonThatSent->property("tablenum");
    quint16 tablenum = (quint16)tn.toInt();

    serverCommunicator->joinTable(tablenum);

}

void MainWindow::leaveThisTable()
{
    serverCommunicator->leaveTable(currentTablenum);

}





void MainWindow::lexiconComboBoxIndexChanged(QString lex)
{
    uiCreateScrambleTable.listWidgetTopLevelList->clear();
    challengesMenu->clear();
    uiScores.comboBoxChallenges->clear();

    for (int i = 0; i < lexiconListsHash[lex].regularWordLists.size(); i++)
    {
        uiCreateScrambleTable.listWidgetTopLevelList->addItem(lexiconListsHash[lex].regularWordLists.at(i));
    }
    for (int i = 0; i < lexiconListsHash[lex].dailyWordLists.size(); i++)
    {
        challengesMenu->addAction(lexiconListsHash[lex].dailyWordLists.at(i));
        uiScores.comboBoxChallenges->addItem(lexiconListsHash[lex].dailyWordLists.at(i));
    }
    challengesMenu->addAction("Get today's scores");
    // gameBoardWidget->setDatabase(lexiconLists.at(index).lexicon);
    gameBoardWidget->setLexicon(lex);
    currentLexicon = lex;
    uiCreateScrambleTable.labelLexiconName->setText(currentLexicon);
    wordFilter->setCurrentLexicon(currentLexicon);
    repopulateMyListsTable();

}


void MainWindow::handleCreateTable(quint16 tablenum, quint8 gameType, QString lexiconName,
                                   QString tableName, quint8 maxPlayers, bool isPrivate)
{
    tableRepresenter* t = new tableRepresenter;
    t->tableNumItem = new QTableWidgetItem(QString("%1").arg(tablenum));
    t->descriptorItem = new QTableWidgetItem("(" + lexiconName +
                                             ") " + tableName);
    t->playersItem = new QTableWidgetItem("");
    t->maxPlayers = maxPlayers;
    t->isPrivate = isPrivate;
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
    t->buttonItem->setEnabled(!t->isPrivate);


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
        gameBoardWidget->addPlayer(player);

    else if (modification == -1)
        gameBoardWidget->removePlayer(player);


}



void MainWindow::giveUpOnThisGame()
{
    serverCommunicator->sendGiveup(currentTablenum);
}

void MainWindow::trySitting(quint8 seatNumber)
{
    serverCommunicator->trySitting(seatNumber, currentTablenum);
}

void MainWindow::standUp()
{
    serverCommunicator->standUp(currentTablenum);
}

void MainWindow::trySetTablePrivate(bool priv)
{
    serverCommunicator->trySetTablePrivate(currentTablenum, priv);

}

void MainWindow::submitReady()
{
    serverCommunicator->sendReady(currentTablenum);

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




void MainWindow::sendClientVersion()
{
    serverCommunicator->sendClientVersion(aerolithVersion);

}



void MainWindow::updateGameTimer()
{
    //if (gameBoardWidget->timerDial->value() == 0) return;

    //gameBoardWidget->timerDial->display(gameBoardWidget->timerDial->value() - 1);

}

void MainWindow::getScores()
{
    uiScores.scoresTableWidget->clearContents();
    uiScores.scoresTableWidget->setRowCount(0);
    serverCommunicator->requestHighScores(uiScores.comboBoxChallenges->currentText());

}

void MainWindow::registerName()
{

    // validate password
    if (uiLogin.desiredFirstPasswordLE->text() != uiLogin.desiredSecondPasswordLE->text())
    {
        QMessageBox::warning(this, "Aerolith", "Your passwords do not match! Please try again");
        uiLogin.desiredFirstPasswordLE->clear();
        uiLogin.desiredSecondPasswordLE->clear();
        return;

    }

    serverCommunicator->connectToServer(uiLogin.serverLE->text(), uiLogin.portLE->text().toInt(),
                                        uiLogin.desiredUsernameLE->text(), uiLogin.desiredFirstPasswordLE->text(),
                                        ServerCommunicator::MODE_REGISTER);


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
        uiCreateScrambleTable.spinBoxProb2->setMaximum(max);
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
    uiLogin.connectStatusLabel->setText("You can now play locally! Log in now.");
    uiLogin.pushButtonStartOwnServer->setText("Stop Server");
    uiLogin.serverLE->setText("localhost");
    uiLogin.portLE->setText(QString::number(DEFAULT_PORT));
}

void MainWindow::serverThreadHasFinished()
{

    uiLogin.connectStatusLabel->setText("Local server has stopped!");
    uiLogin.pushButtonStartOwnServer->setText("Start Own Server");

    uiLogin.serverLE->setText("aerolith.org");
    uiLogin.portLE->setText(QString::number(DEFAULT_PORT));
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

    if (probIndices.size() > REMOTE_LISTSIZE_LIMIT)
    {
        QMessageBox::warning(this, "Error", "This list contains more than " + QString::number(REMOTE_LISTSIZE_LIMIT)
                             + " alphagrams. If you would like to upload "
                             "a very large list please do so locally.");
        return;

    }

    serverCommunicator->uploadWordList(currentLexicon, probIndices, listName);
}

void MainWindow::repopulateMyListsTable()
{
    if (lexiconListsHash[currentLexicon].savedWordLists.size() == 0)
    {
        serverCommunicator->requestSavedWordListInfo(currentLexicon);
        qDebug() << "Requested saved word list info from server";
    }
    else
    {
        uiCreateScrambleTable.tableWidgetMyLists->clearContents();
        uiCreateScrambleTable.tableWidgetMyLists->setRowCount(0);
        //qDebug() << "Going to load saved word list info from hash, size"
          //      << lexiconListsHash[currentLexicon].savedWordLists.size();
        for (int i = 0; i < lexiconListsHash[currentLexicon].savedWordLists.size(); i++)
        {
            uiCreateScrambleTable.tableWidgetMyLists->insertRow(0);
            QStringList labels = lexiconListsHash[currentLexicon].savedWordLists.at(i);
          //  qDebug() << "Labels -------> " << labels;
            for (int j = 0; j < labels.size(); j++)
                uiCreateScrambleTable.tableWidgetMyLists->setItem(0, j, new QTableWidgetItem(labels[j]));
        }
        uiCreateScrambleTable.tableWidgetMyLists->resizeColumnsToContents();
    }


}

void MainWindow::showInviteDialog()
{
    QStringList playerList = peopleLoggedIn;
    playerList.removeAll(currentUsername);

    if (playerList.size() >= 1)
    {
        QString playerToInvite = QInputDialog::getItem(this, "Invite", "Select Player to Invite", playerList, 0, false);

        if (playerToInvite != "")
        {
            serverCommunicator->invitePlayerToTable(currentTablenum, playerToInvite);

        }
    }

}

void MainWindow::bootFromTable(QString playerToBoot)
{
    serverCommunicator->bootFromTable(currentTablenum, playerToBoot);

}

void MainWindow::saveGame()
{
    serverCommunicator->saveGame(currentTablenum);
}

void MainWindow::on_radioButtonProbability_clicked()
{
    uiCreateScrambleTable.stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_radioButtonOtherLists_clicked()
{
    uiCreateScrambleTable.stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_radioButtonMyLists_clicked()
{
    uiCreateScrambleTable.stackedWidget->setCurrentIndex(2);
    uiCreateScrambleTable.radioButtonContinueListQuiz->setChecked(true);

}

void MainWindow::on_pushButtonDeleteList_clicked()
{
    QList<QTableWidgetItem*> si = uiCreateScrambleTable.tableWidgetMyLists->selectedItems();
    if (si.size() != 5) return;

    if (QMessageBox::Yes == QMessageBox::warning(this, "Are you sure?", "Are you sure you wish to delete '" +
                                                 si[0]->text() + "'?",
                                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
    {
        serverCommunicator->deleteList(currentLexicon, si[0]->text());

    }

}

void MainWindow::acceptedInvite()
{
    QPushButton* p = static_cast<QPushButton*> (sender());
    quint16 tablenum = p->property("tablenum").toInt();

    serverCommunicator->joinTable(tablenum);

    p->parent()->deleteLater();
}

void MainWindow::declinedInvite()
{
    QPushButton* p = static_cast<QPushButton*> (sender());

    // send decline packet?


    p->parent()->deleteLater();
}


void MainWindow::showDonationPage()
{

    QString donationUrl = "https://www.paypal.com/us/cgi-bin/webscr?cmd=_flow&SESSION=HAV_kL7GVkuKQpwAI-"
                          "OnlVT3eB8uvWuz4wOzWMlMSSfn7NwCnQjZ-Uko92u&dispatch=50a222a57771920b6a3d7b60623"
                          "9e4d529b525e0b7e69bf0224adecfb0124e9b5efedb82468478c6e115945fd0658595b0be0417af"
                          "d2208f";

    QDesktopServices::openUrl(QUrl(donationUrl));
}

void MainWindow::on_actionSubmitSuggestion_triggered()
{
    if (!serverCommunicator->isConnectedToServer() ||
        uiLogin.serverLE->text() == "localhost")
    {
        QMessageBox::warning(this, "Log in", "Please log in to the main Aerolith server and then try "
                             "again.");
        return;
    }
    bool ok;
    //   QMessageBox::warning(this, "woo", "a suggestion to submit!");
    QString text =  QInputDialog::getText(this, "Suggestion/Bug report",
                                          "Enter your suggestion or bug report. Please "
                                          "be detailed and try to remember what triggered a bug.",
                                          QLineEdit::Normal,
                                          "",
                                          &ok);


    if (ok)
    {
        serverCommunicator->sendSuggestionOrBugReport(text.left(1000));
        QMessageBox::information(this, "Thank you", "Thank you for your input!");
    }
}

void MainWindow::on_comboBoxGameType_currentIndexChanged(QString text)
{
    if (text == "WordScramble")
    {
        uiMainWindow.pushButtonChallenges->setVisible(true);
    }
    else
        uiMainWindow.pushButtonChallenges->setVisible(false);

}

void MainWindow::sentLogin()
{
    currentUsername = uiLogin.usernameLE->text();

}

void MainWindow::userLoggedIn(QString username)
{
    QListWidgetItem *it = new QListWidgetItem(username, uiMainWindow.listWidgetPeopleConnected);
    peopleLoggedIn.append(username);
    if (username == currentUsername)
    {
        if (uiMainWindow.checkBoxSounds->isChecked())
            QSound::play("sounds/enter.wav");

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

void MainWindow::userLoggedOut(QString username)
{
    for (int i = 0; i < uiMainWindow.listWidgetPeopleConnected->count(); i++)
        if (uiMainWindow.listWidgetPeopleConnected->item(i)->text() == username)
        {
        QListWidgetItem *it = uiMainWindow.listWidgetPeopleConnected->takeItem(i);
        delete it;
    }
    peopleLoggedIn.removeAll(username);
}

void MainWindow::chatReceived(QString username, QString text)
{

    uiMainWindow.chatText->moveCursor(QTextCursor::End);
    uiMainWindow.chatText->insertHtml(QString("[")+username+"] " + text);
    uiMainWindow.chatText->append("");

}

void MainWindow::errorFromServer(QString errorString)
{
    QMessageBox::information(loginDialog, "Aerolith client", errorString);
    uiMainWindow.chatText->append("<font color=red>" + errorString + "</font>");

}

void MainWindow::serverConnectionError(QString errorString)
{
    QMessageBox::warning(this, "Error connecting", errorString);
    uiLogin.loginPushButton->setText("Connect");
    uiLogin.connectStatusLabel->setText("Disconnected.");
}

void MainWindow::serverDisconnected()
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

void MainWindow::playerJoinedTable(quint16 tablenum, QString playerName)
{
    qDebug() << playerName << "joined" << tablenum;

    if (!tables.contains(tablenum))
        return;

    tableRepresenter* t = tables.value(tablenum);
    t->playerList << playerName;
    QString textToSet = "";
    foreach(QString thisplayer, t->playerList)
    {
        textToSet += thisplayer + " ";

    }

    quint8 numPlayers = t->playerList.size();
    quint8 maxPlayers = t->maxPlayers;

    textToSet += "(" + QString::number(numPlayers) + "/" + QString::number(maxPlayers) + ")";

    t->playersItem->setText(textToSet);

    if (playerName == currentUsername)
    {

        currentTablenum = tablenum;
        if (!tables.contains(tablenum))
            return;
        tableRepresenter* t = tables.value(tablenum);
        gameBoardWidget->setTableCapacity(t->maxPlayers);
        QString wList = t->descriptorItem->text();
        qDebug() << "I joined table!";
        gameBoardWidget->resetTable(tablenum, wList, playerName);
        gameBoardWidget->show();
        gameBoardWidget->setTableNum(tablenum);
        trySitting(0);  // whenever player joins, they try sitting at spot 0.
        gameBoardWidget->setPrivacy(t->isPrivate);

        uiMainWindow.comboBoxLexicon->setEnabled(false);

    }
    if (currentTablenum == tablenum)
        modifyPlayerLists(tablenum, playerName, 1);
    //  chatText->append(QString("%1 has entered %2").arg(playerName).arg(tablenum));
}

void MainWindow::tablePrivacyChange(quint16 tablenum, bool privacy)
{

    if (tables.contains(tablenum))
    {
        tableRepresenter *t = tables.value(tablenum);
        t->isPrivate = privacy;
        t->buttonItem->setEnabled(!t->isPrivate);
        if (currentTablenum == tablenum)
            gameBoardWidget->setPrivacy(privacy);
    }
}

void MainWindow::tableInvite(quint16 tablenum, QString username)
{

    if (uiMainWindow.checkBoxIgnoreTableInvites->isChecked() == false)
    {
        Ui::inviteForm ui;
        QWidget* inviteWidget = new QWidget(this, Qt::Window);
        ui.setupUi(inviteWidget);
        inviteWidget->setAttribute(Qt::WA_DeleteOnClose);
        inviteWidget->show();

        ui.pushButtonAccept->setProperty("tablenum", tablenum);
        ui.pushButtonAccept->setProperty("host", username);

        ui.pushButtonDecline->setProperty("tablenum", tablenum);
        ui.pushButtonDecline->setProperty("host", username);

        connect(ui.pushButtonAccept, SIGNAL(clicked()), SLOT(acceptedInvite()));
        connect(ui.pushButtonDecline, SIGNAL(clicked()), SLOT(declinedInvite()));
        ui.labelInfo->setText(QString("You have been invited to table %1 by %2.").arg(tablenum).arg(username));
        //TODO watch out for abuses of this! (invite bombs?)
    }
    else
    {
    }   // TODO inform user that their invite didn't go through
}

void MainWindow::bootedFromTable(quint16 tablenum, QString username)
{
    // username is user that booted us.
    if (tablenum == currentTablenum)
        QMessageBox::information(this, "You have been booted",
                                 QString("You have been booted from table %1 by %2").arg(tablenum).arg(username));
}

void MainWindow::playerLeftTable(quint16 tablenum, QString username)
{
    if (currentTablenum == tablenum)
    {
        modifyPlayerLists(tablenum, username, -1);
    }

    if (username == currentUsername)
    {
        currentTablenum = 0;
        //gameStackedWidget->setCurrentIndex(0);
        gameBoardWidget->hide();
        uiMainWindow.comboBoxLexicon->setEnabled(true);
    }



    if (!tables.contains(tablenum))
        return;

    tableRepresenter* t = tables.value(tablenum);

    t->playerList.removeAll(username);
    QString textToSet="";
    foreach(QString thisplayer, t->playerList)
    {
        textToSet += thisplayer + " ";
    }

    int numPlayers = t->playerList.size();
    textToSet += "(" + QString::number(numPlayers) + "/" + QString::number(t->maxPlayers) + ")";
    t->playersItem->setText(textToSet);
}



void MainWindow::handleDeleteTable(quint16 tablenum)
{

    if (!tables.contains(tablenum))
        return;

    tableRepresenter* t = tables.value(tablenum);
    uiMainWindow.roomTableWidget->removeRow(t->tableNumItem->row()); // delete the whole row (this function deletes the elements)
    delete tables.take(tablenum); // delete the tableRepresenter object after removing it from the hash.
}

void MainWindow::gotServerMessage(QString message)
{
    uiMainWindow.chatText->append("<font color=red>" + message + "</font>");
    gameBoardWidget->gotChat("<font color=red>" + message + "</font>");
}

void MainWindow::beginUnscramblegameListData()
{
    uiCreateScrambleTable.tableWidgetMyLists->clearContents();
    uiCreateScrambleTable.tableWidgetMyLists->setRowCount(0);

    unscramblegameUserlistData_clearHash = true;
    // because we dont' know what lexicon we are beginning yet
        // TODO should change this behavior in the server.
}

void MainWindow::addUnscramblegameListData(QString lexicon, QStringList labels)
{
    qDebug() << "Add single data" << lexicon << labels;
    if (lexicon == currentLexicon)
    {
        if (unscramblegameUserlistData_clearHash)
        {
            lexiconListsHash[lexicon].savedWordLists.clear();
            unscramblegameUserlistData_clearHash = false;
        }

        uiCreateScrambleTable.tableWidgetMyLists->insertRow(0);
        for (int j = 0; j < labels.size(); j++)
        {
            uiCreateScrambleTable.tableWidgetMyLists->setItem(0, j, new QTableWidgetItem(labels[j]));
        }
        lexiconListsHash[lexicon].savedWordLists.append(labels);
    }
}

void MainWindow::doneUnscramblegameListData()
{
    uiCreateScrambleTable.tableWidgetMyLists->resizeColumnsToContents();
    unscramblegameUserlistData_clearHash = false;
}

void MainWindow::clearSingleUnscramblegameListData(QString lexicon, QString listname)
{
    bool foundInTable = false;
    bool foundInList = false;
    if (lexicon == currentLexicon) // ! TODO may be a bug
    {
        for (int i = 0; i < uiCreateScrambleTable.tableWidgetMyLists->rowCount(); i++)
        {
            if (uiCreateScrambleTable.tableWidgetMyLists->item(i, 0)->text() == listname && !foundInTable)
            {
                foundInTable = true;
                uiCreateScrambleTable.tableWidgetMyLists->removeRow(i);
            }
            if (i < lexiconListsHash[lexicon].savedWordLists.size() &&
                lexiconListsHash[lexicon].savedWordLists.at(i).at(0) == listname && !foundInList)
            {
                foundInList = true;
                lexiconListsHash[lexicon].savedWordLists.removeAt(i);
            }
            if (foundInList && foundInTable) break; // removed correctly, so break
        }
    }
}

void MainWindow::unscramblegameListSpaceUsage(quint32 usage, quint32 max)
{
    uiCreateScrambleTable.progressBarUsedListSpace->setRange(0, max);
    uiCreateScrambleTable.progressBarUsedListSpace->setValue(usage);

}

void MainWindow::gettingLexiconAndListInfo()
{
    disconnect(uiMainWindow.comboBoxLexicon, SIGNAL(currentIndexChanged(QString)), 0, 0);
    uiMainWindow.comboBoxLexicon->clear();
    lexiconListsHash.clear();

}

void MainWindow::gotLexicon(QByteArray lex)
{
    QString lexicon(lex);
    if (existingLocalDBList.contains(lexicon))
        uiMainWindow.comboBoxLexicon->addItem(lexicon);
    LexiconLists dummyLists;
    dummyLists.lexicon = lexicon;
    lexiconListsHash.insert(lexicon, dummyLists);
}

void MainWindow::addWordList(QByteArray lexicon, QByteArray listname, char type)
{
    if (type == (char)SERVER_WORD_LIST_REGULAR)
        lexiconListsHash[QString(lexicon)].regularWordLists << listname;
    else if (type == (char)SERVER_WORD_LIST_CHALLENGE)
        lexiconListsHash[QString(lexicon)].dailyWordLists << listname;
}

void MainWindow::doneGettingLexAndListInfo()
{

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


    connect(uiMainWindow.comboBoxLexicon, SIGNAL(currentIndexChanged(QString)),
            SLOT(lexiconComboBoxIndexChanged(QString)));

    spinBoxWordLengthChange(uiCreateScrambleTable.spinBoxWL->value());  // TODO why have it here?

}

void MainWindow::clearHighScoresTable()
{
    uiScores.scoresTableWidget->clearContents();
    uiScores.scoresTableWidget->setRowCount(0);
}

void MainWindow::newHighScore(int rank, QString username, double percCorrect, int timeRemaining)
{
    QTableWidgetItem* rankItem = new QTableWidgetItem(QString("%1").arg(rank+1));
    QTableWidgetItem* usernameItem = new QTableWidgetItem(username);
    QTableWidgetItem* correctItem = new QTableWidgetItem(QString("%1%").arg(percCorrect, 0, 'f', 1));
    QTableWidgetItem* timeItem = new QTableWidgetItem(QString("%1 s").arg(timeRemaining));
    uiScores.scoresTableWidget->insertRow(uiScores.scoresTableWidget->rowCount());
    uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 0, rankItem);
    uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 1, usernameItem);
    uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 2, correctItem);
    uiScores.scoresTableWidget->setItem(uiScores.scoresTableWidget->rowCount() -1, 3, timeItem);
}

void MainWindow::endHighScores()
{
    uiScores.scoresTableWidget->resizeColumnsToContents();
}

void MainWindow::gotTableChat(quint16 tablenum, QString username, QString chat)
{
    Q_UNUSED(tablenum);
    gameBoardWidget->gotChat("[" + username + "] " + chat);
}

void MainWindow::gotTableMessage(quint16 tablenum, QString message)
{
    Q_UNUSED(tablenum);
    gameBoardWidget->gotChat("<font color=green>" + message + "</font>");
}

void MainWindow::gotServerTableTimerValue(quint16 tablenum, quint16 timerVal)
{
    gameBoardWidget->gotTimerValue(timerVal);
}

void MainWindow::gotServerTableReadyBegin(quint16 tablenum, quint8 seat)
{
    gameBoardWidget->setReadyIndicator(seat);
    if (uiMainWindow.checkBoxSounds->isChecked())
        QSound::play("sounds/BEEPDOUB.WAV");
}

void MainWindow::gotServerTableGameStart(quint16 tablenum)
{
    gameBoardWidget->setupForGameStart();
    gameBoardWidget->gotChat("<font color=red>The game has started!</font>");
}

void MainWindow::gotServerTableAvatarChange(quint16 tablenum, quint8 seatNumber, quint8 avatarID)
{

     gameBoardWidget->setAvatar(seatNumber, avatarID);

    // then here we can do something like chatwidget->setavatar( etc). but this requires the server
    // to send avatars to more than just the table. so if we want to do this, we need to change the server behavior!
    // this way we can just send everyone's avatar on login. consider changing this!
}

void MainWindow::gotServerTableGameEnd(quint16 tablenum)
{
    // todo replace this with a virtual end game function
    gameBoardWidget->gotChat("<font color=red>This round is over.</font>");
    gameBoardWidget->populateSolutionsTable();
    gameBoardWidget->clearAllWordTiles();
}

void MainWindow::gotServerTableHost(quint16 tablenum, QString host)
{
    gameBoardWidget->setHost(host);
}

void MainWindow::gotServerTableSuccessfulStand(quint16 tablenum, QString username, quint8 seatNumber)
{
    gameBoardWidget->standup(username, seatNumber);
}

void MainWindow::gotServerTableSuccessfulSit(quint16 tablenum, QString username, quint8 seatNumber)
{
    gameBoardWidget->sitdown(username, seatNumber);
}

void MainWindow::gotServerTableGameEndRequest(quint16 tablenum, QString username)
{
    gameBoardWidget->gotChat("<font color=red>" + username + " gave up! </font>");
}

void MainWindow::gotSpecificTableCommand(quint16 tablenum, quint8 commandByte, QByteArray ba)
{
    gameBoardWidget->gotSpecificCommand(commandByte, ba);
}


/* dont understand packet
   QMessageBox::critical(this, "Aerolith client", "Don't understand this packet!");
   */

void MainWindow::badMagicNumber()
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

}

/////////////////////////////////////////////////////

PMWidget::PMWidget(QWidget* parent, QString localUsername, QString remoteUsername) :
        QWidget(parent), localUsername(localUsername), remoteUsername(remoteUsername)
{

    uiPm.setupUi(this);
    connect(uiPm.lineEdit, SIGNAL(returnPressed()), this, SLOT(readAndSendLEContents()));
    setWindowTitle("Conversation with " + remoteUsername);
}

void PMWidget::readAndSendLEContents()
{
    emit sendPM(remoteUsername, uiPm.lineEdit->text());
    uiPm.textEdit->append("[" + localUsername + "] " + uiPm.lineEdit->text());

    uiPm.lineEdit->clear();
    //QSound::play("sounds/outbound.wav");
}

void PMWidget::appendText(QString text)
{
    uiPm.textEdit->append(text);
}

void PMWidget::closeEvent(QCloseEvent* event)
{
    event->accept();
    emit shouldDelete();
}

////////////////////
/* debug */
void MainWindow::testFunction()
{
    QVector <int> a;
    a << 1 << 2 << 3;
    qDebug () << a.mid(4);
}
