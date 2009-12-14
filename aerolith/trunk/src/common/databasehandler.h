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


#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QThread>
#include <QtSql>
#include "dawg.h"

#include "SavedUnscrambleGame.h"


struct Alph
{
    QStringList words;
    int combinations;
    QString alphagram;
    Alph(QStringList w, int c, QString alph)
    {
        alphagram = alph; words = w; combinations = c;
    }
    Alph()
    {
    }

};

struct LexiconInfo
{
    QString lexiconName;
    QString wordsFilename;
    QMap<unsigned char, int> letterDist;
    QString dawgFilename, dawgRFilename;
    Dawg dawg, reverseDawg;
    QSqlDatabase db;
    QVector <int> alphagramsPerLength;
    QList <double> fullChooseCombos;    // copied from Zyzzyva
    QList<QList<double> > subChooseCombos; // ditto

    LexiconInfo(QString name, QString filename, QMap <unsigned char, int> d, QString df, QString drf)
    {
        lexiconName = name;
        wordsFilename = filename; letterDist = d;
        dawgFilename = df;
        dawgRFilename = drf;
        alphagramsPerLength.resize(16);  // 0-15 index
    }
    LexiconInfo()
    {
        alphagramsPerLength.resize(16);
    }
    void resetLetterDistributionVariables();
    double combinations(QString alphagram);

};


//Q_DECLARE_TYPEINFO(LexiconInfo, Q_MOVABLE_TYPE);

enum LessThans
{
    SPANISH_LESS_THAN, ENGLISH_LESS_THAN
        };

class DatabaseHandler : public QThread
{
    Q_OBJECT
public:

    DatabaseHandler()
    {
    }

    enum UserListQuizModes
    {
        MODE_CONTINUE, MODE_RESTART, MODE_FIRSTMISSED
    };


    void createLexiconMap(bool);
    void connectToDatabases(bool clientCall, QStringList dbList);
    void createLexiconDatabases(QStringList);
    QStringList checkForDatabases();
    QStringList availableDatabases;
    QMap<QString, LexiconInfo> lexiconMap;
    int getNumWordsByLength(QString lexiconName, int length);
    bool getProbIndices(QStringList, QString, QSet<quint32>&);
    QByteArray getSavedListArray(QString, QString);
    bool saveNewLists(QString lexiconName, QString listName, QSet <quint32>& probIndices);
    QList <QStringList> getListLabels(QString lexiconName);
    void deleteUserList(QString lexiconName, QString listName);

    void saveGameBA(QByteArray, QString, QString);


private:
     bool saveSingleList(QString lexiconName, QString listName, QSet <quint32>& probIndices);
    QSqlDatabase userlistsDb;
    enum SqlListMakerQueryTypes
    {
      ALPHAGRAM_QUERY, PROBABILITY_QUERY
    };

    QList<unsigned char> letterList;
    QStringList dbsToCreate;
    void run();

    void sqlListMaker(QString queryString, QString listName, quint8 wordLength,
                      QSqlDatabase& db, SqlListMakerQueryTypes queryType = PROBABILITY_QUERY);

    void createLexiconDatabase(QString lexiconName);

    QString reverse(QString);
    QString alphagrammize(QString, LessThans lessThan);
    int fact(int n);
    int nCr(int n, int r);

    QMap <unsigned char, int> getEnglishDist();
    QMap <unsigned char, int> getSpanishDist();
    void updateDefinitions(QHash<QString, QString>&, int, QSqlDatabase& db);
    QString followDefinitionLinks(QString, QHash<QString, QString>&, bool useFollow, int maxDepth);
    QString getSubDefinition(const QString& word, const QString& pos, QHash<QString, QString> &defHash);

signals:
    void setProgressMessage(QString);
    void setProgressValue(int);
    void setProgressRange(int, int);
    void enableClose(bool);
    void createdDatabase(QString);
};



#endif // DATABASEHANDLER_H
