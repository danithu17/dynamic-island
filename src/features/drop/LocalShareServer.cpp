#include "LocalShareServer.h"
#include "QrCode.hpp"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QUuid>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace Aura {

LocalShareServer::LocalShareServer(QObject *parent)
    : QObject(parent),
      m_server(new QTcpServer(this)),
      m_countdownTimer(new QTimer(this)) {
    connect(m_server, &QTcpServer::newConnection, this, &LocalShareServer::onNewConnection);
    connect(m_countdownTimer, &QTimer::timeout, this, &LocalShareServer::onTick);
    m_countdownTimer->setInterval(1000);
}

LocalShareServer::~LocalShareServer() {
    cleanup();
}

QString LocalShareServer::getLanIpAddress() const {
    const QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
            QString ip = address.toString();
            // Match standard LAN IPv4 subnets (192.168.x.x, 10.x.x.x, 172.16-31.x.x)
            if (ip.startsWith("192.168.") || ip.startsWith("10.") || ip.startsWith("172.")) {
                return ip;
            }
        }
    }
    // Fallback to non-loopback address if available
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
            return address.toString();
        }
    }
    return QStringLiteral("127.0.0.1");
}

bool LocalShareServer::startShare(const QString &filePath, const QString &textData, const QString &displayName) {
    cleanup();

    m_sourceFilePath = filePath;
    m_textPayload = textData.toUtf8();
    m_sharedName = displayName;
    if (m_sharedName.isEmpty()) {
        if (!m_sourceFilePath.isEmpty()) {
            m_sharedName = QFileInfo(m_sourceFilePath).fileName();
        } else {
            m_sharedName = QStringLiteral("shared-content.txt");
        }
    }

    // 1. Generate 128-bit random token
    m_token = QUuid::createUuid().toString(QUuid::WithoutBraces);

    // 2. Bind to local LAN IP on random available port
    QString lanIp = getLanIpAddress();
    if (!m_server->listen(QHostAddress(lanIp), 0)) {
        qWarning() << "Failed to start LocalShareServer on" << lanIp << ":" << m_server->errorString();
        return false;
    }

    quint16 port = m_server->serverPort();
    m_shareUrl = QString("http://%1:%2/dl/%3").arg(lanIp).arg(port).arg(m_token);
    qInfo() << "LocalShareServer listening at:" << m_shareUrl;

    // 3. Generate QR code SVG
    try {
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(
            m_shareUrl.toUtf8().constData(),
            qrcodegen::QrCode::Ecc::MEDIUM
        );
        std::string svg = qr.toSvgString(2);

        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m_qrSvgPath = tempDir + "/aura_share_qr.svg";
        QFile qrFile(m_qrSvgPath);
        if (qrFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qrFile.write(svg.data(), static_cast<qint64>(svg.size()));
            qrFile.close();
        }
    } catch (const std::exception &ex) {
        qWarning() << "QR code generation error:" << ex.what();
    }

    m_remainingSeconds = 300; // 5 minutes limit
    m_isRunning = true;
    m_countdownTimer->start();

    emit runningChanged();
    emit shareUrlChanged();
    emit qrSvgPathChanged();
    emit sharedNameChanged();
    emit remainingSecondsChanged();
    return true;
}

void LocalShareServer::stopShare() {
    cleanup();
}

void LocalShareServer::cleanup() {
    if (m_server && m_server->isListening()) {
        m_server->close();
    }
    m_countdownTimer->stop();
    m_token.clear();
    m_sourceFilePath.clear();
    m_textPayload.clear();

    if (m_isRunning) {
        m_isRunning = false;
        emit runningChanged();
    }
}

void LocalShareServer::onTick() {
    if (m_remainingSeconds > 0) {
        m_remainingSeconds--;
        emit remainingSecondsChanged();
    } else {
        qInfo() << "LocalShareServer 5-minute timeout reached. Stopping.";
        cleanup();
        emit expired();
    }
}

void LocalShareServer::onNewConnection() {
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;

    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        QByteArray request = socket->readAll();
        QString reqStr = QString::fromUtf8(request);
        QString firstLine = reqStr.section("\r\n", 0, 0);

        QString expectedPath = QString("/dl/%1").arg(m_token);
        if (firstLine.startsWith("GET ") && firstLine.contains(expectedPath)) {
            QByteArray content;
            if (!m_sourceFilePath.isEmpty() && QFile::exists(m_sourceFilePath)) {
                QFile f(m_sourceFilePath);
                if (f.open(QIODevice::ReadOnly)) {
                    content = f.readAll();
                }
            } else {
                content = m_textPayload;
            }

            QByteArray response;
            response.append("HTTP/1.1 200 OK\r\n");
            response.append("Content-Type: application/octet-stream\r\n");
            response.append("Content-Disposition: attachment; filename=\"" + m_sharedName.toUtf8() + "\"\r\n");
            response.append("Content-Length: " + QByteArray::number(content.size()) + "\r\n");
            response.append("Connection: close\r\n");
            response.append("Access-Control-Allow-Origin: *\r\n\r\n");
            response.append(content);

            socket->write(response);
            socket->flush();
            socket->disconnectFromHost();

            qInfo() << "LocalShareServer served single download for item:" << m_sharedName;
            emit downloadCompleted();

            // Stop server after one download as specified
            cleanup();
        } else {
            // No directory listing or invalid token: 404
            QByteArray notFound = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\nNot Found";
            socket->write(notFound);
            socket->disconnectFromHost();
        }
    });

    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
}

} // namespace Aura
