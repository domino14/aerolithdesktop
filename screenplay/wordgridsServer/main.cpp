#include <QtCore/QCoreApplication>
#include "mainserver.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MainServer mainServer;
    return a.exec();
}
