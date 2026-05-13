#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <QObject>
#include <QDateTime>
#include <QList>
#include <QJsonObject>
#include <QTimer>

struct ScheduledTimer {
    QDateTime targetTime;
    QString message;
    QString voiceTag;  // "y)", "速度(値)" etc.
    int voiceValue;    // Numerical value if applicable
};

class TimerManager : public QObject
{
    Q_OBJECT
public:
    explicit TimerManager(QObject *parent = nullptr);

    bool loadConfig(const QString& path);
    void addTimer(const QDateTime& time, const QString& message, 
                  const QString& voiceTag = "", int voiceValue = 0);
    void removeTimer(int index);
    void clearTimers();

    QList<ScheduledTimer> timers() const { return m_timers; }
    QJsonObject config() const { return m_config; }

    // プリセット計算ロジック
    static QDateTime calculateNextTime(int intervalMinutes);

signals:
    void timerTriggered(const QString& message);
    void timersChanged();

private slots:
    void checkTimers();

private:
    QList<ScheduledTimer> m_timers;
    QJsonObject m_config;
    QTimer* m_checkTimer;
};

#endif // TIMER_MANAGER_H
