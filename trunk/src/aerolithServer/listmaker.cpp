#include "listmaker.h"

ListMaker::ListMaker()
{

}

void ListMaker::createListDatabase()
{
	// creates a word list database.
	// requires the zyzzyva database to be in the user's install directory

	if (QFile::exists("words.db")) return;
	QSqlDatabase zyzzyvaDb;
	QSqlDatabase wordDb;
	
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
	
	wordDb = QSqlDatabase::addDatabase("QSQLITE", "wordDB");
	wordDb.setDatabaseName("words.db");
	wordDb.open();
	
	QSqlQuery wordQuery("wordDB");
	wordQuery.exec("CREATE TABLE IF NOT EXISTS alphagrams(alphagram VARCHAR(15), words VARCHAR(255), length INTEGER)");
	wordQuery.exec("CREATE UNIQUE INDEX alphagram_index on alphagrams(alphagram)");
	
 	QSqlQuery zyzzyvaQuery("zyzzyvaDB");
	
	for (int i = 2; i < 15; i++)
	{
		qDebug() << "Length" << i;
		QString queryString = QString("SELECT word, alphagram from words where length = %1 order by probability_order").arg(i);
		zyzzyvaQuery.exec(queryString);
		QString lastAlphagram = "";
		QString lastAlphagramEntry = "";
		
		while (zyzzyvaQuery.next())
    {
      QString thisAlphagram = zyzzyvaQuery.value(1).toString(); // the alphagram column                                                                                       
      if (thisAlphagram != lastAlphagram)
      {	// new alphagram!
				
      
      	if (lastAlphagram != "")
      	{
      		QString toExecute = "INSERT INTO alphagrams(alphagram, words, length) VALUES(:alphagram, :words, :length)";
					wordQuery.prepare(toExecute);
					wordQuery.bindValue(":alphagram", lastAlphagram);
					wordQuery.bindValue(":words", lastAlphagramEntry);
					wordQuery.bindValue(":length", lastAlphagram.length());
					wordQuery.exec();
				}
				
  			lastAlphagramEntry = zyzzyvaQuery.value(0).toString() + " ";	// the word    
      }
      else
      	lastAlphagramEntry += zyzzyvaQuery.value(0).toString() + " ";

			
      lastAlphagram = thisAlphagram;
    }

	
	}
	wordDb.close();

}