#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QDateTime>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_timerManager = new TimerManager(this);
    m_bouyomiClient = new BouyomiClient("localhost", 50080, 50080, this);

    setupUI();
    
    // 設定ファイルの読み込み（デフォルト）
    m_timerManager->loadConfig("config/sample_config.json");

    // シグナル接続
    connect(m_timerManager, &TimerManager::timerTriggered, this, &MainWindow::onTimerTriggered);
    connect(m_timerManager, &TimerManager::timersChanged, this, &MainWindow::updateUI);

    // 1秒ごとのUI更新タイマー
    QTimer* uiTimer = new QTimer(this);
    connect(uiTimer, &QTimer::timeout, this, &MainWindow::updateUI);
    uiTimer->start(1000);
}

void MainWindow::setupUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* layout = new QVBoxLayout(central);

    // --- 表示エリア ---
    QGroupBox* displayGroup = new QGroupBox("Status", this);
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    
    m_currentTimeLabel = new QLabel("--:--:--", this);
    m_currentTimeLabel->setAlignment(Qt::AlignCenter);
    m_currentTimeLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    
    m_remainingTimeLabel = new QLabel("Next: 00:00:00", this);
    m_remainingTimeLabel->setAlignment(Qt::AlignCenter);
    m_remainingTimeLabel->setStyleSheet("font-size: 32px; color: #0078d4; font-weight: bold;");
    
    displayLayout->addWidget(m_currentTimeLabel);
    displayLayout->addWidget(m_remainingTimeLabel);
    layout->addWidget(displayGroup);

    // --- 設定エリア ---
    QGroupBox* setupGroup = new QGroupBox("Add Timer", this);
    QVBoxLayout* setupLayout = new QVBoxLayout(setupGroup);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem("Manual");
    m_presetCombo->addItem("15 min interval", 15);
    m_presetCombo->addItem("30 min interval", 30);
    m_presetCombo->addItem("1 hour interval", 60);
    
    m_hourSpin = new QSpinBox(this); m_hourSpin->setRange(0, 23);
    m_minSpin = new QSpinBox(this); m_minSpin->setRange(0, 59);
    
    m_voiceCombo = new QComboBox(this);
    m_voiceCombo->addItems({"y)", "b)", "h)", "d)", "a)", "r)", "t)", "g)", "やまびこ)", "エコー)", "速度(値)", "音程(値)"});
    m_voiceCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    
    m_valueSpin = new QSpinBox(this);
    m_valueSpin->setRange(0, 9999);
    m_valueSpin->setFixedWidth(60);
    m_valueSpin->setEnabled(false);

    m_messageEdit = new QLineEdit(this);
    m_messageEdit->setPlaceholderText("Message to speak...");

    inputLayout->addWidget(new QLabel("Preset:"));
    inputLayout->addWidget(m_presetCombo);
    inputLayout->addWidget(new QLabel("Time:"));
    inputLayout->addWidget(m_hourSpin);
    inputLayout->addWidget(new QLabel(":"));
    inputLayout->addWidget(m_minSpin);
    setupLayout->addLayout(inputLayout);

    QHBoxLayout* voiceLayout = new QHBoxLayout();
    voiceLayout->addWidget(new QLabel("Voice:"));
    voiceLayout->addWidget(m_voiceCombo);
    voiceLayout->addWidget(m_valueSpin);
    voiceLayout->addWidget(m_messageEdit);
    setupLayout->addLayout(voiceLayout);

    connect(m_voiceCombo, &QComboBox::currentTextChanged, [this](const QString& text) {
        m_valueSpin->setEnabled(text == "速度(値)" || text == "音程(値)");
    });

    QPushButton* addBtn = new QPushButton("Add Timer", this);
    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addCustomTimer);
    setupLayout->addWidget(addBtn);

    layout->addWidget(setupGroup);

    // --- リストエリア ---
    m_timerListWidget = new QListWidget(this);
    layout->addWidget(new QLabel("Scheduled Timers:"));
    layout->addWidget(m_timerListWidget);

    QPushButton* clearBtn = new QPushButton("Clear All", this);
    connect(clearBtn, &QPushButton::clicked, m_timerManager, &TimerManager::clearTimers);
    layout->addWidget(clearBtn);

    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onPresetChanged);

    setWindowTitle("Bouyomi Schedule C++");
    resize(450, 600);
}

void MainWindow::onTimerTriggered(const QString& message)
{
    m_bouyomiClient->speak(message);
}

void MainWindow::updateUI()
{
    QDateTime now = QDateTime::currentDateTime();
    m_currentTimeLabel->setText(now.toString("HH:mm:ss"));

    auto timers = m_timerManager->timers();
    if (timers.isEmpty()) {
        m_remainingTimeLabel->setText("No timers set");
    } else {
        qint64 secs = now.secsTo(timers[0].targetTime);
        if (secs < 0) secs = 0;
        int h = secs / 3600;
        int m = (secs % 3600) / 60;
        int s = secs % 60;
        m_remainingTimeLabel->setText(QString("Next in: %1:%2:%3")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0')));
    }

    // リストの更新
    m_timerListWidget->clear();
    int index = 0;
    for (const auto& t : timers) {
        QListWidgetItem* item = new QListWidgetItem(m_timerListWidget);
        TimerRowWidget* row = new TimerRowWidget(
            index, 
            t.targetTime.toString("HH:mm"), 
            t.voiceTag, 
            t.voiceValue, 
            t.message, 
            this
        );
        
        item->setSizeHint(row->sizeHint());
        m_timerListWidget->addItem(item);
        m_timerListWidget->setItemWidget(item, row);
        
        connect(row, &TimerRowWidget::deleteRequested, m_timerManager, &TimerManager::removeTimer);
        index++;
    }
}

void MainWindow::addCustomTimer()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime target = now;
    target.setTime(QTime(m_hourSpin->value(), m_minSpin->value()));
    
    if (target <= now) target = target.addDays(1);
    
    m_timerManager->addTimer(target, m_messageEdit->text(), m_voiceCombo->currentText(), m_valueSpin->value());
}

void MainWindow::onPresetChanged(int index)
{
    if (index == 0) return; // Manual
    
    int interval = m_presetCombo->currentData().toInt();
    QDateTime next = TimerManager::calculateNextTime(interval);
    
    m_hourSpin->setValue(next.time().hour());
    m_minSpin->setValue(next.time().minute());
}
