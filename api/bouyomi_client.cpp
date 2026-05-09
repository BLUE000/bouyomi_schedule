#include "bouyomi_client.h"
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDataStream>
#include <QDebug>

BouyomiClient::BouyomiClient(const QString& host, int port, int httpPort, QObject *parent)
    : QObject(parent), m_host(host), m_port(port), m_httpPort(httpPort)
{
    m_networkManager = new QNetworkAccessManager(this);
}

bool BouyomiClient::speak(const QString& message, int speed, int tone, int volume, int voice)
{
    // まずHTTP通信を試す
    if (speakHttp(message, speed, tone, volume, voice)) {
        return true;
    }
    
    // HTTPが失敗した場合はTCPを試す
    return speakTcp(message, speed, tone, volume, voice);
}

bool BouyomiClient::speakHttp(const QString& message, int speed, int tone, int volume, int voice)
{
    QUrl url(QString("http://%1:%2/talk").arg(m_host).arg(m_httpPort));
    QUrlQuery query;
    query.addQueryItem("text", message);
    if (voice != -1) query.addQueryItem("voice", QString::number(voice));
    if (volume != -1) query.addQueryItem("volume", QString::number(volume));
    if (speed != -1) query.addQueryItem("speed", QString::number(speed));
    if (tone != -1) query.addQueryItem("tone", QString::number(tone));
    
    url.setQuery(query);
    
    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, [this, reply, message]() {
        bool success = (reply->error() == QNetworkReply::NoError);
        emit speechFinished(success, message);
        reply->deleteLater();
    });
    
    return true; // 非同期のため、リクエスト開始時点でtrueを返す
}

bool BouyomiClient::speakTcp(const QString& message, int speed, int tone, int volume, int voice)
{
    QTcpSocket* socket = new QTcpSocket(this);
    socket->connectToHost(m_host, m_port);
    
    if (!socket->waitForConnected(3000)) {
        qDebug() << "TCP Connection failed:" << socket->errorString();
        socket->deleteLater();
        return false;
    }
    
    QByteArray messageBytes = message.toUtf8();
    QByteArray packet;
    QDataStream stream(&packet, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // パケット構造を構築 (struct.pack('<HhhhhBi', ...))
    stream << (quint16)0x0001;      // Command
    stream << (qint16)speed;        // Speed
    stream << (qint16)tone;         // Tone
    stream << (qint16)volume;       // Volume
    stream << (qint16)voice;        // Voice
    stream << (quint8)0;            // Encoding (UTF-8)
    stream << (qint32)messageBytes.length();
    packet.append(messageBytes);
    
    socket->write(packet);
    socket->waitForBytesWritten(3000);
    socket->disconnectFromHost();
    
    connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    
    return true;
}
