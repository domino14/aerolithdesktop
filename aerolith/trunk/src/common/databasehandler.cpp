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


#include "databasehandler.h"
#include <QTime>

QVector<double> DatabaseHandler::testVector;
QMap<QString, LexiconInfo> DatabaseHandler::lexiconMap;


bool probLessThan(const Alph &a1, const Alph &a2)
{
    return a1.combinations > a2.combinations;
}

void DatabaseHandler::createLexiconMap()
{
    /* this function is called right when the Aerolith program starts executing, either in stand-alone server mode
       or in server-client mode*/
    lexiconMap.clear();
    // creates a word list database.
    QMap <unsigned char, int> englishLetterDist = getEnglishDist();
    QMap <unsigned char, int> spanishLetterDist = getSpanishDist();

    lexiconMap.insert("OWL2+LWL", LexiconInfo("OWL2+LWL", "owl2-lwl.txt", englishLetterDist,
                                              "owl2-lwl.dwg", "owl2-lwl-r.dwg"));
    lexiconMap.insert("CSW", LexiconInfo("CSW", "csw.txt", englishLetterDist, "csw.dwg", "csw-r.dwg"));
    lexiconMap.insert("Volost", LexiconInfo("Volost", "volost.txt", englishLetterDist, "volost.dwg", "volost-r.dwg"));
    lexiconMap.insert("FISE", LexiconInfo("FISE", "fise.txt", spanishLetterDist, "fise.dwg", "fise-r.dwg"));


}

void DatabaseHandler::connectToAvailableDatabases(bool clientCall)
{
    foreach (QString key, lexiconMap.keys())
    {
        LexiconInfo* lexInfo = &(lexiconMap[key]);
        qDebug() << "Name:" << lexInfo->lexiconName;
        if (clientCall)
        {
            qDebug() <<QThread::currentThread();
            qDebug() << QCoreApplication::instance()->thread();
            lexInfo->clientSideDb =  QSqlDatabase::addDatabase("QSQLITE", key + "DB_client");
       //     lexInfo->clientSideDb.setDatabaseName(key + ".db");
       //     lexInfo->clientSideDb.open();
        }
        else
        {
              lexInfo->serverSideDb =  QSqlDatabase::addDatabase("QSQLITE", key + "DB_server");
            lexInfo->serverSideDb.setDatabaseName(key + ".db");
            lexInfo->serverSideDb.open();
        }
    }
}


void DatabaseHandler::createLexiconDatabases(QStringList lexiconNames)
{
    setProgressMessage("Creating lexicon databases...");
    qDebug() << "In here";

    if (!isRunning())
    {
        dbsToCreate = lexiconNames;
        start();


    }
}

void DatabaseHandler::run()
{
    foreach (QString key, dbsToCreate)
    {
        if (!lexiconMap.contains(key)) continue;
        emit setProgressMessage("Creating database for " + key);
        createLexiconDatabase(key);
        QSqlDatabase::removeDatabase(key + "DB");
    }

}

QString DatabaseHandler::reverse(QString word)
{
    /* reverses a word */
    QString ret;
    for (int i = word.length()-1; i >= 0; i--)
        ret += word[i];

    return ret;
}

void DatabaseHandler::createLexiconDatabase(QString lexiconName)
{

    QDir dir = QDir::home();
    if (!dir.exists(".aerolith"))
        dir.mkdir(".aerolith");
    dir.cd(".aerolith");
    if (!dir.exists("lexica"))
        dir.mkdir("lexica");
    dir.cd("lexica");



    emit setProgressMessage("Loading word graphs...");
    QTime time;
    time.start();
    qDebug() << "Create" << lexiconName;
    LexiconInfo* lexInfo = &(lexiconMap[lexiconName]);
    lexInfo->dawg.readDawg("words/" + lexInfo->dawgFilename);
    lexInfo->reverseDawg.readDawg("words/" + lexInfo->dawgRFilename);




    QHash <QString, Alph> alphagramsHash;

    QFile file("words/" + lexInfo->wordsFilename);
    if (!file.open(QIODevice::ReadOnly)) return;
    QSqlDatabase db =  QSqlDatabase::addDatabase("QSQLITE", lexiconName + "DB");
    db.setDatabaseName(dir.absolutePath() + "/" + lexiconName + ".db");
    db.open();
    QSqlQuery wordQuery(db);
    wordQuery.exec("CREATE TABLE IF NOT EXISTS words(alphagram VARCHAR(15), word VARCHAR(15), "
                   "definition VARCHAR(256), lexiconstrings VARCHAR(5), front_hooks VARCHAR(26), "
                   "back_hooks VARCHAR(26))");
    wordQuery.exec("CREATE TABLE IF NOT EXISTS alphagrams(alphagram VARCHAR(15), words VARCHAR(255), "
                   "probability INTEGER, length INTEGER)");

    wordQuery.exec("CREATE TABLE IF NOT EXISTS wordlists(listname VARCHAR(40), numalphagrams INTEGER, probindices BLOB)");

    LessThans lessThan;
    if (lexInfo->lexiconName == "FISE") lessThan = SPANISH_LESS_THAN;
    else lessThan = ENGLISH_LESS_THAN;

    QTextStream in(&file);
    QString queryText = "INSERT INTO words(alphagram, word, definition, lexiconstrings, front_hooks, back_hooks) "
                        "VALUES(?, ?, ?, ?, ?, ?) ";
    wordQuery.exec("BEGIN TRANSACTION");
    wordQuery.prepare(queryText);
    QHash <QString, QString> definitionsHash;
    QStringList dummy;

    int wordCount = 0;
    while (!in.atEnd())
    {
        in.readLine();
        wordCount++;
    }
    in.seek(0);
    emit setProgressRange(0, wordCount*3);   // 1 accounts for reading the words, 2 accounts for alph, 3 for fixing defs
    emit setProgressValue(0);
    int progress = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        line = line.simplified();
        if (line.length() > 0)
        {
            progress++;
            if (progress%1000 == 0)
                emit setProgressValue(progress);
            QString word = line.section(' ', 0, 0).toUpper();
            QString definition = line.section(' ', 1);
            definitionsHash.insert(word, definition);


            QString alphagram = alphagrammize(word, lessThan);
            if (!alphagramsHash.contains(alphagram))
                alphagramsHash.insert(alphagram, Alph(dummy, combinations(alphagram, lexInfo->letterDist), alphagram));

            alphagramsHash[alphagram].words << word;


            //if (listHash.contains


            QByteArray backHooks = lexInfo->dawg.findHooks(word.toAscii());
            QByteArray frontHooks = lexInfo->reverseDawg.findHooks(reverse(word).toAscii());

            //qDebug() << word << alphagram << definition << backHooks << frontHooks;
            wordQuery.bindValue(0, alphagram);
            wordQuery.bindValue(1, word);
            wordQuery.bindValue(2, definition);
            wordQuery.bindValue(3, "");
            wordQuery.bindValue(4, QString(backHooks));
            wordQuery.bindValue(5, QString(frontHooks));
            wordQuery.exec();
        }
    }
    wordQuery.exec("END TRANSACTION");
    file.close();

    /* now sort alphagramsHash by probability/length */
    emit setProgressMessage("Read words. Sorting by probability...");
    QList <Alph> alphs = alphagramsHash.values();
    qSort(alphs.begin(), alphs.end(), probLessThan);

    emit setProgressMessage("Creating alphagrams...");

    queryText = "INSERT INTO alphagrams(alphagram, words, probability, length) VALUES(?, ?, ?, ?)";
    wordQuery.exec("BEGIN TRANSACTION");
    wordQuery.prepare(queryText);
    int probs[16];
    for (int i = 0; i < 16; i++)
        probs[i] = 0;
    for (int i = 0; i < alphs.size(); i++)
    {
        wordQuery.bindValue(0, alphs[i].alphagram);
        int wordLength = alphs[i].alphagram.length();
        wordQuery.bindValue(1, alphs[i].words.join(" "));

        progress++;
        if (progress%1000 == 0)
            emit setProgressValue(progress); // this is gonna be a little behind because of alphagrams.. it's ok

        if (wordLength <= 15)
            probs[wordLength]++;

        wordQuery.bindValue(2, probs[wordLength] + (wordLength << 24));
        wordQuery.bindValue(3, wordLength);
        wordQuery.exec();
    }

    wordQuery.exec("END TRANSACTION");

    qDebug() << "Created alphas in" << time.elapsed() << "for lexicon" << lexiconName;

    emit setProgressMessage("Updating definitions...");
    wordQuery.exec("CREATE UNIQUE INDEX word_index on words(word)");
    /* update definitions */


    updateDefinitions(definitionsHash, progress, db);

    /* update lexicon symbols if this is CSW (compare to OWL2)*/








    // do this indexing at the end.
    wordQuery.exec("CREATE UNIQUE INDEX probability_index on alphagrams(probability)");



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
    //    sqlDatabaseHandler(vowelQueryString.arg(8).arg(5).arg(lexiconName), "Five-vowel-8s", 8, lexiconName);
    //    sqlDatabaseHandler(vowelQueryString.arg(7).arg(4).arg(lexiconName), "Four-vowel-7s", 7, lexiconName);
    //    QString jqxzQueryString = "SELECT probability from alphagrams where length = %1 and lexiconName = '%2' and "
    //                              "(alphagram like '%Q%' or alphagram like '%J%' or alphagram like '%X%' or alphagram like '%Z%')";
    //
    //    for (int i = 4; i <= 8; i++)
    //        sqlDatabaseHandler(jqxzQueryString.arg(i).arg(lexiconName), QString("JQXZ %1s").arg(i), i, lexiconName);
    //
    //    if (lexiconName == "OWL2+LWL")
    //    {
    //        QString newWordsQueryString = "SELECT probability from alphagrams where length = %1 and lexiconName = '%2' and "
    //                                      "lexiconstrings like '%*%%' ESCAPE '*'";
    //        for (int i = 7; i <= 8; i++)
    //            sqlDatabaseHandler(newWordsQueryString.arg(i).arg(lexiconName), QString("New (OWL2) %1s").arg(i), i, lexiconName);
    //    }
    //    else if (lexiconName == "CSW")
    //    {
    //        QString newWordsQueryString = "SELECT probability from alphagrams where length = %1 and lexiconName = '%2' and "
    //                                      "lexiconstrings like '%#%'";
    //        for (int i = 7; i <= 8; i++)
    //            sqlDatabaseHandler(newWordsQueryString.arg(i).arg(lexiconName), QString("CSW-only %1s").arg(i), i, lexiconName);
    //    }
    //
    //    QString singleAnagramsQueryString = "SELECT probability from alphagrams where length = %1 and num_anagrams = 1 and lexiconName = '%2'";
    //    QString moreThanOneQueryString = "SELECT probability from alphagrams where length = %1 and num_anagrams > 1 and lexiconName = '%2'";
    //
    //    for (int i = 7; i <= 8; i++)
    //    {
    //        sqlDatabaseHandler(singleAnagramsQueryString.arg(i).arg(lexiconName), QString("One-anagram %1s").arg(i), i, lexiconName);
    //        sqlDatabaseHandler(moreThanOneQueryString.arg(i).arg(lexiconName), QString("Multi-anagram %1s").arg(i), i, lexiconName);
    //    }
    //
    //    QString uniqueLettersQueryString = "SELECT probability from alphagrams where num_unique_letters = %1 and length = %2 and lexiconName = '%3'";
    //    for (int i = 7; i <= 8; i++)
    //        sqlDatabaseHandler(uniqueLettersQueryString.arg(i).arg(i).arg(lexiconName), QString("Unique-letter %1s").arg(i), i, lexiconName);
    //
    //    wordQuery.exec("END TRANSACTION");
    //    zyzzyvaDb.close();
    //    QSqlDatabase::removeDatabase("zyzzyvaDB");
}

void DatabaseHandler::sqlListMaker(QString queryString, QString listName, quint8 wordLength, QString lexiconName)
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

void DatabaseHandler::updateDefinitions(QHash<QString, QString>& defHash, int progress, QSqlDatabase &db)
{
    QSqlQuery wordQuery(db);
    wordQuery.exec("BEGIN TRANSACTION");
    wordQuery.prepare("UPDATE words SET definition = ? WHERE word = ?");

    QHashIterator<QString, QString> hashIterator(defHash);
    while (hashIterator.hasNext())
    {
        progress++;
        if (progress%1000 == 0)
            emit setProgressValue(progress);

        hashIterator.next();
        QString word = hashIterator.key();
        QString definition = hashIterator.value();
        QStringList defs = definition.split(" / ");
        QString newDefinition;
        foreach (QString def, defs)
        {
            if (!newDefinition.isEmpty())
                newDefinition += "\n";
            newDefinition += followDefinitionLinks(def, defHash, false, 3);
        }

        if (definition != newDefinition)
        {
            wordQuery.bindValue(0, newDefinition);
            wordQuery.bindValue(1, word);
            wordQuery.exec();
        }

    }
    wordQuery.exec("END TRANSACTION");

}

QString DatabaseHandler::followDefinitionLinks(QString definition, QHash<QString, QString>& defHash, bool useFollow, int maxDepth)
{
    /* this code is basically taken from Michael Thelen's CreateDatabaseThread.cpp, part of Zyzzyva, which is
       GPLed software, source code available at http://www.zyzzyva.net, copyright Michael Thelen. */
    QRegExp followRegex (QString("\\{(\\w+)=(\\w+)\\}"));
    QRegExp replaceRegex (QString("\\<(\\w+)=(\\w+)\\>"));

    // Try to match the follow regex and the replace regex.  If a follow regex
    // is ever matched, then the "follow" replacements should always be used,
    // even if the "replace" regex is matched in a later iteration.
    QRegExp* matchedRegex = 0;

    int index = followRegex.indexIn(definition, 0);
    if (index >= 0) {
        matchedRegex = &followRegex;
        useFollow = true;
    }
    else {
        index = replaceRegex.indexIn(definition, 0);
        matchedRegex = &replaceRegex;
    }

    if (index < 0)
        return definition;

    QString modified (definition);
    QString word = matchedRegex->cap(1);
    QString pos = matchedRegex->cap(2);

    QString replacement;
    QString upper = word.toUpper();
    QString failReplacement = useFollow ? word : upper;
    if (!maxDepth)
    {
        replacement = failReplacement;
    }
    else
    {
        QString subdef = getSubDefinition(upper, pos, defHash);
        if (subdef.isEmpty())
        {
            replacement = failReplacement;
        }
        else if (useFollow)
        {
            replacement = (matchedRegex == &followRegex) ?
                word + " (" + subdef + ")" : subdef;
        }
        else
        {
            replacement = upper + ", " + subdef;
        }
    }

    modified.replace(index, matchedRegex->matchedLength(), replacement);
    int lowerMaxDepth = useFollow ? maxDepth - 1 : maxDepth;
    QString newDefinition = maxDepth
        ? followDefinitionLinks(modified, defHash, useFollow, lowerMaxDepth)
        : modified;
    return newDefinition;
}

QString DatabaseHandler::getSubDefinition(const QString& word, const QString& pos, QHash<QString, QString>& defHash)
{
    if (!defHash.contains(word))
        return QString();

    QString definition = defHash[word];
    QRegExp posRegex (QString("\\[(\\w+)"));
    QStringList defs = definition.split(" / ");
    foreach (QString def, defs)
    {
        if ((posRegex.indexIn(def, 0) > 0) &&
            (posRegex.cap(1) == pos))
        {
            QString str = def.left(def.indexOf("[")).simplified();
            if (!str.isEmpty())
                return str;
        }
    }

    return QString();
}





int DatabaseHandler::fact(int n)
{
    if (n == 0)
        return 1;
    return fact(n-1)*n;
}

int DatabaseHandler::nCr(int n, int r)
{
    if (n < r)
        return 0;
    if (n == r)
        return 1;
    return (fact(n) / (fact(n-r) * fact(r)));


}

bool spanishLessThan(const unsigned char i, const unsigned char j)
{
    // anyone have a less horrible way of doing this?
    float x, y;
    x = (float)tolower(i);
    y = (float)tolower(j);


    if (x == '1') x = (float)'c' + 0.5; // 'ch' is in between c and d
    else if (x == '2') x = (float)'l' + 0.5; // 'll' is in between l and m
    else if (x == '3') x = (float)'r' + 0.5; // 'rr' is in between r and s
    else if (x == '4') x = (float)'n' + 0.5; // n-tilde is in between n and o

    if (y == '1') y = (float)'c' + 0.5; // 'ch' is in between c and d
    else if (y == '2') y = (float)'l' + 0.5; // 'll' is in between l and m
    else if (y == '3') y = (float)'r' + 0.5; // 'rr' is in between r and s
    else if (y == '4') y = (float)'n' + 0.5; // n-tilde is in between n and o

    return x < y;

}

QString DatabaseHandler::alphagrammize(QString word, LessThans lessThan)
{
    QString ret;
    letterList.clear();
    for (int i = 0; i < word.size(); i++)
        letterList << word[i].toLatin1();
    if (lessThan == ENGLISH_LESS_THAN)
        qSort(letterList);
    else if (lessThan == SPANISH_LESS_THAN)
        qSort(letterList.begin(), letterList.end(), spanishLessThan);

    for (int i = 0; i < letterList.size(); i++)
        ret[i] = letterList[i];

    return ret;
}

int DatabaseHandler::combinations(QString alphagram, QMap <unsigned char, int> letterDist)
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

QMap <unsigned char, int> DatabaseHandler::getEnglishDist()
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

QMap <unsigned char, int> DatabaseHandler::getSpanishDist()
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
    DatabaseHandler::createListDatabase();


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
