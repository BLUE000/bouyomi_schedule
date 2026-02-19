import socket
import struct
import urllib.request
import urllib.parse

class BouyomiClient:
    """棒読みちゃんと通信するクライアント"""
    
    def __init__(self, host='localhost', port=50080, http_port=50080):
        self.host = host
        self.port = port
        self.http_port = http_port
    
    def speak(self, message, speed=-1, tone=-1, volume=-1, voice=0):
        """
        棒読みちゃんに読み上げを依頼
        
        Args:
            message: 読み上げるメッセージ
            speed: 速度 (-1: デフォルト)
            tone: 音程 (-1: デフォルト)
            volume: 音量 (-1: デフォルト)
            voice: 声質 (0: デフォルト)
        """
        try:
            # まずHTTP通信を試す
            if self.speak_http(message, speed, tone, volume, voice):
                return True
            
            # HTTP通信が失敗したらTCP/IP通信を試す
            return self.speak_tcp(message, speed, tone, volume, voice)
        except Exception as e:
            print(f"棒読みちゃんへの送信に失敗: {e}")
            return False
    
    def speak_http(self, message, speed=-1, tone=-1, volume=-1, voice=0):
        """HTTP通信で棒読みちゃんに送信"""
        try:
            # URLパラメータを構築
            params = {
                'text': message,
                'voice': voice,
                'volume': volume,
                'speed': speed,
                'tone': tone
            }
            
            # 不要なパラメータを削除
            params = {k: v for k, v in params.items() if v != -1}
            
            # URLを構築
            query = urllib.parse.urlencode(params)
            url = f"http://{self.host}:{self.http_port}/talk?{query}"
            
            print(f"HTTP通信で送信: {url}")
            
            # HTTP通信
            with urllib.request.urlopen(url, timeout=5) as response:
                print(f"HTTP応答: {response.status}")
            
            return True
        except Exception as e:
            print(f"HTTP通信での送信に失敗: {e}")
            return False
    
    def speak_tcp(self, message, speed=-1, tone=-1, volume=-1, voice=0):
        """TCP/IP通信で棒読みちゃんに送信"""
        try:
            # メッセージをUTF-8でエンコード
            message_bytes = message.encode('utf-8')
            
            # パケットを構築
            packet = struct.pack(
                '<HhhhhBi',
                0x0001,  # Command
                speed,
                tone,
                volume,
                voice,
                0,  # Encoding (UTF-8)
                len(message_bytes)
            ) + message_bytes
            
            # ソケット通信
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(5)
                print(f"TCP/IP通信で送信: {self.host}:{self.port}")
                sock.connect((self.host, self.port))
                print(f"送信メッセージ: {message}")
                sock.sendall(packet)
                print("送信完了")
            
            return True
        except Exception as e:
            print(f"TCP/IP通信での送信に失敗: {e}")
            return False
