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

#include <QApplication>

#include "mainwindow.h"
#include "databasehandler.h"
#include "serverthread.h"
QTextStream *outFile = 0;

const QString aerolithVersion = "0.9";
void myMessageOutput(QtMsgType type, const char *msg)
{
    QString debugdate =
            QDateTime::currentDateTime().toString(QLatin1String("[dd.MM.yy hh:mm:ss] "));
    switch (type)
    {
    case QtDebugMsg:
        debugdate += "[D]";
        break;
    case QtWarningMsg:
        debugdate += "[W]";
        break;
    case QtCriticalMsg:
        debugdate += "[C]";
        break;
    case QtFatalMsg:
        debugdate += "[F]";
    }

    (*outFile) << debugdate << " " << msg << endl;
}

void setupApplicationParams()
{

#ifdef Q_WS_MAC
    qDebug() << "Mac defined.";
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    QCoreApplication::setOrganizationName("CesarWare");
    QCoreApplication::setOrganizationDomain("aerolith.org");
    QCoreApplication::setApplicationName("Aerolith");
    QSettings settings;




}
void dbClientConnects(DatabaseHandler* dbHandler, MainWindow* mainWin);
void dbServerConnects(DatabaseHandler* dbHandler, MainServer* mainServer);


int main(int argc, char *argv[])
{
    // /.programname server 1988 logfilename
    qDebug() << "Args: ";
    for (int i = 0; i < argc; i++)
        qDebug() << QString(argv[i]);

    if (argc > 1)
    {


        if (QString(argv[1]) == "server")
        {

            QCoreApplication app(argc, argv);

            bool ok = false;
            quint16 port;
            if (argc >= 3)
            {
                port = QString(argv[2]).toInt(&ok);
                if (!ok)
                    port = DEFAULT_PORT;
            }
            if (argc == 4)
            {
                if (QString(argv[3]) != "-nolog")
                {
                    QFile *log = new QFile(QString(argv[3]));
                    if (log->open(QIODevice::WriteOnly))
                    {
                        outFile = new QTextStream (log);
                        qInstallMsgHandler(myMessageOutput);
                    }
                    else
                    {
                        delete log;
                        qDebug("Can't open log file, all message will be output to debugger and console.");

                    }



                }


            }

            DatabaseHandler databaseHandler;
            DatabaseHandler::createLexiconMap();
            databaseHandler.connectToAvailableDatabases();

            MainServer mainServer(aerolithVersion);
            mainServer.listen(QHostAddress::Any, port);
            qDebug() << "listening on port " << port;
            return app.exec();
        }
    }

    QApplication app(argc, argv);
    setupApplicationParams();
    QFile *log = new QFile("debug.txt");
    if (log->open(QIODevice::WriteOnly))
    {
        outFile = new QTextStream (log);
        qInstallMsgHandler(myMessageOutput);
    }
    else delete log;

    DatabaseHandler databaseHandler;
    DatabaseHandler::createLexiconMap();
    databaseHandler.connectToAvailableDatabases();

    MainServer mainServer(aerolithVersion);
    MainWindow mainWin(aerolithVersion);


    QObject::connect(&mainWin, SIGNAL(startServer()), &mainServer, SLOT(init()));
    QObject::connect(&mainWin, SIGNAL(stopServer()), &mainServer, SLOT(deactivate()));
    QObject::connect(&mainServer, SIGNAL(readyToConnect()), &mainWin, SLOT(serverHasStarted()));
    QObject::connect(&mainServer, SIGNAL(finished()), &mainWin, SLOT(serverHasFinished()));

    /* client-database connects*/
    QObject::connect(&mainWin, SIGNAL(probIndicesRequest(QStringList, QString, QString)),
        &databaseHandler, SLOT(enqueueProbIndicesRequest(QStringList, QString, QString)));
    QObject::connect(&databaseHandler, SIGNAL(returnProbIndices(QList<quint32>,QString, QString)),
                     &mainWin, SLOT(gotProbIndices(QList<quint32>,QString,QString)));

    QObject::connect(&databaseHandler, SIGNAL(enableClose(bool)),
                     &mainWin, SLOT(dbDialogEnableClose(bool)));
    QObject::connect(&databaseHandler, SIGNAL(createdDatabase(QString)),
                     &mainWin, SLOT(databaseCreated(QString)));
    QObject::connect(&databaseHandler, SIGNAL(setProgressMessage(QString)),
                     &mainWin, SLOT(setProgressMessage(QString)));
    QObject::connect(&databaseHandler, SIGNAL(setProgressValue(int)),
                     &mainWin, SLOT(setProgressValue(int)));
    QObject::connect(&databaseHandler, SIGNAL(setProgressRange(int,int)),
                     &mainWin, SLOT(setProgressRange(int, int)));

    QObject::connect(&mainWin, SIGNAL(reconnectToDatabases()),
                     &databaseHandler, SLOT(connectToAvailableDatabases()));
    QObject::connect(&mainWin, SIGNAL(createLexiconDatabases(QStringList)),
                     &databaseHandler, SLOT(enqueueCreateLexiconDatabases(QStringList)));
    QObject::connect(&mainWin, SIGNAL(requestQuestionData(QByteArray, QString, int)),
                     &databaseHandler, SLOT(enqueueGetQuestionData(QByteArray, QString, int)));

    QObject::connect(&databaseHandler, SIGNAL(returnQuestionInfo(QByteArray,QByteArray,int)),
                     &mainWin, SLOT(getUnscrambleGameQuestionInfo(QByteArray, QByteArray, int)));
    QObject::connect(&databaseHandler, SIGNAL(returnAnswerInfo(QByteArray,int)),
                     &mainWin, SLOT(getUnscrambleGameAnswerInfo(QByteArray, int)));


    mainWin.show();
    return app.exec();

}

/* how to make aerolith logo:

supernova effect (in gimp):
color 06385f
X 37 Y 4
Radius 23
Spokes 79
Random Hue 6

*/
