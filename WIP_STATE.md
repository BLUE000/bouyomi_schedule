# WIP_STATE: BouyomiSchedule Porting (Python to C++)

## 📅 Status: 2026-05-09
Python版 `bouyomi_schedule` から C++ (Qt6) への移植を開始。
基本的な通信・ロジック・UIの雛形作成が完了。

## ✅ Completed Tasks
- [x] プロジェクト構成の定義 (api, core, ui)
- [x] CMakeLists.txt の作成
- [x] BouyomiClient (通信クラス) の移植実装
- [x] TimerManager (タイマーロジック) の移植実装
- [x] MainWindow (UI) の基本実装
- [x] build.bat, .gitignore, .ai_rules_local.md の整備

## 🚧 In Progress
- [ ] ビルド確認とデバッグ
- [ ] 詳細設計ドキュメントの作成 (`doc/detailed_design.md`)
- [ ] 設定ファイルの完全な互換性確認

## 🎯 Next Steps
1. `build.bat` を実行し、コンパイルエラーの修正。
2. サンプル設定ファイルを配置し、実動作（棒読みちゃんへの送信）を確認。
3. リソースファイル（アイコン等）の追加。
