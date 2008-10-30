#include <QApplication>
#include <QtCore>
#include <QtGui>
#include "mainwindow.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  MainWindow mainWindow;

  return app.exec();
}
