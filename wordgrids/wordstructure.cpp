#include "wordstructure.h"
#include <QtSql>


WordStructure::WordStructure(QObject* parent) : QThread(parent)
{
}

void WordStructure::loadWordStructure()
{
    if (!isRunning())
    {

        currentThreadOption = GENERATE_WORD_HASH;
        start();
    }
}

void WordStructure::run()
{

    if (currentThreadOption == GENERATE_WORD_HASH)
    {
        generateWordHash();
        emit finishedLoadingWordStructure();
    }
    else
    {

    }

}

void WordStructure::generateWordHash()
{
    wordStructure.clear();
    QSqlDatabase wordDb = QSqlDatabase::addDatabase("QSQLITE");
    wordDb.setDatabaseName("OWL2+LWL.db");
    wordDb.open();

    QSqlQuery query;
    query.exec("BEGIN TRANSACTION");
    query.prepare("SELECT alphagram, words from alphagrams");
    query.exec();

    int numResults = 0;
    while (query.next())
    {
        numResults++;
        wordStructure.insert(query.value(0).toString(), query.value(1).toString());


    }
    wordDb.close();

}
