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

bool probLessThan(const Alph &a1, const Alph &a2)
{
    return a1.combinations > a2.combinations;
}

void DatabaseHandler::createLexiconMap(bool createDawgs)
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

    if (createDawgs)
    {
        foreach (QString key, lexiconMap.keys())
        {
            lexiconMap[key].dawg.readDawg("words/" + lexiconMap[key].dawgFilename);
            lexiconMap[key].reverseDawg.readDawg("words/" + lexiconMap[key].dawgRFilename);
        }
    }

}

QStringList DatabaseHandler::checkForDatabases()
{
    QStringList dbList;
    QDir dir = QDir::home();
    bool databasesExist = true;
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
    return dbList;


}

void DatabaseHandler::connectToDatabases(bool clientCall, QStringList dbList)
{
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
            qDebug() << "Name:" << lexInfo->lexiconName;
            if (clientCall)
            {

                lexInfo->db =  QSqlDatabase::addDatabase("QSQLITE", key + "DB_client");
                lexInfo->db.setDatabaseName(dir.absolutePath() + "/" + key + ".db");
                lexInfo->db.open();
            }
            else
            {
                lexInfo->db =  QSqlDatabase::addDatabase("QSQLITE", key + "DB_server");
                lexInfo->db.setDatabaseName(dir.absolutePath() + "/" + key + ".db");
                lexInfo->db.open();
            }
            // make sure to populate word length in lexInfo!

            QSqlQuery query(lexInfo->db);
            query.exec("BEGIN TRANSACTION");
            query.prepare("SELECT numalphagrams from lengthcounts where length = ?");
            for (int i = 0; i < 16; i++)
            {
                query.addBindValue(i);
                query.exec();
                while (query.next())
                {
                    lexInfo->alphagramsPerLength[i] = query.value(0).toInt();
                    qDebug() << "lengths " << i << lexInfo->alphagramsPerLength[i];
                }
            }
            query.exec("END TRANSACTION");
        }


    }



    if (clientCall)
    {
        dir = QDir::home();
        if (!dir.exists(".aerolith"))
            return;
        dir.cd(".aerolith");
        if (!dir.exists("userlists"))
            return;
        dir.cd("userlists");
        userlistsDb = QSqlDatabase::addDatabase("QSQLITE", "userlistsDB");
        userlistsDb.setDatabaseName(dir.absolutePath() + "/userlists.db");
        userlistsDb.open();


        QSqlQuery userListsQuery(userlistsDb);
        userListsQuery.exec("CREATE TABLE IF NOT EXISTS userlists(lexiconName VARCHAR(15), listName VARCHAR(64), "
                            "lastdateSaved VARCHAR(64), listdata BLOB)");
        userListsQuery.exec("CREATE UNIQUE INDEX listnameIndex ON userlists(listName)");

    }
}



void DatabaseHandler::createLexiconDatabases(QStringList lexiconNames)
{
    if (lexiconNames.size() == 0) return;

    qDebug() << "In here";

    if (!isRunning())
    {
        setProgressMessage("Creating lexicon databases...");
        dbsToCreate = lexiconNames;
        start();


    }
}

void DatabaseHandler::run()
{
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
    lexInfo->dawg.readDawg("words/" + lexInfo->dawgFilename);
    lexInfo->reverseDawg.readDawg("words/" + lexInfo->dawgRFilename);

    lexInfo->resetLetterDistributionVariables();
    emit setProgressMessage(lexiconName + ": Reading in dictionary.");

    QHash <QString, Alph> alphagramsHash;

    QFile file("words/" + lexInfo->wordsFilename);
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

        wordQuery.bindValue(2, probs[wordLength] + (wordLength << 24));
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

bool DatabaseHandler::getProbIndices(QStringList words, QString lexiconName, QList<quint32>& probIndices)
{
    if (!lexiconMap.value(lexiconName).db.isOpen()) return false;

    LessThans lessThan;
    if (lexiconName == "FISE") lessThan = SPANISH_LESS_THAN;
    else lessThan = ENGLISH_LESS_THAN;

    QSqlQuery query(lexiconMap.value(lexiconName).db);
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
                probIndices << (quint32) query.value(0).toInt();
            }
            alphaset.insert(alpha);
        }
    }
}

bool DatabaseHandler::saveNewLists(QString lexiconName, QString listName, QList <quint32>& probIndices)
{
    if (probIndices.size() <= 500 && probIndices.size() > 0) saveSingleList(lexiconName, listName, probIndices);
    else
    {
        bool pIndicesIsEmpty = probIndices.isEmpty();
        int listNum = 1;
        while (!pIndicesIsEmpty)
        {
            QList <quint32> indices;

            for (int i = 0; i < 500; i++)
            {
                if (!probIndices.isEmpty())
                    indices << probIndices.takeFirst();
                else
                {
                    pIndicesIsEmpty = true;
                    break;
                }
            }
            if (indices.size() > 0)
            {
                saveSingleList(lexiconName, listName + "_" + QString::number(listNum), indices);
                listNum++;

            }

        }
    }

}

bool DatabaseHandler::saveSingleList(QString lexiconName, QString listName, QList <quint32>& probIndices)
{
    if (!userlistsDb.isOpen())
    {
        return false;
    }

    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("INSERT INTO userlists(lexiconName, listName, lastdateSaved, listdata) VALUES(?, ?, ?, ?)");

    QByteArray ba;
    QDataStream baWriter(&ba, QIODevice::WriteOnly);

    baWriter << (quint16)probIndices.size();

    foreach(quint32 index, probIndices)
    {
        baWriter << index;
        baWriter << (quint16)0;  /* number of times missed */
    }
    baWriter << (quint16)0; // progress along full list
    baWriter << (quint16)0; // progress along missed list
    baWriter << (quint16)0; // number of missed questions of this level
    baWriter << (quint16)0; // maxNumMissed (i.e. what level miss this is, e.g. words missed 8 times already)

    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.bindValue(1, listName);
    userListsQuery.bindValue(2, QDateTime::currentDateTime().toString("MMM d, yy h:mm:ss ap"));
    userListsQuery.bindValue(3, ba);
    userListsQuery.exec();
}


QList<QStringList> DatabaseHandler::getListLabels(QString lexiconName)
{
    QList<QStringList> retList;
    QSqlQuery userListsQuery(userlistsDb);
    userListsQuery.prepare("SELECT listName, lastdateSaved, listdata from userlists where lexiconName = ?");
    userListsQuery.bindValue(0, lexiconName);
    userListsQuery.exec();

    while (userListsQuery.next())
    {
        QString listName = userListsQuery.value(0).toString();
        QString lastdateSaved = userListsQuery.value(1).toString();
        QByteArray ba = userListsQuery.value(2).toByteArray();

        QDataStream baReader(ba);

        quint16 numAlphas;
        baReader >> numAlphas;
        baReader.skipRawData((int)6*(int)numAlphas);    // skip all of these questions
        quint16 progFull, progMissed, numMissed, maxNumMissed;
        baReader >> progFull >> progMissed >> numMissed >> maxNumMissed;

        QString alphaProg = QString("%1 / %2").arg(progFull).arg(numAlphas);
        QString missedProg = QString("%1 / %2").arg(progMissed).arg(numMissed);

        QStringList sl;
        sl << listName << alphaProg << missedProg << lastdateSaved;
        retList << sl;
    }
    return retList;
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
