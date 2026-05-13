#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include "core/timer_manager.h"
#include "api/bouyomi_client.h"
#include "timer_row_widget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onTimerTriggered(const QString& message);
    void updateUI();
    void addCustomTimer();
    void onPresetChanged(int index);

private:
    void setupUI();

    TimerManager* m_timerManager;
    BouyomiClient* m_bouyomiClient;

    // UI Elements
    QLabel* m_currentTimeLabel;
    QLabel* m_remainingTimeLabel;
    QListWidget* m_timerListWidget;
    QComboBox* m_presetCombo;
    QComboBox* m_voiceCombo;
    QSpinBox* m_valueSpin;
    QSpinBox* m_hourSpin;
    QSpinBox* m_minSpin;
    QLineEdit* m_messageEdit;
};

#endif // MAINWINDOW_H
