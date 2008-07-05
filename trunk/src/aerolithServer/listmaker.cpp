#include "listmaker.h"


const QString WORD_DATABASE_NAME = "wordDB";
const QString WORD_DATABASE_FILENAME = "words.db";

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

	if (QFile::exists(WORD_DATABASE_FILENAME)) 
	  {
	    wordDb.setDatabaseName(WORD_DATABASE_FILENAME);
	    wordDb.open();
	    return;
	  }

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
	wordQuery.exec("CREATE TABLE IF NOT EXISTS alphagrams(alphagram VARCHAR(15), words VARCHAR(255), length INTEGER, probability INTEGER)");
	//	wordQuery.exec("CREATE UNIQUE INDEX alphagram_index on alphagrams(alphagram)");
		wordQuery.exec("CREATE INDEX probability_index on alphagrams(probability)");
		// don't create a UNIQUE index for probability, since there are many words that share the same probability number (i.e. they
		// have different lengths)!
	
	QSqlQuery zyzzyvaQuery(QSqlDatabase::database("zyzzyvaDB"));
	
	for (int i = 2; i <= 15; i++)
	{
		qDebug() << "Length" << i;
		QString queryString = QString("SELECT word, alphagram from words where length = %1 order by probability_order").arg(i);
		zyzzyvaQuery.exec(queryString);
		QString lastAlphagram = "";
		QString lastAlphagramEntry = "";
		int probability = 1;
		while (zyzzyvaQuery.next())
		{
			QString thisAlphagram = zyzzyvaQuery.value(1).toString(); // the alphagram column                                                                                       
			if (thisAlphagram != lastAlphagram)
			{	// new alphagram!
				
			
				if (lastAlphagram != "")
				{
					QString toExecute = "INSERT INTO alphagrams(alphagram, words, length, probability) VALUES(:alphagram, :words, :length, :probability)";
					wordQuery.prepare(toExecute);
					wordQuery.bindValue(":alphagram", lastAlphagram);
					wordQuery.bindValue(":words", lastAlphagramEntry.trimmed());
					wordQuery.bindValue(":length", lastAlphagram.length());
					wordQuery.bindValue(":probability", probability);
					probability++;
					wordQuery.exec();
				}
				
				lastAlphagramEntry = zyzzyvaQuery.value(0).toString() + " ";	// the word    
			}
			else
				lastAlphagramEntry += zyzzyvaQuery.value(0).toString() + " ";

			
			lastAlphagram = thisAlphagram;
		}
		
		QString toExecute = "INSERT INTO alphagrams(alphagram, words, length, probability) VALUES(:alphagram, :words, :length, :probability)";
		wordQuery.prepare(toExecute);
		wordQuery.bindValue(":alphagram", lastAlphagram);
		wordQuery.bindValue(":words", lastAlphagramEntry.trimmed());
		wordQuery.bindValue(":length", lastAlphagram.length());
		wordQuery.bindValue(":probability", probability);
		wordQuery.exec();

	
	}
	

	
	/* top 500 by prob 6s - 9s */

	wordQuery.exec("CREATE TABLE IF NOT EXISTS wordlists(listname VARCHAR(40), wordlength INTEGER, probindices BLOB)");
	wordQuery.exec("CREATE INDEX listname_index on wordlists(listname)");



	
	for (int i = 6; i <= 9; i++)
	{
		int probIndex = 1;
		int numAlphagrams;
		wordQuery.exec(QString("SELECT count(*) from alphagrams where length = %1").arg(i));
		while (wordQuery.next())
		{
			numAlphagrams = wordQuery.value(0).toInt();
		}
		
		int listLength = 500;
		do
		{
			QByteArray ba;
			QDataStream baWriter(&ba, QIODevice::WriteOnly);
			baWriter << (quint8)0 << (quint8)i << (quint16)(probIndex) << (quint16)(probIndex+listLength-1);
			// (quint8)0 means this is a RANGE of indices, and not a list of indices
			QString toExecute = "INSERT INTO wordlists(listname, wordlength, probindices) VALUES(:listname, :wordlength, :probindices)";
			wordQuery.prepare(toExecute);
			wordQuery.bindValue(":listname", QString("Useful %1s (%2 - %3)").arg(i).arg(probIndex).arg(probIndex + listLength-1));
			wordQuery.bindValue(":wordlength", i);
			wordQuery.bindValue(":probindices", ba);
			wordQuery.exec();
			
			probIndex+= 500;
		
		} while (probIndex < numAlphagrams);
	
	
	}

}
