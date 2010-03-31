#ifndef SAVEDUNSCRAMBLEGAME_H
#define SAVEDUNSCRAMBLEGAME_H

#include <QtDebug>

struct SavedUnscrambleGame
{
    QSet <quint32> origIndices;
    QSet <quint32> firstMissed;
    bool seenWholeList;
    bool brandNew;
    QSet <quint32> curQuizSet;
    QSet <quint32> curMissedSet;

    void initialize(const QList <quint32>& initialQs)
    {
        initialize(initialQs.toSet());
    }

    void initialize(const QSet <quint32>& initialQs)
    {
        origIndices = initialQs;
        seenWholeList = false;
        brandNew = true;
        curQuizSet.clear();
        curMissedSet.clear();
        firstMissed.clear();

    }

    void initializeWithIndexRange(quint32 low, quint32 high)
    {
        QSet <quint32> set;
        for (quint32 i = low; i <= high; i++)
            set.insert(i);

        initialize(set);

    }

    QByteArray toByteArray()
    {
        QByteArray ba;
        QDataStream baWriter(&ba, QIODevice::WriteOnly);

        baWriter << origIndices;

        if (brandNew)
            baWriter << (quint8)1;
        // this variable is 1 if this list is brand-new (nothing except index data), 0 otherwise.
        else
        {
            baWriter << (quint8)0; // not brand-new
            baWriter << firstMissed;
            baWriter << seenWholeList;
            baWriter << curQuizSet;
            baWriter << curMissedSet;
        }
        return ba;
    }

    void populateFromByteArray(QByteArray ba)
    {
        origIndices.clear();
        firstMissed.clear();
        seenWholeList = false;
        brandNew = false;
        curQuizSet.clear();
        curMissedSet.clear();

        QDataStream baReader(ba);

        baReader >> origIndices;
        baReader >> brandNew;

        if (brandNew)
            return;
        else
        {
            baReader >> firstMissed >> seenWholeList >> curQuizSet >> curMissedSet;
        }

    }
    void writeToDebug()
    {
        qDebug() << "seenWholeList:" << seenWholeList;
        qDebug() << "brandNew:" << brandNew;
        qDebug() << "origIndices:" << origIndices;
        qDebug() << "firstMissed:" << firstMissed;
        qDebug() << "curQuizList:" << curQuizSet;
        qDebug() << "curMissedList:" << curMissedSet;


    }
};


#endif // SAVEDUNSCRAMBLEGAME_H
