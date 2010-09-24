#ifndef WORDGRIDSTABLE_H
#define WORDGRIDSTABLE_H

#include <QObject>
#include "ClientSocket.h"
class WordgridsTable : public QObject
{
public:
    WordgridsTable(QObject* parent);
    void initialize(ClientSocket*, quint16, int);
    void removePersonFromTable(ClientSocket*);
    QList <ClientSocket*> peopleInTable;
    void cleanupBeforeDelete();
    void personJoined(ClientSocket*);
    int boardSize;
private:
    quint16 tableNum;
};

#endif // WORDGRIDSTABLE_H
