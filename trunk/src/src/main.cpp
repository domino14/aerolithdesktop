#include <QCoreApplication>
#include "mainserver.h"


static const quint16 DEFAULT_PORT = 1988;

QTextStream *outFile = 0;

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

int main (int argc, char **argv)
{

  quint16 port = 0;
  bool ok = false;
  if (argc == 2)
    {
      port = QString(argv[1]).toInt(&ok);
    }
  if (!ok)
    {
      port = DEFAULT_PORT;
    }

  if (argc == 3)
    {
      if (QString(argv[2]) != "-nolog")
	{
	  QFile *log = new QFile(QString(argv[2]));
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

  QCoreApplication app(argc, argv);
  
  MainServer mainServer;
  mainServer.listen(QHostAddress::Any, port);
  qDebug() << "listening on port " << port;


  return app.exec();
  
}
