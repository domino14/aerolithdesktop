//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Wordgrids.
//
//    Wordgrids is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Wordgrids is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Wordgrids.  If not, see <http://www.gnu.org/licenses/>.

#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QTextStream>

QTextStream *outFile = 0;

void myMessageOutput(QtMsgType type, const char *msg)
{
    QString debugdate =
            QDateTime::currentDateTime().toString(QLatin1String("[dd.MM.yy hh:mm:ss.zzz] "));
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
    QFile *log = new QFile("debug.txt");
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

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
