#include "commonDefs.h"
void getUniqueRandomNumbers(QList<quint32>&numbers, quint32 start, quint32 end, int numNums)
{
    // takes all the numbers between start and end, including start and end,
    // randomly shuffles, and returns the first numNums numbers of the shuffled array.

    //  qDebug() << "gurn" << start << end << numNums;
    int size = end - start + 1;
    if (size < 1) size = start - end + 1;
    if (numNums > size) return;

    QList <quint32> pool;
 //   pool.resize(size);
    for (int i = 0; i < size; i++)
    {
        pool << i + start;
    }
    int choose, temp;
    for (int i = 0; i < numNums; i++)
    {
        choose = qrand() % size;
        numbers << pool[choose];
        size--;
        temp = pool[choose];
        pool[choose] = pool[size];
        pool[size] = temp;
    }
}
