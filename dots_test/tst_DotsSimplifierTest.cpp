/* This software is developed by caoweiquan322 OR DynamicFatty.
 * All rights reserved.
 *
 * Author: caoweiquan322
 */

#include <QString>
#include <QtTest>

class DotsSimplifierTest : public QObject
{
    Q_OBJECT

public:
    DotsSimplifierTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCase1_data();
    void testCase1();
};

DotsSimplifierTest::DotsSimplifierTest()
{
}

void DotsSimplifierTest::initTestCase()
{
}

void DotsSimplifierTest::cleanupTestCase()
{
}

void DotsSimplifierTest::testCase1_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void DotsSimplifierTest::testCase1()
{
    QFETCH(QString, data);
    QVERIFY2(false, "Failure");
}

QTEST_APPLESS_MAIN(DotsSimplifierTest)

#include "tst_DotsSimplifierTest.moc"
