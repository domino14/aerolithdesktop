#ifndef _LISTMAKER_H_
#define _LISTMAKER_H_

#include <QtSql>

class ListMaker
{
	public:
		ListMaker();
		static void sqlListMaker(QString queryString, QString listName, quint8 wordLength);
		static void createListDatabase();
		static void testDatabaseTime();
};

#endif
