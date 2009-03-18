#ifndef _LISTMAKER_H_
#define _LISTMAKER_H_

#include <QtSql>

class ListMaker
{
        public:
    static QStringList lexiconList;
    static void sqlListMaker(QString queryString, QString listName, quint8 wordLength, QString lexiconName);
    static void createListDatabase();
    static void testDatabaseTime();
    static void createLexiconDatabase(int lexiconIndex);
};

#endif
