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
  QFile *log = new QFile("log.txt");
  if (log->open(QIODevice::WriteOnly))
  {
    outFile = new QTextStream (log);
    qInstallMsgHandler(myMessageOutput);
  }
  else
  {
    delete log;
    qDebug("Can't open log.txt file, all message will be output to debugger and console.");
  }

  QCoreApplication app(argc, argv);
  quint16 port = 0;
  bool ok = false;
  if (argc > 1)
    {
      port = QString(argv[1]).toInt(&ok);
    }
  if (!ok)
    {
      port = DEFAULT_PORT;
    }
  
  MainServer mainServer;
  mainServer.listen(QHostAddress::Any, port);
  qDebug() << "listening on port " << port;


  return app.exec();
  
}
