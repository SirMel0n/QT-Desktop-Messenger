#include "TcpSocket.h"
#include <QDebug>

TcpSocket::TcpSocket(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected,    this, &TcpSocket::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpSocket::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,    this, &TcpSocket::onReadyRead);
    connect(m_socket, &QAbstractSocket::errorOccurred, this, &TcpSocket::onErrorOccurred);
}

TcpSocket::~TcpSocket()
{
    disconnectFromServer();
}

void TcpSocket::connectToServer(const QString &host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

void TcpSocket::disconnectFromServer()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState)
        m_socket->disconnectFromHost();
}

void TcpSocket::sendMessage(const QString &message)
{
    if (!isConnected()) return;
    QByteArray data = message.toUtf8();  // newline as message delimiter
    m_socket->write(data);
}

bool TcpSocket::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpSocket::onConnected()
{
    qDebug() << "Connected to server.";
    emit connected();
}

void TcpSocket::onDisconnected()
{
    qDebug() << "Disconnected from server.";
    emit disconnected();
}

void TcpSocket::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString message = QString::fromUtf8(m_socket->readLine()).trimmed();
        emit messageReceived(message);
    }
}

void TcpSocket::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}
