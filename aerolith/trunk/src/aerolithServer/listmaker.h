
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

#ifndef _LISTMAKER_H_
#define _LISTMAKER_H_

#include <QtSql>
#include "dawg.h"


struct Alph
{
    QStringList words;
    int combinations;
    Alph(QStringList w, int c)
    {
        words = w; combinations = c;
    }

};

struct LexiconInfo
{
    QString lexiconName;
    QString wordsFilename;
    QMap<unsigned char, int> letterDist;
    QString dawgFilename, dawgRFilename;
    Dawg dawg, reverseDawg;
    LexiconInfo(QString name, QString filename, QMap <unsigned char, int> d, QString df, QString drf)
    {
        lexiconName = name;
        wordsFilename = filename; letterDist = d;
        dawgFilename = df;
        dawgRFilename = drf;
    }
    LexiconInfo()
    {
    }
};

class ListMaker
{
        public:
    static QMap<QString, LexiconInfo> lexiconMap;

    static void sqlListMaker(QString queryString, QString listName, quint8 wordLength, QString lexiconName);
    static void createListDatabase();
    static void createLexiconDatabase(QString lexiconName);
    static QString reverse(QString);
    static int fact(int n);
    static int nCr(int n, int r);
    static int combinations(QString alphagram, QMap <unsigned char, int> letterDist);
    static QMap <unsigned char, int> getEnglishDist();
    static QMap <unsigned char, int> getSpanishDist();

};

#endif

