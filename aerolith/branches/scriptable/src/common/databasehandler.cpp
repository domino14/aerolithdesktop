//    Copyright 2007, 2008, 2009, Cesar Del Solar  <delsolar@gmail.com>
//    This file is part of Aerolith.
//
//    Some of this code is copyright Michael Thelen; noted below.
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

//const QString userlistPathPrefix = "~/.aerolith/userlists/";
#include "commonDefs.h"
const int remoteNumQsQuota = 500000;

QMap <QString, LexiconInfo> DatabaseHandler::lexiconMap;


DatabaseHandler::DatabaseHandler()
{
    shouldQuitThread = false;
    start();
}

DatabaseHandler::~DatabaseHandler()
{
    shouldQuitThread = true;
    QTime time;
    time.start();
    while (isRunning() || time.elapsed() < 5000) ;
}


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

    foreach (QString key, lexiconMap.keys())
    {
        lexiconMap[key].dawg.readDawg(Utilities::getRootDir() + "/words/" + lexiconMap[key].dawgFilename);
        lexiconMap[key].reverseDawg.readDawg(Utilities::getRootDir() + "/words/" + lexiconMap[key].dawgRFilename);
    }

}

QStringList DatabaseHandler::findAvailableDatabases()
{
    QStringList dbList;
    QDir dir = QDir::home();

    if (dir.exists(".aerolith"))
    {
        dir.cd(".aerolith");
        if (dir.exists("lexica"))
        {
            dir.cd("lexica");
            QSqlDatabase lexicaDb = QSqlDatabase::addDatabase("QSQLITE", "lexicaDB");
            lexicaDb.setDatabaseName(dir.absolutePath() + "/lexica.db");
            lexicaDb.open();
            QSqlQuery lexicaQuery(lexicaDb);
            lexicaQuery.exec("SELECT lexiconName from lexica");
            int numLexica = 0;
            while (lexicaQuery.next())
            {
                numLexica++;
                QString lexiconName = lexicaQuery.value(0).toString();
                dbList << lexiconName;
            }
        }

    }
    availableDatabases = dbList;
    QSqlDatabase::removeDatabase("lexicaDB");
    return dbList;


}

QStringList DatabaseHandler::getAvailableDatabases()
{
    QMutexLocker locker(&dbListProtector);
    return availableDatabases;
}

void DatabaseHandler::connectToAvailableDatabases()
{
    QStringList dbList = DatabaseHandler::findAvailableDatabases();
    QDir dir = QDir::home();
    if (!dir.exists(".aerolith"))
        return;
    dir.cd(".aerolith");
    if (!dir.exists("lexica"))
        return;
    dir.cd("lexica");


    foreach (QString key, dbList)
    {
        if (lexiconMap.contains(key))
        {
            LexiconInfo* lexInfo = &(lexiconMap[key]);
            // qDebug() << "Name:" << lexInfo->lexiconName;
            QSqlDatabase* db = NULL;

            lexInfo->dbConn =  QSqlDatabase::addDatabase("QSQLITE", key + "DBconn");
            lexInfo->dbConn.setDatabaseName(dir.absolutePath() + "/" + key + ".db");
            lexInfo->dbConn.open();
            db = &(lexInfo->dbConn);

            // make sure to populate word length in lexInfo!

            QSqlQuery query(*db);
            query.exec("BEGIN TRANSACTION");
            query.prepare("SELECT numalphagrams from lengthcounts where length = ?");
            for (int i = 0; i < 16; i++)
            {
                query.addBindValue(i);
                query.exec();
                while (query.next())
                {
                    lexInfo->alphagramsPerLength[i] = query.value(0).toInt();
                }
            }
            query.exec("END TRANSACTION");
        }


    }


    // todo,future move this to a different static function only called by the server

    dir = QDir::home();
    if (!dir.exists(".aerolith"))
        dir.mkdir(".aerolith");
    dir.cd(".aerolith");
    if (!dir.exists("userlists"))
        dir.mkdir("userlists");
    dir.cd("userlists");
    userlistsDb = QSqlDatabase::addDatabase("QSQLITE", "userlistsDB");
    userlistsDb.setDatabaseName(dir.absolutePath() + "/userlists.db");
    userlistsDb.open();

    qDebug() << "Creating userlistinfo table (if not exists)";
    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.exec("CREATE TABLE IF NOT EXISTS userlistInfo(username VARCHAR(16) NOT NULL, "
                        "listName VARCHAR(64) NOT NULL, "
                        "lexiconName VARCHAR(15) NOT NULL, "
                        "lastdateSaved VARCHAR(64), alphasInList INTEGER, "
                        "path VARCHAR(64) NOT NULL, "
                        "PRIMARY KEY(listName, lexiconName, username) )");

    userListsQuery.exec("CREATE UNIQUE INDEX listnameIndex ON userlistInfo(username, listName, lexiconName)");

    userListsQuery.exec("CREATE TABLE IF NOT EXISTS numQsByUsername(username VARCHAR(16) NOT NULL, "
                        "numQs INTEGER)");

    // the list itself is saved in a file:
    // ~/.aerolith/userlists/username/lexiconName/path

}


void DatabaseHandler::createLexiconDatabases(QStringList dbsToCreate)
{
    setProgressMessage("Creating lexicon databases...");
    emit enableClose(false);
    // TODO above signal is misnamed

    QDir dir = QDir::home();
    if (!dir.exists(".aerolith"))
        dir.mkdir(".aerolith");
    dir.cd(".aerolith");
    if (!dir.exists("lexica"))
        dir.mkdir("lexica");
    dir.cd("lexica");

    QSqlDatabase lexiconNamesDB = QSqlDatabase::addDatabase("QSQLITE", "lexicaDB");
    lexiconNamesDB.setDatabaseName(dir.absolutePath() + "/lexica.db");
    lexiconNamesDB.open();

    QSqlQuery lexiconQuery(lexiconNamesDB);
    lexiconQuery.exec("CREATE TABLE IF NOT EXISTS lexica(lexiconName VARCHAR(15))");
    lexiconQuery.exec("CREATE UNIQUE INDEX lexicaIndex ON lexica(lexiconName)");
    lexiconQuery.prepare("INSERT INTO lexica(lexiconName) VALUES(?)");

    foreach (QString key, dbsToCreate)
    {


        if (!lexiconMap.contains(key)) continue;
        createLexiconDatabase(key);
        emit createdDatabase(key);
        lexiconQuery.bindValue(0, key);
        lexiconQuery.exec();
        QSqlDatabase::removeDatabase(key + "DB");
    }
    lexiconNamesDB.close();
    QSqlDatabase::removeDatabase("lexicaDB");
    emit setProgressMessage("All databases created. Please close this window.");
    emit enableClose(true);
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

    if (dir.exists(lexiconName + ".db"))
        dir.remove(lexiconName + ".db");

    emit setProgressMessage(lexiconName + ": Loading word graphs...");
    QTime time;
    time.start();
    qDebug() << "Create" << lexiconName;
    LexiconInfo* lexInfo = &(lexiconMap[lexiconName]);
    lexInfo->dawg.readDawg(Utilities::getRootDir() + "/words/" + lexInfo->dawgFilename);
    lexInfo->reverseDawg.readDawg(Utilities::getRootDir() + "/words/" + lexInfo->dawgRFilename);

    lexInfo->resetLetterDistributionVariables();
    emit setProgressMessage(lexiconName + ": Reading in dictionary.");

    QHash <QString, Alph> alphagramsHash;

    QFile file(Utilities::getRootDir() + "/words/" + lexInfo->wordsFilename);
    if (!file.open(QIODevice::ReadOnly)) return;
    QSqlDatabase db =  QSqlDatabase::addDatabase("QSQLITE", lexiconName + "DB");
    db.setDatabaseName(dir.absolutePath() + "/" + lexiconName + ".db");
    db.open();
    QSqlQuery wordQuery(db);
    wordQuery.exec("CREATE TABLE IF NOT EXISTS dbVersion(version INTEGER)");
    wordQuery.exec("INSERT INTO dbVersion(version) VALUES(1)"); // version 1
    wordQuery.exec("CREATE TABLE IF NOT EXISTS words(alphagram VARCHAR(15), word VARCHAR(15), "
                   "definition VARCHAR(256), lexiconstrings VARCHAR(5), front_hooks VARCHAR(26), "
                   "back_hooks VARCHAR(26))");
    wordQuery.exec("CREATE TABLE IF NOT EXISTS alphagrams(alphagram VARCHAR(15), words VARCHAR(255), "
                   "probability INTEGER PRIMARY KEY, length INTEGER, num_vowels INTEGER)");

    wordQuery.exec("CREATE TABLE IF NOT EXISTS wordlists(listname VARCHAR(40), numalphagrams INTEGER, probindices BLOB)");
    wordQuery.exec("CREATE TABLE IF NOT EXISTS lengthcounts(length INTEGER, numalphagrams INTEGER)");
    // TOO create index for wordlists?
    LessThans lessThan;
    if (lexInfo->lexiconName == "FISE") lessThan = SPANISH_LESS_THAN;
    else lessThan = ENGLISH_LESS_THAN;

    bool updateCSWPoundSigns = false;
    /* update lexicon symbols if this is CSW (compare to OWL2)*/
    LexiconInfo* lexInfoAmerica = NULL;
    if (lexiconName == "CSW" &&
        (availableDatabases.contains("OWL2+LWL") || dbsToCreate.contains("OWL2+LWL")))
    {
        updateCSWPoundSigns = true;
        lexInfoAmerica = &(lexiconMap["OWL2+LWL"]);
    }

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
                alphagramsHash.insert(alphagram, Alph(dummy, lexInfo->combinations(alphagram), alphagram));

            alphagramsHash[alphagram].words << word;

            QByteArray backHooks = lexInfo->dawg.findHooks(word.toAscii());
            QByteArray frontHooks = lexInfo->reverseDawg.findHooks(reverse(word).toAscii());
            QString lexSymbols = "";
            if (updateCSWPoundSigns && lexInfoAmerica && !lexInfoAmerica->dawg.findWord(word.toAscii()))
                lexSymbols = "#";

            //qDebug() << word << alphagram << definition << backHooks << frontHooks;
            wordQuery.bindValue(0, alphagram);
            wordQuery.bindValue(1, word);
            wordQuery.bindValue(2, definition);
            wordQuery.bindValue(3, lexSymbols);
            wordQuery.bindValue(4, QString(frontHooks));
            wordQuery.bindValue(5, QString(backHooks));
            wordQuery.exec();
        }
    }
    wordQuery.exec("END TRANSACTION");
    file.close();

    /* now sort alphagramsHash by probability/length */
    emit setProgressMessage(lexiconName + ": Sorting by probability...");
    QList <Alph> alphs = alphagramsHash.values();
    qSort(alphs.begin(), alphs.end(), probLessThan);

    emit setProgressMessage(lexiconName + ": Creating alphagrams...");

    queryText = "INSERT INTO alphagrams(alphagram, words, probability, length, num_vowels) VALUES(?, ?, ?, ?, ?)";
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

        wordQuery.bindValue(2, encodeProbIndex(probs[wordLength], wordLength));
        wordQuery.bindValue(3, wordLength);
        wordQuery.bindValue(4, alphs[i].alphagram.count(QChar('A')) +  alphs[i].alphagram.count(QChar('E')) +
                            alphs[i].alphagram.count(QChar('I')) +  alphs[i].alphagram.count(QChar('O')) +
                            alphs[i].alphagram.count(QChar('U')));
        wordQuery.exec();
    }

    wordQuery.exec("END TRANSACTION");

    qDebug() << "Created alphas in" << time.elapsed() << "for lexicon" << lexiconName;

    emit setProgressMessage(lexiconName + ": updating definitions...");
    wordQuery.exec("CREATE UNIQUE INDEX word_index on words(word)");
    /* update definitions */


    updateDefinitions(definitionsHash, progress, db);



    emit setProgressMessage(lexiconName + ": Indexing database...");


    // do this indexing at the end.
    //    wordQuery.exec("CREATE UNIQUE INDEX probability_index on alphagrams(probability)");
    wordQuery.exec("CREATE UNIQUE INDEX alphagram_index on alphagrams(alphagram)");

    wordQuery.prepare("INSERT INTO lengthcounts(length, numalphagrams) VALUES(?, ?)");
    for (int i = 2; i <= 15; i++)
    {
        wordQuery.addBindValue(i);
        wordQuery.addBindValue(probs[i]);
        wordQuery.exec();
    }

    emit setProgressMessage(lexiconName + ": Creating special lists...");

    wordQuery.exec("BEGIN TRANSACTION");

    QString vowelQueryString = "SELECT probability from alphagrams where length = %1 and num_vowels = %2";
    sqlListMaker(vowelQueryString.arg(8).arg(5), "Five-vowel-8s", 8, db);
    sqlListMaker(vowelQueryString.arg(7).arg(4), "Four-vowel-7s", 7, db);
    QString jqxzQueryString = "SELECT probability from alphagrams where length = %1 and "
                              "(alphagram like '%Q%' or alphagram like '%J%' or alphagram like '%X%' or alphagram like '%Z%')";

    for (int i = 4; i <= 8; i++)
        sqlListMaker(jqxzQueryString.arg(i), QString("JQXZ %1s").arg(i), i, db);

    if (lexiconName == "CSW")
    {
        QString newWordsQueryString = "SELECT alphagram from words where length(alphagram) = %1 and "
                                      "lexiconstrings like '%#%'";
        for (int i = 7; i <= 8; i++)
            sqlListMaker(newWordsQueryString.arg(i), QString("CSW-only %1s").arg(i), i, db, ALPHAGRAM_QUERY);
    }

    wordQuery.exec("END TRANSACTION");


    emit setProgressMessage(lexiconName + ": Database created!");
    emit setProgressValue(0);

}

void DatabaseHandler::sqlListMaker(QString queryString, QString listName, quint8 wordLength,
                                   QSqlDatabase& db, SqlListMakerQueryTypes queryType)
{

    QSqlQuery wordQuery(db);
    wordQuery.exec(queryString);
    QVector <quint32> probIndices;
    if (queryType == PROBABILITY_QUERY)
    {
        while (wordQuery.next())
        {
            probIndices.append(wordQuery.value(0).toInt());
        }
        qDebug() << listName << "found" << probIndices.size();
        if (probIndices.size() == 0) return;
    }
    else if (queryType == ALPHAGRAM_QUERY)
    {
        QStringList alphagrams;
        while (wordQuery.next())
        {
            alphagrams.append(wordQuery.value(0).toString());
        }
        /* has a list of all the alphagrams */
        if (alphagrams.size() == 0) return;
        foreach (QString alpha, alphagrams)
        {
            wordQuery.exec("SELECT probability from alphagrams where alphagram = '" + alpha + "'");

            while (wordQuery.next())
            {
                probIndices.append(wordQuery.value(0).toInt());
            }

        }

    }
    QByteArray ba;
    QDataStream baWriter(&ba, QIODevice::WriteOnly);

    baWriter << (quint8)1 << (quint8)wordLength << (quint32)probIndices.size();

    // (quint8)1 means this is a LIST of indices
    // second param is word length.
    // third param is number of indices
    foreach(quint32 index, probIndices)
        baWriter << index;

    QString toExecute = "INSERT INTO wordlists(listname, numalphagrams, probindices) "
                        "VALUES(?,?,?)";
    wordQuery.prepare(toExecute);
    wordQuery.bindValue(0, listName);
    wordQuery.bindValue(1, probIndices.size());
    wordQuery.bindValue(2, ba);
    wordQuery.exec();


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

int DatabaseHandler::getNumWordsByLength(QString lexiconName, int length)
{
    if (length >= 2 && length <= 15)
        return lexiconMap.value(lexiconName).alphagramsPerLength[length];
    else return 0;
}

void DatabaseHandler::getProbIndices(QStringList words, QString lexiconName, QString listName)
{
    QList <quint32> probIndices;
    if (!lexiconMap.value(lexiconName).dbConn.isOpen()) return;

    LessThans lessThan;
    if (lexiconName == "FISE") lessThan = SPANISH_LESS_THAN;
    else lessThan = ENGLISH_LESS_THAN;

    QSqlQuery query(lexiconMap.value(lexiconName).dbConn);
    query.exec("BEGIN TRANSACTION");
    query.prepare("SELECT probability from alphagrams where alphagram = ?");
    QSet <QString> alphaset;
    foreach (QString word, words)
    {
        QString alpha = alphagrammize(word.toUpper(), lessThan);
        if (!alphaset.contains(alpha))
        {
            query.addBindValue(alpha);
            query.exec();

            while (query.next())
            {
                probIndices.append((quint32) query.value(0).toInt());
            }
            alphaset.insert(alpha);
        }
    }
    query.exec("END TRANSACTION");
    emit returnProbIndices(probIndices, lexiconName, listName);
}

void DatabaseHandler::getQuestionData(QByteArray ba, QString lex, int tablenum)
{
    QSqlQuery query(lexiconMap.value(lex).dbConn);
    query.exec("BEGIN TRANSACTION");

    QDataStream in(ba);
    in.setVersion(QDataStream::Qt_4_2);
    quint8 numRacks;
    in >> numRacks;

    query.prepare("select words, alphagram from alphagrams "
                      "where probability = ?");


    QByteArray oba;
    QDataStream out(&oba, QIODevice::WriteOnly);


    QList <QStringList> solsList;
    QList <QString> alphList;
    QList <quint32> probList;
    for (int i = 0; i < numRacks; i++)
    {
        quint32 probIndex;
        in >> probIndex;
        quint8 numSolutionsNotYetSolved;
        in >> numSolutionsNotYetSolved;

        in.skipRawData(numSolutionsNotYetSolved);


        query.bindValue(0, probIndex);
        query.exec();

        QString alphagram;
        QStringList solutions;

        while (query.next())   // should only be one result.
        {
            alphagram = query.value(1).toString();
            solutions = query.value(0).toString().split(" ");
        }
        out << alphagram << solutions;

        probList << probIndex;
        alphList << alphagram;
        solsList << solutions;  // save solutions for looking up definitions later
    }
    emit returnQuestionInfo(ba, oba, tablenum); // return the original byte array + the new byte array
                                                    // with actual words.


    query.exec("END TRANSACTION");

    QByteArray solsByteArray;
    QDataStream outSols(&solsByteArray, QIODevice::WriteOnly);

    // now look up definitions.
    query.exec("BEGIN TRANSACTION");
    query.prepare("select definition, lexiconstrings, front_hooks, back_hooks from words where word = ?");


    outSols << (quint8)alphList.size();
    for (int i = 0; i < alphList.size(); i++)
    {
        QString alphagram = alphList.at(i);
        outSols << alphagram;

        if (alphagram == "") continue;

        outSols << (probList.at(i) & 0xFFFFFF); // convert from encoded probability to actual probability
                                            // for this word length.
        outSols << (quint8)solsList.at(i).size();
        for (int j = 0; j < solsList.at(i).size(); j++)
        {
            QString word = solsList.at(i).at(j);
            query.bindValue(0, word);
            query.exec();
            while (query.next())
            {
                QString frontHooks = query.value(2).toString();
                QString backHooks = query.value(3).toString();

                word += query.value(1).toString();
                QString definition = query.value(0).toString();
                outSols << word << definition << frontHooks << backHooks;
            }
        }
    }
    emit returnAnswerInfo(solsByteArray, tablenum);

    query.exec("END TRANSACTION");


}

/********************************************************************************/
/* QUEUED commands
*
*
*
*/
void DatabaseHandler::enqueueOtherDatabaseRequest(QByteArray ba)
{
    QMutexLocker locker(&queueProtector);
    commandQueue.append(ba);
    // assume command is already in correct format -- todo make this more elegant in the future!
}

void DatabaseHandler::enqueueCreateLexiconDatabases(QStringList lexiconNames)
{
    if (lexiconNames.size() == 0) return;
    QMutexLocker locker(&queueProtector);

    QByteArray command;
    QDataStream o(&command, QIODevice::WriteOnly);
    o << (quint8)CREATE_LEXICON_DATABASES;
    o << lexiconNames;

    commandQueue.append(command);
}


void DatabaseHandler::enqueueProbIndicesRequest(QStringList words, QString lexicon, QString listName)
{
    QMutexLocker locker(&queueProtector);

    QByteArray command;
    QDataStream o(&command, QIODevice::WriteOnly);
    o << (quint8)REQUEST_PROB_INDICES_FOR_UPLOADED_WORDLIST;
    o << words << lexicon << listName;

    commandQueue.append(command);
}

void DatabaseHandler::enqueueGetQuestionData(QByteArray ba, QString lexicon, int tablenum)
{
    QMutexLocker locker(&queueProtector);
    QByteArray command;
    QDataStream o(&command, QIODevice::WriteOnly);
    o << (quint8)GET_QUESTION_DATA;
    o << ba;
    o << lexicon;
    o << tablenum;

    commandQueue.append(command);
}


void DatabaseHandler::enqueueSaveSingleList(QString lexicon, QString listname, QString username, QList<quint32> indices)
{
    QMutexLocker locker(&queueProtector);
    QByteArray command;
    QDataStream o(&command, QIODevice::WriteOnly);
    o << (quint8)SAVE_SINGLE_LIST;
    o << lexicon << listname << username << indices;

    commandQueue.append(command);
}

void DatabaseHandler::enqueueListInfoRequest(QString lexicon, QString username)
{
    QMutexLocker locker(&queueProtector);
    QByteArray command;
    QDataStream o(&command, QIODevice::WriteOnly);
    o << (quint8)GET_LIST_INFO;
    o << lexicon << username;

    commandQueue.append(command);
}

void DatabaseHandler::enqueueListDeleteRequest(QString lexicon, QString listname, QString username)
{
    QMutexLocker locker(&queueProtector);
    QByteArray command;
    QDataStream o(&command, QIODevice::WriteOnly);
    o << (quint8)DELETE_LIST;
    o << lexicon << listname << username;

    commandQueue.append(command);
}


void DatabaseHandler::run()
{
    while (!shouldQuitThread)
    {
        if (commandQueue.size() > 0)
        {
            QByteArray cmd = commandQueue.dequeue();
            processCommand(cmd);
        }

        msleep(2);

    }

}

void DatabaseHandler::processCommand(QByteArray cmd)
{
    QDataStream i(cmd);

    quint8 header;
    i >> header;
    switch (header)
    {
    case CREATE_LEXICON_DATABASES:
        {
            QStringList dbsToCreate;
            i >> dbsToCreate;
            createLexiconDatabases(dbsToCreate);

        }
        break;
    case REQUEST_PROB_INDICES_FOR_UPLOADED_WORDLIST:
        {
            QStringList words;
            QString lexicon, listName;
            i >> words >> lexicon >> listName;
            getProbIndices(words, lexicon, listName);
        }
        break;
    case GET_QUESTION_DATA:
        {
            QByteArray ba;
            QString lex;
            int tablenum;
            i >> ba >> lex >> tablenum;
            getQuestionData(ba, lex, tablenum);
        }
        break;
    case SAVE_SINGLE_LIST:
        {
            QString lexicon;
            QString listname;
            QString username;
            QList<quint32> indices;
            i >> lexicon >> listname >> username >> indices;
            saveSingleList(lexicon, listname, username, indices);
        }
        break;
    case GET_LIST_INFO:
        {
            QString lexicon;
            QString username;

            i >> lexicon >> username;
            getAllListLabels(lexicon, username);
        }
        break;
    case DELETE_LIST:
        {
            QString lexicon, listname, username;
            i >> lexicon >> listname >> username;
            deleteUserList(lexicon, listname, username);
        }
        break;
    case DOES_LIST_EXIST:
        {
            QString lexicon, listname, username;
            quint64 tableid;
            quint16 tablenum;
            i >> lexicon >> listname >> username >> tablenum >> tableid;
            savedListExists(lexicon, listname, username, tablenum, tableid, true);
        }
        break;
    case GENERATE_QUIZ_ARRAY:
        {
            QString lexicon, listname;
            quint64 tableid;
            quint16 tablenum;
            quint8 listType, userlistMode;
            QString username;
            i >> listType >> lexicon >> listname >> userlistMode >> tablenum >> tableid >> username;
            generateUnscramblegameQuizArray(lexicon, listname, listType, userlistMode, tablenum, tableid, username);
        }
        break;
    }


}


/***********************************************************************/

QString DatabaseHandler::getSavedListRelativePath(QString lexiconName, QString listName, QString username)
{
    username = username.toLower();
    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("SELECT path from userlistInfo where lexiconName = ? and listName = ? and username = ?");
    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, listName);
    userListsQuery.bindValue(2, username);
    userListsQuery.exec();

    QString path;
    while (userListsQuery.next())
        path = userListsQuery.value(0).toString();


    return path;

}

QString DatabaseHandler::getSavedListAbsolutePath(QString lex, QString list, QString username)
{
    QDir dir = QDir::home();
    dir.cd(".aerolith");
    dir.cd("userlists");
    dir.cd(username);
    dir.cd(lex);
    return dir.filePath(getSavedListRelativePath(lex, list, username));

}

bool DatabaseHandler::savedListExists(QString lexicon, QString listName, QString username,
                                      quint16 tablenum,quint64 tableid, bool alsoEmit)
{
    username = username.toLower();
    QSqlQuery userListsQuery(userlistsDb);

    userListsQuery.exec("BEGIN TRANSACTION");
    userListsQuery.prepare("SELECT count(*) from userlistInfo WHERE lexiconName = ? and "
                           "listName = ? and username = ?");

    userListsQuery.bindValue(0, lexicon);
    userListsQuery.bindValue(1, listName);
    userListsQuery.bindValue(2, username);

    userListsQuery.exec();
    bool exists = false;
    while (userListsQuery.next())
    {
        int count = userListsQuery.value(0).toInt();
        if (count == 1)
        {
            exists = true;
        }
    }
    userListsQuery.exec("END TRANSACTION");

    if (alsoEmit) emit requestedListExists(exists, tablenum, tableid);
    return exists;
}

bool DatabaseHandler::saveGame(SavedUnscrambleGame sug, QString lex, QString list, QString username)
{

    QByteArray ba = sug.toByteArray();
    username = username.toLower();

    bool update = savedListExists(lex, list, username, 0 /*dummy*/, 0 /*dummy*/,false);
    qDebug() << "--Save Game--";
    qDebug() << "Update =" << update;
    bool resultSuccess = false;

    QDateTime curDT = QDateTime::currentDateTime();
    QString pathIfNew;

    QSqlQuery userListsQuery(userlistsDb);
    if (!update)
    {
        /* check we're not over quota */
        userListsQuery.prepare("SELECT numQs from numQsByUsername where username = ?");
        userListsQuery.bindValue(0, username);
        userListsQuery.exec();
        bool foundUsername = userListsQuery.next();
        qDebug() << "Found username=" << foundUsername;
        int numQs = 0;
        if (foundUsername)
            numQs = userListsQuery.value(0).toInt();

        if (numQs + sug.origIndices.size() > remoteNumQsQuota)
            return false;   // over quota, don't allow saving.

        pathIfNew = QString::number(curDT.toTime_t()) + "_" + curDT.toString("zzz");

        userListsQuery.prepare("INSERT INTO userlistInfo (lexiconName, listName, username, lastdateSaved, "
                               "alphasInList, path) VALUES (?, ?, ?, ?, ?, ?)");


        userListsQuery.bindValue(0, lex);
        userListsQuery.bindValue(1, list);
        userListsQuery.bindValue(2, username);
        userListsQuery.bindValue(3, curDT.toString("MMM d, yy h:mm:ss ap"));
        userListsQuery.bindValue(4, sug.origIndices.size());
        userListsQuery.bindValue(5, pathIfNew);
        resultSuccess = userListsQuery.exec();
        qDebug() << "Result success" << resultSuccess;
        if (resultSuccess)
        {
            numQs += sug.origIndices.size();
            if (!foundUsername)
            {
                userListsQuery.prepare("INSERT INTO numQsByUsername (numQs, username) VALUES (?, ?)");

            }
            else
            {
                userListsQuery.prepare("UPDATE numQsByUsername SET numQs = ? WHERE username = ?");
            }
            userListsQuery.bindValue(0, numQs);
            userListsQuery.bindValue(1, username);
            userListsQuery.exec();
            qDebug() << username << numQs;
        }
        else
            qCritical() << "Unexpected error saving list." << lex << list << username;


    }
    else
    {
        userListsQuery.prepare("UPDATE userlistInfo SET lastDateSaved = ? "
                               "WHERE lexiconName = ? and listName = ? and username = ?");

        userListsQuery.bindValue(0, QDateTime::currentDateTime().toString("MMM d, yy h:mm:ss ap"));
        userListsQuery.bindValue(1, lex);
        userListsQuery.bindValue(2, list);
        userListsQuery.bindValue(3, username);
        resultSuccess = userListsQuery.exec();

    }
    if (resultSuccess)
    {
        qDebug() << "Writing to file";
        QDir dir = QDir::home();
        dir.cd(".aerolith");
        dir.cd("userlists");
        if (!dir.exists(username))
        {
            dir.mkdir(username);
        }
        dir.cd(username);

        if (!dir.exists(lex))
        {
            dir.mkdir(lex);
        }

        dir.cd(lex);
        QFile f(dir.filePath(update ? getSavedListRelativePath(lex, list, username) : pathIfNew));

        f.open(QIODevice::WriteOnly);
        f.write(ba);
        f.close();

    }
    return resultSuccess;
}

void DatabaseHandler::saveSingleList(QString lexiconName, QString listName, QString username, QList <quint32> probIndices)
{
    username = username.toLower();

    qDebug() << "saveSingleList called";
    if (!userlistsDb.isOpen())
    {
        emit saveWordListFailed(username);
        return;
    }
    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("SELECT numQs from numQsByUsername where username = ?");
    userListsQuery.bindValue(0, username);
    userListsQuery.exec();
    bool foundUsername = userListsQuery.next();
    qDebug() << "In SLL. Found" << username << foundUsername;
    int numQs = 0;
    if (foundUsername)
        numQs = userListsQuery.value(0).toInt();

    if (numQs + probIndices.size() > remoteNumQsQuota)
    {
        emit saveWordListFailed(username);
        return;

    } // over quota, don't allow saving.

    QDateTime curDT = QDateTime::currentDateTime();
    QString path = QString::number(curDT.toTime_t()) + "_" + curDT.toString("zzz");

    userListsQuery.prepare("INSERT INTO userlistInfo(lexiconName, listName, username, lastdateSaved, "
                           "alphasInList, path) "
                           "VALUES(?, ?, ?, ?, ?, ?)");

    SavedUnscrambleGame sug;
    sug.initialize(probIndices);

    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, listName);
    userListsQuery.bindValue(2, username);
    userListsQuery.bindValue(3, curDT.toString("MMM d, yy h:mm:ss ap"));
    userListsQuery.bindValue(4, probIndices.size());
    userListsQuery.bindValue(5, path);
    bool retVal = userListsQuery.exec();

    if (!retVal)
    {
        emit saveWordListFailed(username);
        return;
    }

    numQs += probIndices.size();
    if (foundUsername)
    {
        userListsQuery.prepare("INSERT INTO numQsByUsername (numQs, username) VALUES (?, ?)");

    }
    else
    {
        userListsQuery.prepare("UPDATE numQsByUsername SET numQs = ? WHERE username = ?");
    }
    userListsQuery.bindValue(0, numQs);
    userListsQuery.bindValue(1, username);
    userListsQuery.exec();


    QDir dir = QDir::home();
    dir.cd(".aerolith");
    dir.cd("userlists");
    // assume the previous two directories exist, because they were created when the server started.

    if (!dir.exists(username))
    {
        dir.mkdir(username);
    }
    dir.cd(username);

    if (!dir.exists(lexiconName))
    {
        dir.mkdir(lexiconName);
    }
    dir.cd(lexiconName);

    QFile f(dir.filePath(path));
    f.open(QIODevice::WriteOnly);
    f.write(sug.toByteArray());
    f.close();

   // success!
    emit saveWordListSuccess(lexiconName, listName, probIndices.size(), username);
}

QStringList DatabaseHandler::getSingleListLabels(QString lexiconName, QString username, QString listname)
{
    username = username.toLower();
    QStringList retList;
    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("SELECT lastdateSaved from userlistInfo where "
                           "lexiconName = ? and username = ? and listName = ?");
    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, username);
    userListsQuery.bindValue(2, listname);
    userListsQuery.exec();


    QFile f;
    while (userListsQuery.next())
    {

        SavedUnscrambleGame sug;
        QString lastdateSaved = userListsQuery.value(0).toString();

        f.setFileName(getSavedListAbsolutePath(lexiconName, listname, username));
        f.open(QIODevice::ReadOnly);
        QByteArray ba = f.readAll();
        f.close();

        sug.populateFromByteArray(ba);

        QString totalQs, currentQs, firstMissedQs;
        totalQs = QString::number(sug.origIndices.size());
        firstMissedQs = QString::number(sug.firstMissed.size());

        if (sug.brandNew)
        {
            // list was never started
            currentQs = totalQs;
        }
        else
            currentQs = QString::number(sug.curQuizSet.size() + sug.curMissedSet.size());

        if (sug.seenWholeList)
        {
            // list has been gone through once already. (at least)
            totalQs += " *";    // to mark that this list has been gone through once already
        }

        retList << listname << totalQs << currentQs <<  firstMissedQs << lastdateSaved;
    }
    return retList;
}

void DatabaseHandler::getAllListLabels(QString lexiconName, QString username)
{
    QList<QStringList> outList;
    username = username.toLower();

    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("SELECT listName, lastdateSaved from userlistInfo where "
                           "lexiconName = ? and username = ?");
    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, username);
    userListsQuery.exec();

    QFile f;
    while (userListsQuery.next())
    {
        SavedUnscrambleGame sug;

        QString listName = userListsQuery.value(0).toString();
        QString lastdateSaved = userListsQuery.value(1).toString();


        f.setFileName(getSavedListAbsolutePath(lexiconName, listName, username));
        f.open(QIODevice::ReadOnly);
        QByteArray ba = f.readAll();
        f.close();

        sug.populateFromByteArray(ba);

        QString totalQs, currentQs, firstMissedQs;
        totalQs = QString::number(sug.origIndices.size());
        firstMissedQs = QString::number(sug.firstMissed.size());

        if (sug.brandNew)
        {
            // list was never started
            currentQs = totalQs;
        }
        else
            currentQs = QString::number(sug.curQuizSet.size() + sug.curMissedSet.size());

        if (sug.seenWholeList)
        {
            // list has been gone through once already. (at least)
            totalQs += " *";    // to mark that this list has been gone through once already
        }

        QStringList sl;
        sl << listName << totalQs << currentQs <<  firstMissedQs << lastdateSaved;
        outList << sl;
    }

    emit wordListInfo(username, lexiconName, outList);
}

void DatabaseHandler::deleteUserList(QString lexiconName, QString listName, QString username)
{
    username = username.toLower();

    QString path = getSavedListRelativePath(lexiconName, listName, username);
    QSqlQuery userListsQuery(userlistsDb);

    userListsQuery.prepare("SELECT alphasInList from userlistInfo where lexiconName = ? and listName = ? "
                           "and username = ?");
    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, listName);
    userListsQuery.bindValue(2, username);
    userListsQuery.exec();
    bool exists = userListsQuery.next();
    if (!exists) emit deleteWordListFailed(username);

    int thisNumQs= userListsQuery.value(0).toInt();


    userListsQuery.prepare("DELETE from userlistInfo where lexiconName = ? and listName = ? and username = ?");
    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, listName);
    userListsQuery.bindValue(2, username);
    bool retVal = userListsQuery.exec();

    if (!retVal) emit deleteWordListFailed(username);

    QDir dir = QDir::home();
    dir.cd(".aerolith");
    dir.cd("userlists");
    dir.cd(username);
    dir.cd(lexiconName);

    QFile::remove(dir.filePath(path));

    userListsQuery.prepare("SELECT numQs from numQsByUsername where username = ?");
    userListsQuery.bindValue(0, username);
    userListsQuery.exec();
    bool foundUsername = userListsQuery.next();

    int numQs = 0;
    if (foundUsername)
    {
        numQs = userListsQuery.value(0).toInt();
        qDebug() << "In Delete. Found" << username << foundUsername << numQs;
    }
    else
    {
        qCritical() << "Trying to delete a non existing list! Error!";
        emit deleteWordListFailed(username);
    }

    numQs -= thisNumQs;


    userListsQuery.prepare("UPDATE numQsByUsername SET numQs = ? WHERE username = ?");
    userListsQuery.bindValue(0, numQs);
    userListsQuery.bindValue(1, username);
    userListsQuery.exec();
    emit deleteWordListSuccess(lexiconName, listName, username);
}

void DatabaseHandler::getListSpaceUsage(QString username, quint32 &usage, quint32 &max)
{
    username = username.toLower();
    usage = 0;
    max = remoteNumQsQuota;

    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("SELECT numQs from numQsByUsername WHERE username = ?");
    userListsQuery.bindValue(0, username);
    userListsQuery.exec();
    if (userListsQuery.next())
    {
        usage = userListsQuery.value(0).toInt();
    }
}

void DatabaseHandler::generateUnscramblegameQuizArray(QString lexicon, QString listname,
                                                      quint8 listType, quint8 userlistMode,
                                                      quint16 tablenum,quint64 tableid, QString username)
{
    SavedUnscrambleGame sug;
    QList <quint32> missedArray;
    QList <quint32> quizArray;
    quint16 numTotalRacks;
    switch (listType)
    {
        case LIST_TYPE_NAMED_LIST:
            {
                QSqlQuery query(lexiconMap.value(lexicon).dbConn);

                query.prepare("SELECT probindices from wordlists where listname = ?");
                query.bindValue(0, listname);
                query.exec();

                QByteArray indices;

                while (query.next())
                {
                    indices = query.value(0).toByteArray();
                }

                QDataStream stream(indices);
                quint8 type, length;
                stream >> type >> length;   //TODO remove length from the parameters

                if (type == 1)
                {
                    // a list of indices
                    quint32 size;
                    stream >> size;

                    quint32 index;
                    quizArray.clear();
                    for (quint32 i = 0; i < size; i++)
                    {

                        stream >> index;
                        quizArray << index;
                    }

                    Utilities::shuffle(quizArray);
                    numTotalRacks = size;
                }
                else
                    qCritical() << "ERROR: type != 1 !";    // we always have a list of indices for named lists

                sug.initialize(quizArray);
                // and set SUG accordingly
                break;
            }
        case LIST_TYPE_USER_LIST:
            {
                QString pathName = getSavedListAbsolutePath(lexicon, listname, username);

                QFile f(pathName);
                f.open(QIODevice::ReadOnly);
                QByteArray ba = f.readAll();

                sug.populateFromByteArray(ba);
                QList <quint32> qindices;
                QList <quint32> mindices;
                switch (userlistMode)
                {

                    case UNSCRAMBLEGAME_USERLIST_MODE_RESTART:
                        sug.initialize(sug.origIndices);

                        qindices = sug.origIndices.toList();
                        mindices.clear();

                        break;

                    case UNSCRAMBLEGAME_USERLIST_MODE_FIRSTMISSED:
                        qindices = sug.firstMissed.toList();
                        mindices.clear();

                        sug.curQuizSet = sug.firstMissed;
                        sug.curMissedSet.clear();
                        break;

                    case UNSCRAMBLEGAME_USERLIST_MODE_CONTINUE:
                        if (sug.brandNew)
                        {
                            qindices = sug.origIndices.toList();
                            mindices.clear();
                        }
                        else
                        {
                            qindices = sug.curQuizSet.toList();
                            mindices = sug.curMissedSet.toList();
                        }
                     //   qDebug() << "Index:" << qindices.at(0);
                        break;
                }

                // a list of indices
                quint32 size = qindices.size();

                // TODO modularize this; put into a function. shouldn't repeat code.
                if (size > 0)
                {
                    quizArray = qindices;
                    Utilities::shuffle(quizArray);

                }
                numTotalRacks = size;

                size = mindices.size();
                if (size > 0)
                {
                    missedArray = mindices;
                    Utilities::shuffle(missedArray);
                }

                break;
            }
    }
    emit unscramblegameQuizArray(quizArray, missedArray, sug.toByteArray(), tablenum, tableid);

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
    dist.insert('Y', 2); dist.insert('Z', 1); dist.insert('?', 2);
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
    dist.insert('N', 5); dist.insert('4', 1); dist.insert('O', 9);  // 4 is enye
    dist.insert('P', 2); dist.insert('Q', 1); dist.insert('R', 5);
    dist.insert('S', 6); dist.insert('T', 4); dist.insert('U', 5);
    dist.insert('V', 1); dist.insert('X', 1); dist.insert('Y', 1);
    dist.insert('Z', 1); dist.insert('?', 2);
    return dist;
}

quint32 DatabaseHandler::encodeProbIndex(quint32 probIndex, quint32 wordLength)
{
    return probIndex + (wordLength << 24);
}

/*

  ****************

  */


void LexiconInfo::resetLetterDistributionVariables()
{
    int maxFrequency = 15;

    int totalLetters = 0;

    foreach (unsigned char c, letterDist.keys())
    {
        int frequency = letterDist.value(c);
        totalLetters += frequency;
        if (frequency > maxFrequency)
            maxFrequency = frequency;
    }

    // Precalculate M choose N combinations - use doubles because the numbers
    // get very large
    double a = 1;
    double r = 1;
    for (int i = 0; i <= maxFrequency; ++i, ++r)
    {
        fullChooseCombos.append(a);
        a *= (totalLetters + 1.0 - r) / r;

        QList<double> subList;
        for (int j = 0; j <= maxFrequency; ++j)
        {
            if ((i == j) || (j == 0))
                subList.append(1.0);
            else if (i == 0)
                subList.append(0.0);
            else {
                // XXX: For some reason this crashes on Linux when referencing
                // the first value as subChooseCombos[i-1][j-1], so value() is
                // used instead.  Weeeeird.
                subList.append(subChooseCombos.value(i-1).value(j-1) +
                               subChooseCombos.value(i-1).value(j));
            }
        }
        subChooseCombos.append(subList);
    }

}

double LexiconInfo::combinations(QString alphagram)
{
    // Build parallel arrays of letters with their counts, and the
    // precalculated combinations based on the letter frequency
    QList<unsigned char> letters;
    QList<int> counts;
    QList<const QList<double>*> combos;
    for (int i = 0; i < alphagram.length(); ++i) {
        unsigned char c = alphagram.at(i).toAscii();

        bool foundLetter = false;
        for (int j = 0; j < letters.size(); ++j) {
            if (letters[j] == c) {
                ++counts[j];
                foundLetter = true;
                break;
            }
        }

        if (!foundLetter) {
            letters.append(c);
            counts.append(1);
            combos.append(&subChooseCombos[ letterDist[c] ]);
        }
    }

    // XXX: Generalize the following code to handle arbitrary number of blanks
    double totalCombos = 0.0;
    int numLetters = letters.size();

    // Calculate the combinations with no blanks
    double thisCombo = 1.0;
    for (int i = 0; i < numLetters; ++i) {
        thisCombo *= (*combos[i])[ counts[i] ];
    }
    totalCombos += thisCombo;

    // Calculate the combinations with one blank
    for (int i = 0; i < numLetters; ++i) {
        --counts[i];
        thisCombo = subChooseCombos[ letterDist['?'] ][1];
        for (int j = 0; j < numLetters; ++j) {
            thisCombo *= (*combos[j])[ counts[j] ];
        }
        totalCombos += thisCombo;
        ++counts[i];
    }

    // Calculate the combinations with two blanks
    for (int i = 0; i < numLetters; ++i) {
        --counts[i];
        for (int j = i; j < numLetters; ++j) {
            if (!counts[j])
                continue;
            --counts[j];
            thisCombo = subChooseCombos[ letterDist['?'] ][2];

            for (int k = 0; k < numLetters; ++k) {
                thisCombo *= (*combos[k])[ counts[k] ];
            }
            totalCombos += thisCombo;
            ++counts[j];
        }
        ++counts[i];
    }

    return totalCombos;

}
