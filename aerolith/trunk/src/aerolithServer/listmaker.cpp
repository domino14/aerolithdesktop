#include "listmaker.h"
#include <QTime>

extern const QString WORD_DATABASE_NAME = "wordDB";
extern const QString WORD_DATABASE_FILENAME = "words.db";

/*http://bytes.com/forum/thread138180.html for discussion on having to declare extern explicitly
with const */

ListMaker::ListMaker()
{

}
void ListMaker::testDatabaseTime()
{
  QTime timer;
  QString strings[10000];
  QSqlQuery wordQuery(QSqlDatabase::database(WORD_DATABASE_NAME));  
  timer.start();
  int strIndex;
  wordQuery.exec("SELECT alphagram, words from alphagrams where length = 8 and probability between 5000 and 6000");
  qDebug() << "1: " << timer.elapsed();
  strIndex = 0;
  while (wordQuery.next())
    {
      strings[strIndex] = wordQuery.value(0).toString() + "---" + wordQuery.value(1).toString();
      strIndex++;
    }

  qDebug() << "2: " << timer.elapsed();

  for (int i = 0; i < strIndex; i++)
    qDebug() << i << strings[i];

}

void ListMaker::createListDatabase()
{
	// creates a word list database.
	// requires the zyzzyva database to be in the user's install directory
	QSqlDatabase wordDb;
	wordDb = QSqlDatabase::addDatabase("QSQLITE", WORD_DATABASE_NAME);

	wordDb.setDatabaseName(WORD_DATABASE_FILENAME);
	wordDb.open();

	QSqlDatabase zyzzyvaDb;
	
	
	#ifdef Q_WS_MAC
	QSettings ZyzzyvaSettings("pietdepsi.com", "Zyzzyva");
	#else
	QSettings ZyzzyvaSettings("Piet Depsi", "Zyzzyva");
	#endif
	ZyzzyvaSettings.beginGroup("Zyzzyva");

	QString defaultUserDir = QDir::homePath() + "/.zyzzyva";
	QString zyzzyvaDataDir = QDir::cleanPath (ZyzzyvaSettings.value("user_data_dir", defaultUserDir).toString());

	if (QFile::exists(zyzzyvaDataDir + "/lexicons/OWL2+LWL.db"))
	{
		zyzzyvaDb = QSqlDatabase::addDatabase("QSQLITE", "zyzzyvaDB");
		zyzzyvaDb.setDatabaseName(zyzzyvaDataDir + "/lexicons/OWL2+LWL.db");
		zyzzyvaDb.open();
	}
	else
	{
		qDebug() << "A Zyzzyva installation was not found. Cannot generate word lists!";
		return;
	}
	
	QSqlQuery wordQuery(QSqlDatabase::database(WORD_DATABASE_NAME));
	QSqlQuery transactionQuery(QSqlDatabase::database(WORD_DATABASE_NAME));
	
	bool alphagramsExists = false;
	wordQuery.exec("select tbl_name from sqlite_master where name='alphagrams'");
	while (wordQuery.next())
	{
		if (wordQuery.value(0).toString() == "alphagrams")
			alphagramsExists = true;
	}
	qDebug() << "alphagramsExists" << alphagramsExists;
	
	if (!alphagramsExists)
	{
		wordQuery.exec("CREATE TABLE IF NOT EXISTS alphagrams(alphagram VARCHAR(15), words VARCHAR(255), "
		"length INTEGER, probability INTEGER, num_vowels INTEGER, lexiconstring VARCHAR(5), "
		"num_anagrams INTEGER, num_unique_letters INTEGER)");
		wordQuery.exec("CREATE UNIQUE INDEX alphagram_index on alphagrams(alphagram)");
		wordQuery.exec("CREATE UNIQUE INDEX probability_index on alphagrams(probability, length)");
		
		QSqlQuery zyzzyvaQuery(QSqlDatabase::database("zyzzyvaDB"));
		QTime time;
		for (int i = 2; i <= 15; i++)
		{
			time.start();
			qDebug() << "Length" << i;
			
			QString queryString = QString("SELECT word, alphagram, lexicon_symbols, num_vowels, num_anagrams, num_unique_letters "
			"from words where length = %1 order by probability_order").arg(i);
			zyzzyvaQuery.exec(queryString);
			int probability = 1;
			zyzzyvaQuery.next();
			bool nextSucceeded;
			transactionQuery.exec("BEGIN TRANSACTION");
			while (true)
			{

				int num_anagrams = zyzzyvaQuery.value(4).toInt();
				QString alphagram;
				QString wordString = "";
				QString lexiconString = "";
				int numVowels;
				int numAnagrams;
				int numUniqueLetters;
				for (int n = 0; n < num_anagrams; n++)
				{
					alphagram = zyzzyvaQuery.value(1).toString();
					wordString += zyzzyvaQuery.value(0).toString() + " ";
					if (zyzzyvaQuery.value(2).toString().contains("%")) lexiconString = "%";
					numVowels = zyzzyvaQuery.value(3).toInt();
					numAnagrams = zyzzyvaQuery.value(4).toInt();
					numUniqueLetters = zyzzyvaQuery.value(5).toInt();
					nextSucceeded = zyzzyvaQuery.next();

				}
				QString toExecute = "INSERT INTO alphagrams(alphagram, words, length, probability, num_vowels, lexiconstring, "
						"num_anagrams, num_unique_letters) "
						"VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
				wordQuery.prepare(toExecute);
				wordQuery.bindValue(0, alphagram);
				wordQuery.bindValue(1, wordString.trimmed());
				wordQuery.bindValue(2, alphagram.length());
				wordQuery.bindValue(3, probability);
				wordQuery.bindValue(4, numVowels);
				wordQuery.bindValue(5, lexiconString);
				wordQuery.bindValue(6, numAnagrams);
				wordQuery.bindValue(7, numUniqueLetters);
				probability++;
				
				wordQuery.exec();
				
				if (!nextSucceeded) break;
			}
			transactionQuery.exec("END TRANSACTION");
		}
// 	
	}


	bool wordlistsExists = false;
	wordQuery.exec("select tbl_name from sqlite_master where name='wordlists'");
	while (wordQuery.next())
	{
		if (wordQuery.value(0).toString() == "wordlists")
			wordlistsExists = true;
	}
	qDebug() << "wordlistsExists" << wordlistsExists;
	if (!wordlistsExists)
	{

	
		/* top 500 by prob 6s - 9s */
	
		wordQuery.exec("CREATE TABLE IF NOT EXISTS wordlists(listname VARCHAR(40), wordlength INTEGER, numalphagrams INTEGER, probindices BLOB)");
		wordQuery.exec("CREATE INDEX listname_index on wordlists(listname)");
	
	
		
		QVector <int> pick;
		pick.append(7);
		pick.append(8);
		pick.append(6);
		pick.append(9);
		foreach (int i, pick)
		{
			int probIndex = 1;
			int numAlphagrams;
			wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1").arg(i));
			while (wordQuery.next())
			{
				numAlphagrams = wordQuery.value(0).toInt();
			}
			
			int listLength = 500;
			int actualListLength;
			do
			{
				QByteArray ba;
				QDataStream baWriter(&ba, QIODevice::WriteOnly);
				
				wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1 and probability between %2 and %3").
				arg(i).arg(probIndex).arg(probIndex + listLength -1));
				
				while (wordQuery.next())
					actualListLength = wordQuery.value(0).toInt();
				
				baWriter << (quint8)0 << (quint8)i << (quint16)(probIndex) << (quint16)(probIndex+actualListLength-1);
				// (quint8)0 means this is a RANGE of indices, and not a list of indices
				// second param is word length.
				
				
				QString toExecute = "INSERT INTO wordlists(listname, wordlength, numalphagrams, probindices) "
				"VALUES(?, ?, ?, ?)";
				wordQuery.prepare(toExecute);
				wordQuery.bindValue(0, QString("Useful %1s (%2 - %3)").arg(i).arg(probIndex).arg(probIndex + actualListLength-1));
				wordQuery.bindValue(1, i);
				wordQuery.bindValue(2, actualListLength);
				wordQuery.bindValue(3, ba);
				wordQuery.exec();
				
				probIndex+= 500;
			
			} while (probIndex < numAlphagrams);
		}
		
		for (int i = 2; i <= 15; i++)
		{
			int numAlphagrams;
			QByteArray ba;
			QDataStream baWriter(&ba, QIODevice::WriteOnly);
			wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1").arg(i));
			while (wordQuery.next())
			{
				numAlphagrams = wordQuery.value(0).toInt();
			}
			
			baWriter << (quint8)0 << (quint8)i << (quint16)1 << (quint16)numAlphagrams;
			QString toExecute = "INSERT INTO wordlists(listname, wordlength, numalphagrams, probindices) "
				"VALUES(?, ?, ?, ?)";
			wordQuery.prepare(toExecute);
			wordQuery.bindValue(0, QString("OWL2 %1s").arg(i));
			wordQuery.bindValue(1, i);
			wordQuery.bindValue(2, numAlphagrams);
			wordQuery.bindValue(3, ba);
			wordQuery.exec();
		}
		
		QString vowelQueryString = "SELECT probability from alphagrams where length = %1 and num_vowels = %2"; 
		sqlListMaker(vowelQueryString.arg(8).arg(5), "Five-vowel-8s", 8);
		sqlListMaker(vowelQueryString.arg(7).arg(4), "Four-vowel-7s", 7);
		QString jqxzQueryString = "SELECT probability from alphagrams where length = %1 and "
		"(alphagram like '%Q%' or alphagram like '%J%' or alphagram like '%X%' or alphagram like '%Z%')";
		
		for (int i = 4; i <= 8; i++)
			sqlListMaker(jqxzQueryString.arg(i), QString("JQXZ %1s").arg(i), i);
	
		QString newWordsQueryString = "SELECT probability from alphagrams where length = %1 and "
		"lexiconstring = '%'";
		for (int i = 7; i <= 8; i++)
			sqlListMaker(newWordsQueryString.arg(i), QString("New (OWL2) %1s").arg(i), i);
			
		QString singleAnagramsQueryString = "SELECT probability from alphagrams where length = %1 and num_anagrams = 1";
		QString moreThanOneQueryString = "SELECT probability from alphagrams where length = %1 and num_anagrams > 1";
	
		for (int i = 7; i <= 8; i++)
		{
			sqlListMaker(singleAnagramsQueryString.arg(i), QString("One-anagram %1s").arg(i), i);
			sqlListMaker(moreThanOneQueryString.arg(i), QString("Multi-anagram %1s").arg(i), i);
		}
	
		QString uniqueLettersQueryString = "SELECT probability from alphagrams where num_unique_letters = %1 and length = %2";
		for (int i = 7; i <= 8; i++)
			sqlListMaker(uniqueLettersQueryString.arg(i).arg(i), QString("Unique-letter %1s").arg(i), i);



	
	}
	
	

}

void ListMaker::sqlListMaker(QString queryString, QString listName, quint8 wordLength)
{
	qDebug() << listName;
	QSqlQuery wordQuery(QSqlDatabase::database(WORD_DATABASE_NAME));
	wordQuery.exec(queryString);
	QVector <quint16> probIndices;
	
	while (wordQuery.next())
	{
		probIndices.append(wordQuery.value(0).toInt());
	}
	
	QByteArray ba;
	QDataStream baWriter(&ba, QIODevice::WriteOnly);
	
	baWriter << (quint8)1 << (quint8)wordLength << (quint16)probIndices.size();
	
	// (quint8)1 means this is a LIST of indices
	// second param is word length.
	// third param is number of indices
	foreach(quint16 index, probIndices)
		baWriter << index;
	
	QString toExecute = "INSERT INTO wordlists(listname, wordlength, numalphagrams, probindices) "
	"VALUES(?,?,?,?)";
	wordQuery.prepare(toExecute);
	wordQuery.bindValue(0, listName);
	wordQuery.bindValue(1, wordLength);
	wordQuery.bindValue(2, probIndices.size());
	wordQuery.bindValue(3, ba);
	wordQuery.exec();
	

}

