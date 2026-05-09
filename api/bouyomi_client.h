#ifndef BOUYOMI_CLIENT_H
#define BOUYOMI_CLIENT_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QTcpSocket>

class BouyomiClient : public QObject
{
    Q_OBJECT
public:
    explicit BouyomiClient(const QString& host = "localhost", 
                          int port = 50080, 
                          int httpPort = 50080, 
                          QObject *parent = nullptr);

    /**
     * @brief 棒読みちゃんに読み上げを依頼
     * @return 送信処理を開始した場合はtrue
     */
    bool speak(const QString& message, int speed = -1, int tone = -1, 
               int volume = -1, int voice = 0);

signals:
    void speechFinished(bool success, const QString& message);

private:
    bool speakHttp(const QString& message, int speed, int tone, int volume, int voice);
    bool speakTcp(const QString& message, int speed, int tone, int volume, int voice);

    QString m_host;
    int m_port;
    int m_httpPort;
    QNetworkAccessManager* m_networkManager;
};

#endif // BOUYOMI_CLIENT_H
