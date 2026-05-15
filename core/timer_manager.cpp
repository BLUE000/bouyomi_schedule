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

void TimerManager::addTimer(const QDateTime& time, const QString& message, 
                           const QString& voiceTag, int voiceValue)
{
    m_timers.append({time, message, voiceTag, voiceValue});
    
    // 時間順にソート（安定ソートを使用して同刻の登録順を維持）
    std::stable_sort(m_timers.begin(), m_timers.end(), [](const ScheduledTimer& a, const ScheduledTimer& b) {
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
    QTime nowTruncated(now.time().hour(), now.time().minute());
    
    QList<ScheduledTimer> triggered;

    // 先頭（時間の早い順）からチェックしてトリガー対象を取り出す
    for (int i = 0; i < m_timers.size(); ) {
        QTime targetTime = m_timers[i].targetTime.time();
        QTime targetTruncated(targetTime.hour(), targetTime.minute());

        // 時・分が一致する場合
        if (targetTruncated == nowTruncated) {
            triggered.append(m_timers.takeAt(i));
        } else if (m_timers[i].targetTime < now) {
            // 万が一、時分が一致せずに過去になってしまったものも救い上げる（安全策）
            triggered.append(m_timers.takeAt(i));
        } else {
            // ソートされているため、これ以降は未来
            break;
        }
    }

    // 取り出した順（時系列順）に読み上げ
    for (const auto& t : triggered) {
        QString finalMessage = t.message;
        if (!t.voiceTag.isEmpty()) {
            if (t.voiceTag == "速度(値)") {
                finalMessage = QString("速度(%1)%2").arg(t.voiceValue).arg(t.message);
            } else if (t.voiceTag == "音程(値)") {
                finalMessage = QString("音程(%1)%2").arg(t.voiceValue).arg(t.message);
            } else {
                finalMessage = t.voiceTag + t.message;
            }
        }
        emit timerTriggered(finalMessage);
    }

    if (!triggered.isEmpty()) {
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
