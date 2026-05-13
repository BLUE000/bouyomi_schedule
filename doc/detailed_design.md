# 詳細設計書: BouyomiSchedule (C++ Port)

## 1. クラス設計

### 1.1 `BouyomiClient` (API)
棒読みちゃんとの通信を抽象化するクラス。
- **メソッド**:
    - `speak(message, speed, tone, volume, voice)`: 統合送信メソッド。
    - `speakHttp(...)`: GET リクエストによる送信。
    - `speakTcp(...)`: バイナリパケットによる送信。
- **パケット構造 (TCP)**:
    - 0-1 byte: `0x0001` (Command, Little Endian)
    - 2-9 byte: `speed`, `tone`, `volume`, `voice` (int16 x 4)
    - 10 byte: `0x00` (Encoding: UTF-8)
    - 11-14 byte: `length` (int32, Message length)
    - 15- byte: `message` (UTF-8 string)

### 1.2 `TimerManager` (Core)
タイマーのライフサイクルと計算を担当。
- **データ構造**: `QList<ScheduledTimer>` (時刻とメッセージのペア)。
- **計算ロジック**:
    - `calculateNextTime(interval)`: 
        - 60分: 現在時刻の次の `hh:00:00` を計算。
        - 30分: 次の `00分` または `30分` を計算。
        - 15分: `00, 15, 30, 45分` のうち最も近い未来の時刻を計算。

### 1.3 `MainWindow` (UI)
- **タイマー監視**: `QTimer` で 1秒ごとに UI（現在時刻、残り時間）を更新。
- **入力バリデーション**: 時刻（0-23時, 0-59分）の範囲チェック。
- **スケジュール行のレイアウト**:
    - `QHBoxLayout` を使用し、各ウィジェットを水平に配置。
    - **声質プルダウン (`QComboBox`)**:
        - `fixedWidth` をフォントメトリクスに基づき設定。
        - アイテム: `y)`, `b)`, `h)`, `d)`, `a)`, `r)`, `t)`, `g)`, `やまびこ)`, `エコー)`, `速度(値)`, `音程(値)`。
    - **数値入力 (`QSpinBox` または `QLineEdit`)**:
        - `setFixedWidth` で4桁分に固定。
        - デフォルトは `setEnabled(false)`。
    - **メッセージ入力 (`QLineEdit`)**:
        - `sizePolicy` を `Expanding` に設定し、残りの幅を占有。
    - **削除ボタン (`QPushButton`)**:
        - アイコン（ゴミ箱等）または「×」テキストを表示。
        - クリック時に当該行のデータを破棄する。
- **動的制御ロジック**:
    - `QComboBox::currentIndexChanged` シグナルをスロットに接続。
    - 選択された文字列が「速度(値)」または「音程(値)」を含む場合のみ、数値入力の `setEnabled(true)` を呼び出す。
    - **削除処理**: 削除ボタンの `clicked` シグナルを受け取り、UIリストおよび `TimerManager` の対応するスケジュールデータを削除。

## 2. 設定ファイル仕様 (`config/sample_config.json`)
Python版と完全互換を維持する。
```json
{
  "bouyomi": {
    "host": "localhost",
    "port": 50080,
    "repeat_count": 1
  },
  "timer_presets": [
    { "name": "15min", "interval_minutes": 15, "message": "15分経過" }
  ]
}
```

## 3. 並列処理
- 棒読みちゃんへの送信処理は、UI スレッドをブロックしないよう非同期（QNetworkReply またはスレッド）で実行する。
- 繰り返し通知のリトライ待機は `QTimer` または `QThread::sleep`（別スレッド内）を使用する。
