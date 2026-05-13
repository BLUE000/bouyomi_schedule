#ifndef TIMER_ROW_WIDGET_H
#define TIMER_ROW_WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

class TimerRowWidget : public QWidget {
    Q_OBJECT
public:
    explicit TimerRowWidget(int index, const QString& time, const QString& voice, int value, const QString& message, QWidget* parent = nullptr);

signals:
    void deleteRequested(int index);

private slots:
    void onVoiceChanged(const QString& text);

private:
    int m_index;
    QLabel* m_timeLabel;
    QComboBox* m_voiceCombo;
    QSpinBox* m_valueSpin;
    QLineEdit* m_messageEdit;
    QPushButton* m_deleteBtn;
};

#endif // TIMER_ROW_WIDGET_H
