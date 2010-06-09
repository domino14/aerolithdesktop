#include "testutilities.h"
extern const quint16 MAGIC_NUMBER = 0; // needed by commonDefs.h

void TestUtilities::initTestCase()
{
    qsrand(QDateTime::currentDateTime().toTime_t());
}

void TestUtilities::testGetUniqueRandomNumbers()
{
    QList <quint32> indexList;
    int low = 15;
    int high = 30000;
    int numTotalRacks = high - low + 1;
    QBENCHMARK{
    Utilities::getUniqueRandomNumbers(indexList, low, high, numTotalRacks);
}
    QCOMPARE(indexList.size(), numTotalRacks);

    QSet <quint32> set;
    foreach (quint32 index, indexList)
        set.insert(index);

    QCOMPARE(set.size(), numTotalRacks);   // set guarantees unique numbers.
    // this completes the duplicates test.
}

void TestUtilities::testShuffle()
{
    int low = 10000;
    int high = 100000;
    int numTotalRacks = high - low + 1;
    QList <quint32> indexList;
    Utilities::getUniqueRandomNumbers(indexList, low, high, numTotalRacks);

    QBENCHMARK{
    Utilities::shuffle(indexList);
    }
    QCOMPARE(indexList.size(), numTotalRacks);

    QSet <quint32> set;
    foreach (quint32 index, indexList)
        set.insert(index);

    QCOMPARE(set.size(), numTotalRacks);   // set guarantees unique numbers.
    QVERIFY(indexList.at(0) != 10000);  // well, sometimes this test will fail.

}

QTEST_MAIN(TestUtilities);
//#include "testutilities.moc"

