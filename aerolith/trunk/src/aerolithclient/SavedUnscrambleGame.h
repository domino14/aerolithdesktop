#ifndef SAVEDUNSCRAMBLEGAME_H
#define SAVEDUNSCRAMBLEGAME_H

struct SavedUnscrambleGame
{
    QSet <quint32> origIndices;
    QSet <quint32> firstMissed;
    bool seenWholeList;
    bool brandNew;
    QSet <quint32> curQuizList;
    QSet <quint32> curMissedList;

    void initialize(QSet <quint32>& initialQs)
    {
        origIndices = initialQs;
        seenWholeList = false;
        brandNew = true;
        curQuizList.clear();
        curMissedList.clear();
        firstMissed.clear();

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
            baWriter << curQuizList;
            baWriter << curMissedList;
        }
        return ba;
    }

    void populateFromByteArray(QByteArray ba)
    {
        origIndices.clear();
        firstMissed.clear();
        seenWholeList = false;
        brandNew = false;
        curQuizList.clear();
        curMissedList.clear();

        QDataStream baReader(ba);

        baReader >> origIndices;
        baReader >> brandNew;

        if (brandNew)
            return;
        else
        {
            baReader >> firstMissed >> seenWholeList >> curQuizList >> curMissedList;
        }

    }
};


#endif // SAVEDUNSCRAMBLEGAME_H
