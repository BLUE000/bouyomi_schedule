#include "timer_row_widget.h"

TimerRowWidget::TimerRowWidget(int index, const QString& time, const QString& voice, int value, const QString& message, QWidget* parent)
    : QWidget(parent), m_index(index)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 2, 5, 2);

    m_timeLabel = new QLabel(time, this);
    m_timeLabel->setFixedWidth(50);

    m_voiceCombo = new QComboBox(this);
    m_voiceCombo->addItems({"y)", "b)", "h)", "d)", "a)", "r)", "t)", "g)", "やまびこ)", "エコー)", "速度(値)", "音程(値)"});
    m_voiceCombo->setCurrentText(voice);
    m_voiceCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    m_valueSpin = new QSpinBox(this);
    m_valueSpin->setRange(0, 9999);
    m_valueSpin->setValue(value);
    m_valueSpin->setFixedWidth(60);
    
    m_messageEdit = new QLineEdit(message, this);
    m_messageEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_deleteBtn = new QPushButton("×", this);
    m_deleteBtn->setFixedWidth(30);
    m_deleteBtn->setStyleSheet("color: red; font-weight: bold;");

    layout->addWidget(m_timeLabel);
    layout->addWidget(m_voiceCombo);
    layout->addWidget(m_valueSpin);
    layout->addWidget(m_messageEdit);
    layout->addWidget(m_deleteBtn);

    connect(m_voiceCombo, &QComboBox::currentTextChanged, this, &TimerRowWidget::onVoiceChanged);
    connect(m_deleteBtn, &QPushButton::clicked, [this]() { emit deleteRequested(m_index); });

    // 初期状態の制御
    onVoiceChanged(m_voiceCombo->currentText());
}

void TimerRowWidget::onVoiceChanged(const QString& text)
{
    bool needsValue = (text == "速度(値)" || text == "音程(値)");
    m_valueSpin->setEnabled(needsValue);
}
