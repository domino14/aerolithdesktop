#include <QApplication>

#include "mainwindow.h"
#include "mainserver.h"
QTextStream *outFile = 0;


static const quint16 DEFAULT_PORT = 1988;

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
	  if (argc == 3)
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
	  
	  MainServer mainServer;
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


  MainWindow mainWin;
  mainWin.show();      
  return app.exec();  

}
