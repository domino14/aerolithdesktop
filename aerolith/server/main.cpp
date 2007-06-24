#include <QCoreApplication>
#include "mainserver.h"


static const quint16 DEFAULT_PORT = 1988;

int main (int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  quint16 port = 0;
  bool ok = false;
  if (argc > 0)
    {
      port = QString(argv[0]).toInt(&ok);
    }
  if (!ok)
    {
      port = DEFAULT_PORT;
    }
  MainServer mainServer;
  mainServer.listen(QHostAddress::Any, port);
  


  return app.exec();
  
}
