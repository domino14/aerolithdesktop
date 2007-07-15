#include <QCoreApplication>
#include <QtSql>
#include <QFile>
#include <QTextStream>
#include <QString>

void sqlHandler();

int wordlength;
QString filename;

int main (int argc, char **argv)
{
  if (argc != 3) 
    {
      return 0;
    }
  else
    {
      wordlength = QString(argv[1]).toInt();
      filename = QString(argv[2]);
    }

  QCoreApplication app(argc, argv);
  sqlHandler();
  return app.exec();
  
}

void sqlHandler()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("/home/cesar/.zyzzyva/lexicons/OWL2+LWL.db");
  db.open();
  QSqlQuery query;
  QString queryString = QString("SELECT * from words where length = %1 order by probability_order").arg(wordlength);
  query.exec(queryString);
  QString lastAlphagram = "";
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;

  QTextStream out(&file);

  while (query.next()) 
    {
      QString thisAlphagram = query.value(6).toString(); // the alphagram column
      if (thisAlphagram != lastAlphagram)
	out << "\n" << thisAlphagram << " ";
      out << query.value(0).toString() << " ";
      lastAlphagram = thisAlphagram;
    }
  qDebug() << "All done.";
  file.close();

}
