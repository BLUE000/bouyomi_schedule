import tkinter as tk
from tkinter import ttk, messagebox
import json
import sys
import os
from datetime import datetime, timedelta
from bouyomi_client import BouyomiClient
import threading
import time

class TimerApp:
    """タイマーアプリケーション"""
    
    def __init__(self, config_path):
        # 設定ファイルを読み込み
        self.config = self.load_config(config_path)
        
        # ロックファイルで同時起動を制御
        self.lock_file = f".lock_{self.config['tag']}"
        if not self.acquire_lock():
            messagebox.showerror("エラー", f"タグ '{self.config['tag']}' のタイマーは既に起動しています")
            sys.exit(1)
        
        # 棒読みちゃんクライアント
        bouyomi_config = self.config['bouyomi']
        self.bouyomi = BouyomiClient(
            host=bouyomi_config['host'],
            port=bouyomi_config['port']
        )
        
        # タイマーリスト
        self.timers = []  # [(datetime, message), ...]
        
        # メインウィンドウを作成
        self.root = tk.Tk()
        self.setup_main_window()
        
        # 設定ウィンドウ（最初は非表示）
        self.settings_window = None
        
        # 更新タイマーを開始
        self.update_display()
        self.check_timers()
        
        # 終了時の処理
        self.root.protocol("WM_DELETE_WINDOW", self.on_closing)
    
    def load_config(self, config_path):
        """設定ファイルを読み込む"""
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
                
                # ウィンドウ幅の最小値を設定（設定値が小さすぎる場合は上書き）
                min_width = 400  # 最小幅を400pxに設定
                if 'window' in config and 'width' in config['window']:
                    if config['window']['width'] < min_width:
                        config['window']['width'] = min_width
                
                return config
        except Exception as e:
            messagebox.showerror("エラー", f"設定ファイルの読み込みに失敗しました:\n{e}")
            sys.exit(1)
    
    def acquire_lock(self):
        """ロックファイルを取得"""
        if os.path.exists(self.lock_file):
            return False
        try:
            with open(self.lock_file, 'w') as f:
                f.write(str(os.getpid()))
            return True
        except:
            return False
    
    def release_lock(self):
        """ロックファイルを解放"""
        try:
            if os.path.exists(self.lock_file):
                os.remove(self.lock_file)
        except:
            pass
    
    def setup_main_window(self):
        """メインウィンドウをセットアップ"""
        window_config = self.config['window']
        
        # ウィンドウの幅を設定値または最小値に設定
        width = max(window_config.get('width', 400), 400)  # 最小幅を400pxに設定
        
        self.root.title(window_config['title'])
        self.root.geometry(f"{width}x{window_config['height']}")
        self.root.configure(bg=window_config['background_color'])
        
        # 時間表示用のフレーム（横並び）
        time_frame = tk.Frame(self.root, bg=window_config['background_color'])
        time_frame.pack(pady=30)
        
        # 現在時刻表示用ラベル
        self.current_time_label = tk.Label(
            time_frame,
            text="--:--",
            font=(self.config['display']['font_family'], self.config['display']['current_time']['font_size']),
            fg=self.config['display']['current_time']['color'],
            bg=window_config['background_color']
        )
        self.current_time_label.pack(side='left')
        
        # 区切り文字
        tk.Label(
            time_frame,
            text=" / ",
            font=(self.config['display']['font_family'], self.config['display']['current_time']['font_size']),
            fg=self.config['display']['current_time']['color'],
            bg=window_config['background_color']
        ).pack(side='left')
        
        # 残り時間表示用ラベル
        self.remaining_time_label = tk.Label(
            time_frame,
            text="00:00:00",
            font=(self.config['display']['font_family'], self.config['display']['remaining_time']['font_size']),
            fg=self.config['display']['remaining_time']['color'],
            bg=window_config['background_color']
        )
        self.remaining_time_label.pack(side='left')
        
        # 設定ボタン
        self.settings_button = tk.Button(
            self.root,
            text="設定",
            font=(self.config['display']['font_family'], 14),
            command=self.open_settings,
            width=10,
            bg="#333333",
            fg="#FFFFFF"
        )
        self.settings_button.pack(pady=20)

    def update_display(self):
        """表示を更新"""
        # 現在時刻を更新
        now = datetime.now()
        current_time_config = self.config['display']['current_time']
        time_str = now.strftime(current_time_config['format'])
        self.current_time_label.config(text=time_str)
        
        # 残り時間を更新
        remaining_str = "00:00:00"  # デフォルトは00:00:00に変更
        if self.timers:
            # 最も近いタイマーを取得
            next_timer = min(self.timers, key=lambda x: x[0])
            remaining = next_timer[0] - now
            
            if remaining.total_seconds() > 0:
                hours = int(remaining.total_seconds() // 3600)
                minutes = int((remaining.total_seconds() % 3600) // 60)
                seconds = int(remaining.total_seconds() % 60)
                
                remaining_time_config = self.config['display']['remaining_time']
                format_str = remaining_time_config['format']
                remaining_str = format_str.format(hours=hours, minutes=minutes, seconds=seconds)
            else:
                # タイマーのメッセージを表示
                remaining_str = next_timer[1]
        
        # 残り時間を表示
        self.remaining_time_label.config(text=remaining_str)
        
        # 1秒後に再度更新
        self.root.after(1000, self.update_display)

    def check_timers(self):
        """タイマーをチェック"""
        now = datetime.now()
        triggered_timers = []
        
        for timer_time, message in self.timers:
            if now >= timer_time:
                triggered_timers.append((timer_time, message))
        
        # トリガーされたタイマーを処理
        for timer_time, message in triggered_timers:
            self.timers.remove((timer_time, message))
            self.notify(message)
        
        # 1秒後に再度チェック
        self.root.after(1000, self.check_timers)
    
    def notify(self, message):
        """通知を送信"""
        bouyomi_config = self.config['bouyomi']
        repeat_count = bouyomi_config['repeat_count']
        repeat_interval = bouyomi_config['repeat_interval_sec']
        
        # 画面にもメッセージを表示
        self.remaining_time_label.config(text=message)
        
        def repeat_notify():
            try:
                for i in range(repeat_count):
                    # 棒読みちゃんに送信
                    success = self.bouyomi.speak(message)
                    if not success:
                        print(f"棒読みちゃんへの送信に失敗しました（試行 {i+1}/{repeat_count}）")
                    
                    if i < repeat_count - 1:
                        time.sleep(repeat_interval)
            except Exception as e:
                print(f"棒読みちゃん通知エラー: {e}")
        
        # 別スレッドで実行
        thread = threading.Thread(target=repeat_notify, daemon=True)
        thread.start()

    def open_settings(self):
        """設定ウィンドウを開く"""
        if self.settings_window is not None and self.settings_window.winfo_exists():
            self.settings_window.lift()
            return
        
        self.settings_window = tk.Toplevel(self.root)
        self.settings_window.title("タイマー設定")
        self.settings_window.geometry("500x600")
        
        # プリセット選択
        frame1 = tk.Frame(self.settings_window)
        frame1.pack(pady=10, padx=10, fill='x')
        
        tk.Label(frame1, text="プリセット:", font=('メイリオ', 10)).pack(side='left')
        
        self.preset_var = tk.StringVar()
        # カスタムを除外したプリセット名のリストを作成
        preset_names = [p['name'] for p in self.config['timer_presets'] 
                       if p['name'].lower() != "カスタム" and p['name'].lower() != "custom"]
        
        self.preset_combo = ttk.Combobox(
            frame1,
            textvariable=self.preset_var,
            values=preset_names,
            state='readonly',
            width=20
        )
        self.preset_combo.pack(side='left', padx=5)
        self.preset_combo.bind('<<ComboboxSelected>>', self.on_preset_selected)
        
        # 手動入力 - 時刻指定に変更
        frame2 = tk.Frame(self.settings_window)
        frame2.pack(pady=10, padx=10, fill='x')
        
        tk.Label(frame2, text="または時刻指定:", font=('メイリオ', 10)).pack(anchor='w')
        
        # 時刻入力フレーム
        frame2_1 = tk.Frame(frame2)
        frame2_1.pack(anchor='w', pady=5)
        
        # デフォルト値を00時00分に設定
        default_hour = 0
        default_minute = 0
        
        self.hour_var = tk.StringVar(value=f"{default_hour:02d}")
        self.minute_var = tk.StringVar(value=f"{default_minute:02d}")
        
        # 時間入力（0-23）
        hour_frame = tk.Frame(frame2_1)
        hour_frame.pack(side='left', padx=(0, 10))
        
        tk.Label(hour_frame, text="時:", font=('メイリオ', 10)).pack(side='left')
        hour_spinbox = ttk.Spinbox(
            hour_frame, 
            from_=0, 
            to=23, 
            width=2, 
            textvariable=self.hour_var,
            wrap=True,
            format="%02.0f"
        )
        hour_spinbox.pack(side='left', padx=2)
        
        # 分入力（0-55、5分単位）
        minute_frame = tk.Frame(frame2_1)
        minute_frame.pack(side='left')
        
        tk.Label(minute_frame, text="分:", font=('メイリオ', 10)).pack(side='left')
        minute_spinbox = ttk.Spinbox(
            minute_frame, 
            from_=0, 
            to=55,  # 5分単位なので最大は55
            increment=5,  # 5分単位で増減
            width=2, 
            textvariable=self.minute_var,
            wrap=True,
            format="%02.0f"
        )
        minute_spinbox.pack(side='left', padx=2)
        
        # メッセージ入力
        frame3 = tk.Frame(self.settings_window)
        frame3.pack(pady=10, padx=10, fill='x')
        
        tk.Label(frame3, text="通知メッセージ:", font=('メイリオ', 10)).pack(anchor='w')
        
        self.message_var = tk.StringVar()
        tk.Entry(frame3, textvariable=self.message_var, width=40).pack(anchor='w', pady=5)
        
        # タイマー追加ボタン
        tk.Button(
            self.settings_window,
            text="タイマー追加",
            command=self.add_timer,
            font=('メイリオ', 10)
        ).pack(pady=10)
        
        # 登録済みタイマーリスト
        tk.Label(
            self.settings_window,
            text="─── 登録済みタイマー ───",
            font=('メイリオ', 10)
        ).pack(pady=10)
        
        # スクロール可能なフレーム
        list_frame = tk.Frame(self.settings_window)
        list_frame.pack(pady=10, padx=10, fill='both', expand=True)
        
        scrollbar = tk.Scrollbar(list_frame)
        scrollbar.pack(side='right', fill='y')
        
        self.timer_listbox = tk.Listbox(
            list_frame,
            yscrollcommand=scrollbar.set,
            font=('メイリオ', 9),
            height=10
        )
        self.timer_listbox.pack(side='left', fill='both', expand=True)
        scrollbar.config(command=self.timer_listbox.yview)
        
        self.update_timer_list()
        
        # 削除ボタン
        button_frame = tk.Frame(self.settings_window)
        button_frame.pack(pady=5)
        
        tk.Button(
            button_frame,
            text="選択を削除",
            command=self.delete_selected_timer,
            font=('メイリオ', 10)
        ).pack(side='left', padx=5)
        
        tk.Button(
            button_frame,
            text="全クリア",
            command=self.clear_all_timers,
            font=('メイリオ', 10)
        ).pack(side='left', padx=5)
        
        # 保存・キャンセルボタン
        bottom_frame = tk.Frame(self.settings_window)
        bottom_frame.pack(pady=10)
        
        tk.Button(
            bottom_frame,
            text="保存",
            command=self.save_settings,
            font=('メイリオ', 12),
            width=10
        ).pack(side='left', padx=10)
        
        tk.Button(
            bottom_frame,
            text="キャンセル",
            command=self.cancel_settings,
            font=('メイリオ', 12),
            width=10
        ).pack(side='left', padx=10)

    def on_preset_selected(self, event):
        """プリセット選択時の処理"""
        preset_name = self.preset_var.get()
        for preset in self.config['timer_presets']:
            if preset['name'] == preset_name:
                if preset['interval_minutes'] is not None:
                    # 現在時刻を取得
                    now = datetime.now()
                    interval_minutes = preset['interval_minutes']
                    
                    # 時間区切りに基づいてターゲット時間を計算
                    if interval_minutes == 60:  # 1時間ごと (hh:00)
                        # 次の正時を計算
                        target_hour = now.hour
                        if now.minute > 0 or now.second > 0:
                            target_hour += 1
                        target_hour = target_hour % 24  # 24時間表記に調整
                        target_time = datetime(now.year, now.month, now.day, target_hour, 0)
                    
                    elif interval_minutes == 30:  # 30分ごと (hh:00 または hh:30)
                        # 現在時刻の分が30以上なら次の時間の00分、そうでなければ現在時間の30分
                        target_hour = now.hour
                        target_minute = 30
                        if now.minute >= 30:
                            target_hour = (target_hour + 1) % 24
                            target_minute = 0
                        target_time = datetime(now.year, now.month, now.day, target_hour, target_minute)
                    
                    elif interval_minutes == 15:  # 15分ごと (hh:00, hh:15, hh:30, hh:45)
                        target_hour = now.hour
                        # 現在の分を15分単位で切り上げ
                        current_minute = now.minute
                        if current_minute % 15 == 0 and now.second == 0:
                            # 丁度15分区切りの場合、次の15分区切りに
                            target_minute = (current_minute + 15) % 60
                            if target_minute == 0:
                                target_hour = (target_hour + 1) % 24
                        else:
                            # 15分単位で切り上げ
                            target_minute = ((current_minute // 15) * 15 + 15) % 60
                            if target_minute == 0:
                                target_hour = (target_hour + 1) % 24
                        target_time = datetime(now.year, now.month, now.day, target_hour, target_minute)
                    
                    else:  # その他の時間間隔はデフォルトの動作（現在時刻から加算）
                        target_time = now + timedelta(minutes=interval_minutes)
                        # 5分単位に丸める
                        rounded_minute = (target_time.minute // 5) * 5
                        target_time = datetime(target_time.year, target_time.month, target_time.day, 
                                              target_time.hour, rounded_minute)
                    
                    # 日付をまたぐ場合の調整
                    if target_time <= now:
                        target_time += timedelta(days=1)
                    
                    # 時刻を設定
                    self.hour_var.set(f"{target_time.hour:02d}")
                    self.minute_var.set(f"{target_time.minute:02d}")
                
                # メッセージを設定
                self.message_var.set(preset['message'])
                break
    
    def add_timer(self):
        """タイマーを追加"""
        try:
            # 時刻指定から目標時刻を計算
            hour = int(self.hour_var.get())
            minute = int(self.minute_var.get())
            # 5分単位に丸める
            minute = (minute // 5) * 5
            message = self.message_var.get()
            
            if hour < 0 or hour > 23 or minute < 0 or minute > 59:
                messagebox.showerror("エラー", "有効な時刻を入力してください（時: 0-23, 分: 0-59）")
                return
            
            if not message:
                messagebox.showerror("エラー", "メッセージを入力してください")
                return
            
            # 現在時刻を取得
            now = datetime.now()
            
            # 指定された時刻で今日の日付のdatetimeオブジェクトを作成
            target_time = datetime(now.year, now.month, now.day, hour, minute)
            
            # 指定時刻が現在時刻より前の場合は翌日の同時刻に設定
            if target_time <= now:
                target_time += timedelta(days=1)
            
            # タイマーを追加
            self.timers.append((target_time, message))
            self.timers.sort(key=lambda x: x[0])
            
            # リストを更新
            self.update_timer_list()
            
            # 入力をクリア（00時00分に設定）
            self.hour_var.set("00")
            self.minute_var.set("00")
            self.message_var.set("")
            self.preset_var.set("")

#            messagebox.showinfo("成功", "タイマーを追加しました")
            
        except ValueError:
            messagebox.showerror("エラー", "時間は数値で入力してください")
    
    def update_timer_list(self):
        """タイマーリストを更新"""
        self.timer_listbox.delete(0, tk.END)
        for timer_time, message in sorted(self.timers, key=lambda x: x[0]):
            time_str = timer_time.strftime("%H:%M:%S")
            self.timer_listbox.insert(tk.END, f"{time_str} - {message}")
    
    def delete_selected_timer(self):
        """選択されたタイマーを削除"""
        selection = self.timer_listbox.curselection()
        if not selection:
            messagebox.showwarning("警告", "削除するタイマーを選択してください")
            return
        
        index = selection[0]
        sorted_timers = sorted(self.timers, key=lambda x: x[0])
        timer_to_delete = sorted_timers[index]
        self.timers.remove(timer_to_delete)
        
        # リストを更新
        self.update_timer_list()
        
        # 成功メッセージを表示しない
        # messagebox.showinfo("成功", "タイマーを削除しました")
    
    def clear_all_timers(self):
        """全タイマーをクリア"""
        if not self.timers:
            messagebox.showinfo("情報", "タイマーがありません")
            return
        
        # 確認ダイアログなしで直接削除
        self.timers.clear()
        self.update_timer_list()
        
        # 成功メッセージも表示しない
        # messagebox.showinfo("成功", "全てのタイマーを削除しました")
    
    def save_settings(self):
        """設定を保存して閉じる"""
        self.settings_window.destroy()
        self.settings_window = None
    
    def cancel_settings(self):
        """設定をキャンセルして閉じる"""
        self.settings_window.destroy()
        self.settings_window = None
    
    def on_closing(self):
        """ウィンドウを閉じる時の処理"""
        self.release_lock()
        self.root.destroy()
    
    def run(self):
        """アプリケーションを実行"""
        self.root.mainloop()


def main():
    if len(sys.argv) < 2:
        print("使用方法: python timer.py <設定ファイルパス>")
        print("例: python timer.py config/sample_config.json")
        sys.exit(1)
    
    config_path = sys.argv[1]
    
    if not os.path.exists(config_path):
        print(f"エラー: 設定ファイルが見つかりません: {config_path}")
        sys.exit(1)
    
    app = TimerApp(config_path)
    app.run()


if __name__ == "__main__":
    main()