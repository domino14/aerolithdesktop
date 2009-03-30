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


#include "listMaker.h"
#include <QTime>


QMap<QString, LexiconInfo> ListMaker::lexiconMap;

void ListMaker::createListDatabase()
{

    lexiconMap.clear();
    // creates a word list database.
    QMap <unsigned char, int> englishLetterDist = getEnglishDist();
    QMap <unsigned char, int> spanishLetterDist = getSpanishDist();

    lexiconMap.insert("OWL2+LWL", LexiconInfo("OWL2+LWL", "owl2-lwl.txt", englishLetterDist,
                                              "owl2-lwl.dwg", "owl2-lwl-r.dwg"));
/*    lexiconMap.insert("CSW", LexiconInfo("CSW", "csw.txt", englishLetterDist, "csw.dwg", "csw-r.dwg"));
    lexiconMap.insert("Volost", LexiconInfo("Volost", "volost.txt", englishLetterDist, "volost.dwg", "volost-r.dwg"));
    lexiconMap.insert("FISE", LexiconInfo("FISE", "fise.txt", spanishLetterDist, "fise.dwg", "fise-r.dwg"));
    lexiconMap.insert("OWL+LWL", LexiconInfo("OWL+LWL", "owl-lwl.txt", englishLetterDist, "owl-lwl.dwg", "owl-lwl-r.dwg"));*/
    foreach (QString key, lexiconMap.keys())
        createLexiconDatabase(key);


    //    QSqlQuery query(QSqlDatabase::database("fiseDB"));
    //    query.exec("create table if not exists words(word varchar(15))");
    //    query.exec("begin transaction");
    //    query.prepare("insert into words(word) values(?)");
    //    QFile file("fise.txt");
    //    if (!file.open(QIODevice::ReadOnly)) return;
    //    while (!file.atEnd())
    //    {
    //        QByteArray line = file.readLine();
    //        line = line.trimmed();
    //        QString l = QString::fromLatin1(line.constData(), line.size());
    //
    //        query.bindValue(0, l);
    //        query.exec();
    //    }
    //    qDebug() << "done mit list";
    //    query.exec("end transaction");
    //    wordDb.close();


}

QString ListMaker::reverse(QString word)
{
    /* reverses a word */
    QString ret;
    for (int i = word.length()-1; i >= 0; i--)
        ret += word[i];

    return ret;
}

void ListMaker::createLexiconDatabase(QString lexiconName)
{

    qDebug() << "Create" << lexiconName;
    LexiconInfo* lexInfo = &(lexiconMap[lexiconName]);
    lexInfo->dawg.readDawg("words/" + lexInfo->dawgFilename);
    lexInfo->reverseDawg.readDawg("words/" + lexInfo->dawgRFilename);



    QHash <QString, Alph> listHash;

    QFile file("words/" + lexInfo->wordsFilename);
    if (!file.open(QIODevice::ReadOnly)) return;

    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.length() > 0)
        {
            QStringList split = line.split(" ");
            QString word = split[0];
            QByteArray backHooks = lexInfo->dawg.findHooks(word.toAscii());
            QByteArray frontHooks = lexInfo->reverseDawg.findHooks(reverse(word).toAscii());

            qDebug() << frontHooks << word << backHooks;
        }
    }

    return;

    QSqlDatabase wordDb;
    wordDb = QSqlDatabase::addDatabase("QSQLITE", lexiconName + "DB");

    //QString dbFilename = "words/" + lexiconFilename;
    // wordDb.setDatabaseName(dbFilename);
    wordDb.open();

    QSqlQuery wordQuery(QSqlDatabase::database(lexiconName + "DB"));
    wordQuery.exec("CREATE TABLE IF NOT EXISTS words(alphagram VARCHAR(15), word VARCHAR(15), "
                   "definition VARCHAR(256), lexiconstrings VARCHAR(5), front_hooks VARCHAR(26), "
                   "back_hooks VARCHAR(26))");
    wordQuery.exec("CREATE TABLE IF NOT EXISTS alphagrams(alphagram VARCHAR(15), words VARCHAR(255), "
                   "probability INTEGER, length INTEGER)");

    wordQuery.exec("CREATE TABLE IF NOT EXISTS wordlists(listname VARCHAR(40), numalphagrams INTEGER, probindices BLOB)");


    QTime time;
    time.start();
    for (int i = 2; i <= 15; i++)
    {
        qDebug() << lexiconName << i;

        int probability = 1;
        //  bool nextSucceeded = zyzzyvaQuery.next();
        wordQuery.exec("BEGIN TRANSACTION");
        while (/*nextSucceeded*/ 1)
        {

            //    int num_anagrams = zyzzyvaQuery.value(4).toInt();
            QString alphagram;
            QString wordString = "";
            QString lexiconString = "";
            QString definitions, front_hooks, back_hooks;
            int numVowels;
            int numAnagrams;
            int numUniqueLetters;

            //            for (int n = 0; n < num_anagrams; n++)
            //            {
            //                alphagram = zyzzyvaQuery.value(1).toString();
            //                wordString += zyzzyvaQuery.value(0).toString() + " ";
            //                lexiconString += zyzzyvaQuery.value(2).toString() + "@";
            //                numVowels = zyzzyvaQuery.value(3).toInt();
            //                numAnagrams = zyzzyvaQuery.value(4).toInt();
            //                numUniqueLetters = zyzzyvaQuery.value(5).toInt();
            //                definitions += zyzzyvaQuery.value(6).toString() + "@";
            //                front_hooks += zyzzyvaQuery.value(7).toString() + "@";
            //                back_hooks += zyzzyvaQuery.value(8).toString() + "@";
            //                nextSucceeded = zyzzyvaQuery.next();
            //
            //            }
            int wordlength = alphagram.length();
            QString toExecute = "INSERT INTO alphagrams(alphagram, words, length, probability, num_vowels, lexiconstrings, "
                                "num_anagrams, num_unique_letters, lexiconName, definitions, front_hooks, back_hooks) "
                                "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
            wordQuery.prepare(toExecute);
            wordQuery.bindValue(0, alphagram);
            wordQuery.bindValue(1, wordString.trimmed());
            wordQuery.bindValue(2, wordlength);
            wordQuery.bindValue(3, probability + (wordlength << 24));
            wordQuery.bindValue(4, numVowels);
            wordQuery.bindValue(5, lexiconString);
            wordQuery.bindValue(6, numAnagrams);
            wordQuery.bindValue(7, numUniqueLetters);
            wordQuery.bindValue(8, lexiconName);
            wordQuery.bindValue(9, definitions);
            wordQuery.bindValue(10, front_hooks);
            wordQuery.bindValue(11, back_hooks);
            probability++;

            wordQuery.exec();

            // if (!nextSucceeded) break;
        }
        wordQuery.exec("END TRANSACTION");
    }

    qDebug() << "Created alphas in" << time.elapsed() << "for lexicon" << lexiconName;

    //wordQuery.exec("CREATE INDEX listname_index on wordlists(listname)");
    wordQuery.exec("CREATE UNIQUE INDEX probability_index on alphagrams(probability)");
    wordQuery.exec("CREATE UNIQUE INDEX word_index on words(word)");


    //
    //
    //    QVector <int> pick;
    //    /* top 500 by prob 6s - 9s */
    //    pick.append(7);
    //    pick.append(8);
    //    pick.append(6);
    //    pick.append(9);
    //    wordQuery.exec("BEGIN TRANSACTION");
    //    foreach (int i, pick)
    //    {
    //        int probIndex = 1;
    //
    //        int modifiedProbIndex = 1 + (i << 24); // i << 24 is basically encoding the length of the alphagram in this number
    //        // this makes it easier to have different anagram lengths for a table.
    //        int numAlphagrams = 0;
    //        wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1 and lexiconName = '%2'").
    //                       arg(i).arg(lexiconName));
    //
    //        while (wordQuery.next())
    //        {
    //            numAlphagrams = wordQuery.value(0).toInt();
    //        }
    //        qDebug() << "Count:" << i << numAlphagrams;
    //
    //        int listLength = 500;
    //        int actualListLength;
    //        do
    //        {
    //            QByteArray ba;
    //            QDataStream baWriter(&ba, QIODevice::WriteOnly);
    //
    //            wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1 and probability between %2 and %3"
    //                                   " and lexiconName = '%4'").
    //                           arg(i).arg(modifiedProbIndex).arg(modifiedProbIndex + listLength -1).arg(lexiconName));
    //
    //            while (wordQuery.next())
    //                actualListLength = wordQuery.value(0).toInt();
    //            //    qDebug() << " .." << actualListLength;
    //
    //            if (actualListLength == 0) continue;
    //            baWriter << (quint8)0 << (quint8)i << (quint32)(modifiedProbIndex) << (quint32)(modifiedProbIndex+actualListLength-1);
    //            // (quint8)0 means this is a RANGE of indices, and not a list of indices
    //            // second param is word length.
    //
    //
    //            QString toExecute = "INSERT INTO wordlists(listname, wordlength, numalphagrams, probindices, lexiconName) "
    //                                "VALUES(?, ?, ?, ?, ?)";
    //            wordQuery.prepare(toExecute);
    //            wordQuery.bindValue(0, QString("Useful %1s (%2 - %3)").arg(i).arg(probIndex).arg(probIndex + actualListLength-1));
    //            wordQuery.bindValue(1, i);
    //            wordQuery.bindValue(2, actualListLength);
    //            wordQuery.bindValue(3, ba);
    //            wordQuery.bindValue(4, lexiconName);
    //            wordQuery.exec();
    //
    //            probIndex+= 500;
    //            modifiedProbIndex += 500;
    //
    //        } while (probIndex < numAlphagrams);
    //    }
    //
    //    for (int i = 2; i <= 15; i++)
    //    {
    //        int numAlphagrams;
    //        QByteArray ba;
    //        QDataStream baWriter(&ba, QIODevice::WriteOnly);
    //        wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1 and lexiconName = '%2'").arg(i).arg(lexiconName));
    //        while (wordQuery.next())
    //        {
    //            numAlphagrams = wordQuery.value(0).toInt();
    //        }
    //        if (numAlphagrams == 0) continue;
    //        baWriter << (quint8)0 << (quint8)i << (quint32)(1 + (i << 24)) << (quint32)(numAlphagrams + (i << 24));
    //        QString toExecute = "INSERT INTO wordlists(listname, wordlength, numalphagrams, probindices, lexiconName) "
    //                            "VALUES(?, ?, ?, ?, ?)";
    //        wordQuery.prepare(toExecute);
    //        wordQuery.bindValue(0, QString(lexiconName+ " %1s").arg(i));
    //        wordQuery.bindValue(1, i);
    //        wordQuery.bindValue(2, numAlphagrams);
    //        wordQuery.bindValue(3, ba);
    //        wordQuery.bindValue(4, lexiconName);
    //        wordQuery.exec();
    //    }
    //
    //    QString vowelQueryString = "SELECT probability from alphagrams where length = %1 and num_vowels = %2 and lexiconName = '%3'";
    //    sqlListMaker(vowelQueryString.arg(8).arg(5).arg(lexiconName), "Five-vowel-8s", 8, lexiconName);
    //    sqlListMaker(vowelQueryString.arg(7).arg(4).arg(lexiconName), "Four-vowel-7s", 7, lexiconName);
    //    QString jqxzQueryString = "SELECT probability from alphagrams where length = %1 and lexiconName = '%2' and "
    //                              "(alphagram like '%Q%' or alphagram like '%J%' or alphagram like '%X%' or alphagram like '%Z%')";
    //
    //    for (int i = 4; i <= 8; i++)
    //        sqlListMaker(jqxzQueryString.arg(i).arg(lexiconName), QString("JQXZ %1s").arg(i), i, lexiconName);
    //
    //    if (lexiconName == "OWL2+LWL")
    //    {
    //        QString newWordsQueryString = "SELECT probability from alphagrams where length = %1 and lexiconName = '%2' and "
    //                                      "lexiconstrings like '%*%%' ESCAPE '*'";
    //        for (int i = 7; i <= 8; i++)
    //            sqlListMaker(newWordsQueryString.arg(i).arg(lexiconName), QString("New (OWL2) %1s").arg(i), i, lexiconName);
    //    }
    //    else if (lexiconName == "CSW")
    //    {
    //        QString newWordsQueryString = "SELECT probability from alphagrams where length = %1 and lexiconName = '%2' and "
    //                                      "lexiconstrings like '%#%'";
    //        for (int i = 7; i <= 8; i++)
    //            sqlListMaker(newWordsQueryString.arg(i).arg(lexiconName), QString("CSW-only %1s").arg(i), i, lexiconName);
    //    }
    //
    //    QString singleAnagramsQueryString = "SELECT probability from alphagrams where length = %1 and num_anagrams = 1 and lexiconName = '%2'";
    //    QString moreThanOneQueryString = "SELECT probability from alphagrams where length = %1 and num_anagrams > 1 and lexiconName = '%2'";
    //
    //    for (int i = 7; i <= 8; i++)
    //    {
    //        sqlListMaker(singleAnagramsQueryString.arg(i).arg(lexiconName), QString("One-anagram %1s").arg(i), i, lexiconName);
    //        sqlListMaker(moreThanOneQueryString.arg(i).arg(lexiconName), QString("Multi-anagram %1s").arg(i), i, lexiconName);
    //    }
    //
    //    QString uniqueLettersQueryString = "SELECT probability from alphagrams where num_unique_letters = %1 and length = %2 and lexiconName = '%3'";
    //    for (int i = 7; i <= 8; i++)
    //        sqlListMaker(uniqueLettersQueryString.arg(i).arg(i).arg(lexiconName), QString("Unique-letter %1s").arg(i), i, lexiconName);
    //
    //    wordQuery.exec("END TRANSACTION");
    //    zyzzyvaDb.close();
    //    QSqlDatabase::removeDatabase("zyzzyvaDB");
}

void ListMaker::sqlListMaker(QString queryString, QString listName, quint8 wordLength, QString lexiconName)
{
    //    QSqlQuery wordQuery(QSqlDatabase::database(WORD_DATABASE_NAME));
    //    wordQuery.exec(queryString);
    //    QVector <quint32> probIndices;
    //
    //    while (wordQuery.next())
    //    {
    //        probIndices.append(wordQuery.value(0).toInt());
    //    }
    //    qDebug() << listName << "found" << probIndices.size();
    //    if (probIndices.size() == 0) return;
    //
    //    QByteArray ba;
    //    QDataStream baWriter(&ba, QIODevice::WriteOnly);
    //
    //    baWriter << (quint8)1 << (quint8)wordLength << (quint16)probIndices.size();
    //
    //    // (quint8)1 means this is a LIST of indices
    //    // second param is word length.
    //    // third param is number of indices
    //    foreach(quint32 index, probIndices)
    //        baWriter << index;
    //
    //    QString toExecute = "INSERT INTO wordlists(listname, wordlength, numalphagrams, probindices, lexiconName) "
    //                        "VALUES(?,?,?,?,?)";
    //    wordQuery.prepare(toExecute);
    //    wordQuery.bindValue(0, listName);
    //    wordQuery.bindValue(1, wordLength);
    //    wordQuery.bindValue(2, probIndices.size());
    //    wordQuery.bindValue(3, ba);
    //    wordQuery.bindValue(4, lexiconName);
    //    wordQuery.exec();


}

int ListMaker::fact(int n)
{
    if (n == 0)
        return 1;
    return fact(n-1)*n;
}

int ListMaker::nCr(int n, int r)
{
    if (n < r)
        return 0;
    if (n == r)
        return 1;
    return (fact(n) / (fact(n-r) * fact(r)));


}

int ListMaker::combinations(QString alphagram, QMap <unsigned char, int> letterDist)
{
    int temp = 1;
    QMapIterator<unsigned char, int> i(letterDist);

    while (i.hasNext())
    {
        i.next();
        int ct = alphagram.count(QChar(i.key()));
        if (ct > 0)
            temp *= nCr(letterDist.value(i.key()), ct);
    }
    return temp;
}

QMap <unsigned char, int> ListMaker::getEnglishDist()
{
    QMap <unsigned char, int> dist;


    dist.insert('A', 9); dist.insert('B', 2); dist.insert('C', 2);
    dist.insert('D', 4); dist.insert('E', 12); dist.insert('F', 2);
    dist.insert('G', 3); dist.insert('H', 2); dist.insert('I', 9);
    dist.insert('J', 1); dist.insert('K', 1); dist.insert('L', 4);
    dist.insert('M', 2); dist.insert('N', 6); dist.insert('O', 8);
    dist.insert('P', 2); dist.insert('Q', 1); dist.insert('R', 6);
    dist.insert('S', 4); dist.insert('T', 6); dist.insert('U', 4);
    dist.insert('V', 2); dist.insert('W', 2); dist.insert('X', 1);
    dist.insert('Y', 2); dist.insert('Z', 1);
    return dist;
}

QMap <unsigned char, int> ListMaker::getSpanishDist()
{
    QMap <unsigned char, int> dist;
    dist.insert('1', 1); dist.insert('2', 1); dist.insert('3', 1);
    dist.insert('A', 12); dist.insert('B', 2); dist.insert('C', 4);
    dist.insert('D', 5); dist.insert('E', 12); dist.insert('F', 1);
    dist.insert('G', 2); dist.insert('H', 2); dist.insert('I', 6);
    dist.insert('J', 1); dist.insert('L', 4); dist.insert('M', 2);
    dist.insert('N', 5); dist.insert(0xF1, 1); dist.insert('O', 9);
    dist.insert('P', 2); dist.insert('Q', 1); dist.insert('R', 5);
    dist.insert('S', 6); dist.insert('T', 4); dist.insert('U', 5);
    dist.insert('V', 1); dist.insert('X', 1); dist.insert('Y', 1);
    dist.insert('Z', 1);
    return dist;
}
/*
int main(int argc, char**argv)
{
    QCoreApplication app(argc, argv);
    ListMaker::createListDatabase();

    
    Dawg dawg;
    dawg.readDawg("words/owl2-lwl.dwg");
    qDebug() << dawg.findHooks("easting");
    dawg.readDawg("words/owl2-lwl-r.dwg");
    qDebug() << dawg.findHooks("gnitsae");


    dawg.checkDawg("words/fise.txt");

    dawg.readDawg("words/fise-r.dwg");
    dawg.checkDawg("fiseSortedReversed.txt");

    dawg.readDawg("words/owl2-lwl.dwg");
    dawg.checkDawg("words/owl2-lwl.txt");
    dawg.readDawg("words/owl2-lwl-r.dwg");
    dawg.checkDawg("owl2-lwlReversed.txt");
    dawg.readDawg("words/owl-lwl.dwg");
    dawg.checkDawg("words/owl-lwl.txt");
    dawg.readDawg("words/owl-lwl-r.dwg");
    dawg.checkDawg("owl-lwlReversed.txt");

    dawg.readDawg("words/volost.dwg");
    dawg.checkDawg("words/volost.txt");
    dawg.readDawg("words/volost-r.dwg");
    dawg.checkDawg("volostr.txt");

     dawg.readDawg("words/csw.dwg");
    dawg.checkDawg("words/csw.txt");
    dawg.readDawg("words/csw-r.dwg");
    dawg.checkDawg("cswReversed.txt");




    return app.exec();

}
*/
