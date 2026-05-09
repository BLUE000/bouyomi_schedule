#include <QCoreApplication>
#include "api/bouyomi_client.h"
#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    BouyomiClient client("localhost", 50080, 50080);
    
    qDebug() << "Sending test message to Bouyomi-chan...";
    
    // HTTP/TCP両方を試すため、speakメソッドを使用
    QString message = "C++版の、BouyomiSchedule、実機テスト送信です。正しく聞こえていますか？";
    
    if (client.speak(message)) {
        qDebug() << "Send request successful. Waiting for a moment...";
    } else {
        qDebug() << "Send request failed.";
        return 1;
    }

    // 通信が非同期なので少し待ってから終了
    QTimer::singleShot(3000, &a, &QCoreApplication::quit);
    
    return a.exec();
}
