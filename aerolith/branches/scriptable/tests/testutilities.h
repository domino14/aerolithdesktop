#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <QtTest/QtTest>
#include "commonDefs.h"
class TestUtilities : public QObject
{
Q_OBJECT

private slots:
    void initTestCase();
    void testGetUniqueRandomNumbers();
    void testShuffle();
};

#endif // TESTUTILITIES_H
