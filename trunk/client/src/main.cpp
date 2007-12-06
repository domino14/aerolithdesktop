#include <QApplication>

#include "mainwindow.h"

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


        



int main(int argc, char *argv[])
{

  QApplication app(argc, argv);

 
QFile *log = new QFile("debug.txt");
if (log->open(QIODevice::WriteOnly))
{
	outFile = new QTextStream (log);
	qInstallMsgHandler(myMessageOutput);
}
else delete log;

#ifdef Q_WS_MAC
  qDebug() << "Mac defined.";
  QDir dir(QApplication::applicationDirPath());
  dir.cdUp();
  dir.cd("plugins");
  QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif





 MainWindow mainWin;
  mainWin.show();


  
  return app.exec();
}
