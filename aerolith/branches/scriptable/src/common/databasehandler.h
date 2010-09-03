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
    QSqlDatabase dbConn;
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

    DatabaseHandler();
    ~DatabaseHandler();
    QQueue <QByteArray> commandQueue;
    QMutex queueProtector;
    static QMutex dbListProtector;
    volatile bool shouldQuitThread;
    enum QueuedCommandHeader
    {
        CREATE_LEXICON_DATABASES = 1,
        REQUEST_PROB_INDICES_FOR_UPLOADED_WORDLIST = 2,
        GET_QUESTION_DATA = 3,
        SAVE_SINGLE_LIST = 4,
        GET_LIST_INFO = 5,
        DELETE_LIST = 6,
        DOES_LIST_EXIST = 7,
        GENERATE_QUIZ_ARRAY = 8

    };


    // static utility functions:
    static quint32 encodeProbIndex(quint32 probIndex, quint32 wordLength);
    static QStringList getAvailableDatabases(); // accessor function

    // other static functions. these are only called once when setting up the lexicon map and connecting to databases
    // at the very beginning (see main.cpp)
    static void createLexiconMap();
    static QMap<QString, LexiconInfo> lexiconMap;
    static QMap <unsigned char, int> getEnglishDist();
    static QMap <unsigned char, int> getSpanishDist();

    static int getNumWordsByLength(QString lexiconName, int length);

    QString getSavedListRelativePath(QString, QString, QString);
    QString getSavedListAbsolutePath(QString, QString, QString);

    //  bool saveNewLists(QString lexiconName, QString listName, QSet <quint32>& probIndices);
    QStringList getSingleListLabels(QString lexiconName, QString username, QString listname);



    bool saveGame(SavedUnscrambleGame, QString, QString, QString);


    void getListSpaceUsage(QString username, quint32& usage, quint32& max);


private:
    static QStringList availableDatabases;
    static QStringList findAvailableDatabases();

    bool savedListExists(QString lexicon, QString listName, QString username, quint16 tablenum, quint64 tableid, bool alsoEmit);
    void deleteUserList(QString lexiconName, QString listName, QString username);
    void saveSingleList(QString lexiconName, QString listName, QString username, QList <quint32> probIndices);
    void getAllListLabels(QString lexiconName, QString username);
    void processCommand(QByteArray);

    void generateUnscramblegameQuizArray(QString lexicon, QString listname,
                                            quint8 listType, quint8 userlistMode,
                                            quint16 tablenum, quint64 tableid, QString username);

    void createLexiconDatabases(QStringList);
    void getQuestionData(QByteArray, QString, int);

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


    void updateDefinitions(QHash<QString, QString>&, int, QSqlDatabase& db);
    QString followDefinitionLinks(QString, QHash<QString, QString>&, bool useFollow, int maxDepth);
    QString getSubDefinition(const QString& word, const QString& pos, QHash<QString, QString> &defHash);
    void getProbIndices(QStringList, QString, QString);
signals:
    void setProgressMessage(QString);
    void setProgressValue(int);
    void setProgressRange(int, int);
    void enableClose(bool);
    void createdDatabase(QString);

    void setMaxListSpaceUsage(int);
    void setCurListSpaceUsage(int);

    void returnProbIndices(QList<quint32>,QString, QString);

    void returnQuestionInfo(QByteArray, QByteArray, int);
    void returnAnswerInfo(QByteArray, int);

    void saveWordListFailed(QString username);
    void saveWordListSuccess(QString lexicon, QString listName, quint32 listSize, QString username);
    void wordListInfo(QString username, QString lexicon, QList<QStringList> labels);
    void deleteWordListFailed(QString username);
    void deleteWordListSuccess(QString lexicon, QString listname, QString username);
    void requestedListExists(bool exists, quint16 tablenum,quint64 tableid);
    void unscramblegameQuizArray(QList <quint32>, QList <quint32>, QByteArray, quint16 tablenum,quint64 tableid);
public slots:
    void enqueueListDeleteRequest(QString lexicon, QString listname, QString username);
    void enqueueListInfoRequest(QString lexicon, QString username);
    void enqueueSaveSingleList(QString, QString, QString, QList <quint32>);
    void enqueueProbIndicesRequest(QStringList, QString, QString);
    void enqueueCreateLexiconDatabases(QStringList);
    void enqueueGetQuestionData(QByteArray, QString, int);
    void enqueueOtherDatabaseRequest(QByteArray);
    void connectToAvailableDatabases();
};



#endif // DATABASEHANDLER_H
