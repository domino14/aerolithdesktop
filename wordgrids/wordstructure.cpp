#include "wordstructure.h"
#include <QtSql>


WordStructure::WordStructure(QObject* parent) : QThread(parent)
{

}

void WordStructure::loadWordStructure()
{
    wordDb = QSqlDatabase::addDatabase("QSQLITE");
    wordDb.setDatabaseName("OWL2+LWL.db");
    wordDb.open();
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

QString WordStructure::getDefinitions(QString words)
{
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION");
    query.prepare("SELECT definition from words where word = ?");

    QString retString;
    QStringList wList = words.split(" ");
    foreach (QString w, wList)
    {
        retString += w + ": ";
        query.addBindValue(w);
        query.exec();
        while (query.next())
        {
            retString += query.value(0).toString();
        }
        retString += "\n\n";
    }

    return retString;


}

bool WordStructure::containsOnlyPlurals(QString words)
{
    QSqlQuery query;
    query.exec("BEGIN TRANSACTION");
    query.prepare("SELECT definition from words where word = ?");

    bool onlyPlurals = true;
    QStringList wList = words.split(" ");
    foreach (QString w, wList)
    {
        query.addBindValue(w);
        query.exec();
        query.next();
        QString def = query.value(0).toString();
        qDebug() << w << def << w.left(w.length()-1) << def.startsWith(w.left(w.length()-1));

        QString leftSide = w.left(w.length()-1);
        QString rightSide = w.right(1);

        if (! (def.startsWith(leftSide) && rightSide == "S") )
        {
            onlyPlurals = false;
            qDebug() << "FALSE!";
        }
    }

    // TODO i think this is broken for stuff like the following:
    // LABS is on the board and I add an S to make SLABS. that's not pluralizing but it would fail this test.
    return onlyPlurals;
}

void WordStructure::generateWordHash()
{
    wordStructure.clear();

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

}
