#include <QCoreApplication>
#include "mainserver.h"

int main (int argc, char **argv)
{
  qDebug("here");
  QCoreApplication app(argc, argv);
  qDebug("andhere");
  MainServer mainServer(0);
  qDebug("y aqui");
  app.exec();
  return 0;
}
