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
#include "serverthread.h"
QTextStream *outFile = 0;


static const quint16 DEFAULT_PORT = 1988;
const QString aerolithVersion = "0.4.1";
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

int main(int argc, char *argv[])
{
  // /.programname server 1988 logfilename
  if (argc > 1)
    {
      qDebug() << "Args: ";
      for (int i = 0; i < argc; i++)
	qDebug() << QString(argv[i]);
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
		      qDebug("Can't open log file, all message will be output to ebugger and console.");

		    }



		}


	    }

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

  ServerThread serverThread(aerolithVersion);
  MainWindow mainWin(aerolithVersion);

  QObject::connect(&mainWin, SIGNAL(startServerThread()), &serverThread, SLOT(startThread()));
  QObject::connect(&mainWin, SIGNAL(stopServerThread()), &serverThread, SLOT(stopThread()));
  QObject::connect(&serverThread, SIGNAL(readyToConnect()), &mainWin, SLOT(serverThreadHasStarted()));
  QObject::connect(&serverThread, SIGNAL(finished()), &mainWin, SLOT(serverThreadHasFinished()));
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