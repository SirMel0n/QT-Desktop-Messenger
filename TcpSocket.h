#pragma once

#include <QObject>
#include <QTcpSocket>

class TcpSocket : public QObject
{
    Q_OBJECT

public:
    explicit TcpSocket(QObject *parent = nullptr);
    ~TcpSocket();

    void connectToServer(const QString &host, quint16 port);
    void disconnectFromServer();
    void sendMessage(const QString &message);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &message);
    void errorOccurred(const QString &errorMsg);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
};
