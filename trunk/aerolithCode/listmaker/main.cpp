#include <QDir>
#include <QCoreApplication>
#include <QtSql>
#include <QFile>
#include <QTextStream>
#include <QString>

void sqlListMaker(QString queryString, QString filename);
void sqlInitializer();
void splitLists(QString);
void listInfoWriter();

QSqlDatabase db;
QFile listsDescriptor("lists/LISTS");
QTextStream listsStream(&listsDescriptor);
const char SEPARATOR = '@';

int main (int argc, char **argv)
{

  QCoreApplication app(argc, argv);
  sqlInitializer();
  listsDescriptor.open(QIODevice::WriteOnly | QIODevice::Text);

  

  for (int x = 4; x <= 9; x++)
    {
      QString queryString = 
	QString("SELECT * from words where length = %1 order by probability_order").arg(x);
      sqlListMaker(queryString, QString("%1s").arg(x));
    }
    
  QString vowelQueryString = "SELECT * from words where length = %1 and num_vowels = %2 order by probability_order";


  sqlListMaker(vowelQueryString.arg(8).arg(5), "5vowel8s");
  sqlListMaker(vowelQueryString.arg(7).arg(4), "4vowel7s");
  QString jqxzQueryString = "SELECT * from words where length = %1 and (alphagram like '%Q%' or alphagram like '%J%' or alphagram like '%X%' or alphagram like '%Z%') order by probability_order";

  for (int i = 4; i <= 8; i++)
    sqlListMaker(jqxzQueryString.arg(i), QString("JQXZ_%1s").arg(i));
  
  for (int i = 6; i <= 9; i++)
    splitLists(QString("%1s").arg(i));


  listInfoWriter();
  listsDescriptor.close();
  qDebug() << " Final.";
  //  return app.exec();
  
}

void splitLists(QString filename)
{
  // split into 1-1000, etc
  QFile file("lists/" + filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QTextStream in(&file);
  
  int linecounter = 0;

  bool shouldKeepGoing = true;

  while(shouldKeepGoing)
    {
      QFile output(QString("lists/%1s__%2_to_%3").arg(filename.left(1))
		   .arg(linecounter + 1).arg(linecounter + 1000));
      if (!output.open(QIODevice::WriteOnly | QIODevice::Text)) return;
      QTextStream out(&output);

      for (int i = 1; i <= 1000; i++)
	{
	  if (in.atEnd())
	    {
	      shouldKeepGoing = false;
	      output.close();
	      break;


	    }
	  QString tmp = in.readLine();
	  out << tmp << "\n";
	}
      linecounter += 1000;
     
      output.close();


    }
  file.close();
  //  output.close();
  
  qDebug() << "done splitting " << filename;
}

void sqlInitializer()
{
  db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName(QDir::homePath()+"/.zyzzyva/lexicons/OWL2+LWL.db");
  db.open();

}

void listInfoWriter()
{
  QDir directory("lists");
  QStringList filenameList = directory.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time);
  qDebug() << "Files: " << filenameList;
  foreach (QString filename, filenameList)
    {
      if (filename.left(5) == "LISTS") continue;

  
      if (filename.contains("JQXZ"))
	{
	  listsStream << filename << SEPARATOR;
	  listsStream << "JQXZ " << filename.right(2) << "\n";
	}
      else if (filename.contains("__"))
	{
	  QStringList tmp = filename.split("_", QString::SkipEmptyParts);
	  qDebug() << tmp;
	  QString blah;
	  if (tmp.at(0).left(1).toInt() < 7 || tmp.at(0).left(1).toInt() > 8) continue;
	  if (tmp.at(0).left(1).toInt() == 7) blah = "Essential Sevens";
	  if (tmp.at(0).left(1).toInt() == 8) blah = "Essential Eights";
	  
	  if (tmp.at(1).toInt() < 10000)
	    {
	      listsStream << filename << SEPARATOR;
	      listsStream << blah << QString(" (%1 - %2)\n").arg(tmp.at(1)).arg(tmp.at(3));
	    }
	}
      else if (filename.contains("vowel"))
	{
	  listsStream << filename << SEPARATOR;
	  QStringList tmp = filename.split("vowel");
	  listsStream << tmp.at(0) << " vowel " << tmp.at(1) << "\n";
	  
	}
      else
	{
	  listsStream << filename << SEPARATOR;
	  listsStream << "Official " << filename << "\n";
	  
	}
    }
}

void sqlListMaker(QString queryString, QString filename)
{
  QSqlQuery query;
  query.exec(queryString);
  QString lastAlphagram = "";
  QFile file("lists/" + filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);

  while (query.next()) 
    {
      QString thisAlphagram = query.value(6).toString(); // the alphagram column
      if (thisAlphagram != lastAlphagram)
	out << ((lastAlphagram == "") ? "" : "\n") << thisAlphagram << " ";

      out << query.value(0).toString() << " ";
      lastAlphagram = thisAlphagram;
    }

  qDebug() << "All done with " << queryString;
  file.close();

}
