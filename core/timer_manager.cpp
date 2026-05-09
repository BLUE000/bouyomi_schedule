#include "timer_manager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <algorithm>

TimerManager::TimerManager(QObject *parent) : QObject(parent)
{
    m_checkTimer = new QTimer(this);
    connect(m_checkTimer, &QTimer::timeout, this, &TimerManager::checkTimers);
    m_checkTimer->start(1000); // 1秒ごとにチェック
}

bool TimerManager::loadConfig(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) return false;

    m_config = doc.object();
    return true;
}

void TimerManager::addTimer(const QDateTime& time, const QString& message)
{
    m_timers.append({time, message});
    
    // 時間順にソート
    std::sort(m_timers.begin(), m_timers.end(), [](const ScheduledTimer& a, const ScheduledTimer& b) {
        return a.targetTime < b.targetTime;
    });
    
    emit timersChanged();
}

void TimerManager::removeTimer(int index)
{
    if (index >= 0 && index < m_timers.size()) {
        m_timers.removeAt(index);
        emit timersChanged();
    }
}

void TimerManager::clearTimers()
{
    m_timers.clear();
    emit timersChanged();
}

void TimerManager::checkTimers()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<int> triggeredIndices;

    for (int i = 0; i < m_timers.size(); ++i) {
        if (now >= m_timers[i].targetTime) {
            triggeredIndices.append(i);
        }
    }

    // 逆順に削除（インデックスがずれないように）
    for (int i = triggeredIndices.size() - 1; i >= 0; --i) {
        int idx = triggeredIndices[i];
        QString msg = m_timers[idx].message;
        m_timers.removeAt(idx);
        emit timerTriggered(msg);
    }

    if (!triggeredIndices.isEmpty()) {
        emit timersChanged();
    }
}

QDateTime TimerManager::calculateNextTime(int intervalMinutes)
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime target = now;
    
    // 秒とミリ秒を切り捨て
    target.setTime(QTime(target.time().hour(), target.time().minute(), 0, 0));

    if (intervalMinutes == 60) { // 1時間ごと (hh:00)
        target = target.addSecs(3600 - (target.time().minute() * 60));
    }
    else if (intervalMinutes == 30) { // 30分ごと (00分または30分)
        if (target.time().minute() < 30) {
            target = target.addSecs((30 - target.time().minute()) * 60);
        } else {
            target = target.addSecs((60 - target.time().minute()) * 60);
        }
    }
    else if (intervalMinutes == 15) { // 15分ごと (00, 15, 30, 45分)
        int currentMin = target.time().minute();
        int nextMin = ((currentMin / 15) + 1) * 15;
        target = target.addSecs((nextMin - currentMin) * 60);
    }
    else {
        // それ以外は単純加算して5分単位に切り上げ
        target = now.addSecs(intervalMinutes * 60);
        int roundedMin = ((target.time().minute() + 4) / 5) * 5;
        if (roundedMin >= 60) {
            target = target.addSecs((60 - target.time().minute()) * 60);
        } else {
            target.setTime(QTime(target.time().hour(), roundedMin, 0));
        }
    }

    return target;
}
