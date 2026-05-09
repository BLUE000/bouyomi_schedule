#include <QtTest>
#include <QCoreApplication>
#include "core/timer_manager.h"

class TestBouyomiSchedule : public QObject
{
    Q_OBJECT
private slots:
    void testNextTime15Min();
    void testNextTime30Min();
    void testNextTime60Min();
};

void TestBouyomiSchedule::testNextTime15Min()
{
    // 現在時刻から次の15分刻みを計算
    // 例: 12:05 -> 12:15, 12:14 -> 12:15, 12:15 -> 12:30
    QDateTime next = TimerManager::calculateNextTime(15);
    
    QCOMPARE(next.time().second(), 0);
    QVERIFY(next.time().minute() % 15 == 0);
    QVERIFY(next > QDateTime::currentDateTime());
}

void TestBouyomiSchedule::testNextTime30Min()
{
    // 現在時刻から次の30分刻みを計算
    // 例: 12:05 -> 12:30, 12:35 -> 13:00
    QDateTime next = TimerManager::calculateNextTime(30);
    
    QCOMPARE(next.time().second(), 0);
    QVERIFY(next.time().minute() == 0 || next.time().minute() == 30);
    QVERIFY(next > QDateTime::currentDateTime());
}

void TestBouyomiSchedule::testNextTime60Min()
{
    // 現在時刻から次の正時を計算
    // 例: 12:05 -> 13:00
    QDateTime next = TimerManager::calculateNextTime(60);
    
    QCOMPARE(next.time().second(), 0);
    QCOMPARE(next.time().minute(), 0);
    QVERIFY(next > QDateTime::currentDateTime());
}

QTEST_MAIN(TestBouyomiSchedule)
#include "test_main.moc"
