#include "AgentHookServer.h"
#include "../../core/AppState.h"
#include "../../core/Settings.h"
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <windows.h>
#include <QDebug>

namespace Aura {

AgentHookServer::AgentHookServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)) {
    connect(m_server, &QTcpServer::newConnection, this, &AgentHookServer::onNewConnection);
}

AgentHookServer::~AgentHookServer() {
    stopServer();
}

bool AgentHookServer::startServer(quint16 port, const QString &token) {
    stopServer();
    m_port = port;
    m_token = token.isEmpty() ? Settings::instance().agentServerToken() : token;

    // Strictly bind to 127.0.0.1 for local security
    if (!m_server->listen(QHostAddress::LocalHost, m_port)) {
        qWarning() << "AgentHookServer failed to listen on port" << m_port << ":" << m_server->errorString();
        return false;
    }

    emit serverStateChanged();
    return true;
}

void AgentHookServer::stopServer() {
    if (m_server->isListening()) {
        m_server->close();
        emit serverStateChanged();
    }
}

bool AgentHookServer::isListening() const {
    return m_server && m_server->isListening();
}

void AgentHookServer::onNewConnection() {
    while (m_server->hasPendingConnections()) {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            handleSocketData(socket);
        });
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void AgentHookServer::handleSocketData(QTcpSocket *socket) {
    QByteArray rawData = socket->readAll();
    QString req = QString::fromUtf8(rawData);
    QStringList lines = req.split("\r\n");
    if (lines.isEmpty()) return;

    QString requestLine = lines[0];
    QStringList parts = requestLine.split(" ");
    if (parts.size() < 2) return;

    QString method = parts[0];
    QString path = parts[1];

    // Check authorization header
    bool authorized = m_token.isEmpty();
    for (const QString &line : lines) {
        if (line.startsWith("Authorization: Bearer ", Qt::CaseInsensitive)) {
            QString suppliedToken = line.mid(22).trimmed();
            if (suppliedToken == m_token) {
                authorized = true;
            }
        }
    }

    if (!authorized) {
        sendResponse(socket, 401, "{\"error\": \"Unauthorized - Invalid Token\"}");
        socket->disconnectFromHost();
        return;
    }

    if (method == "POST" && path.startsWith("/api/hook")) {
        // Find body after double newline
        int bodyIdx = req.indexOf("\r\n\r\n");
        QByteArray body = (bodyIdx != -1) ? rawData.mid(bodyIdx + 4) : QByteArray();

        QJsonDocument doc = QJsonDocument::fromJson(body);
        QJsonObject obj = doc.object();

        m_currentAgent = obj.value("agent").toString("Claude Code");
        m_currentStatus = obj.value("status").toString("working");
        m_currentMessage = obj.value("message").toString();
        m_pendingAction = obj.value("action").toString();

        AppState::instance().setAgentStatus(m_currentStatus);
        AppState::instance().setAgentMessage(m_currentMessage);

        if (m_currentStatus == "done") {
            MessageBeep(MB_ICONASTERISK); // soft chime on done
            AppState::instance().showToast("Agent Task Completed: " + m_currentMessage, "success");
            m_isApprovalPending = false;
            sendResponse(socket, 200, "{\"ok\": true, \"status\": \"done\"}");
            socket->disconnectFromHost();
        } else if (m_currentStatus == "needs_approval") {
            m_isApprovalPending = true;
            m_pendingApprovalSocket = socket;
            AppState::instance().expandTab("dev");
            AppState::instance().showToast("Agent requires approval for action!", "warning");
            // Do not disconnect yet: hold connection until user allows or denies
        } else {
            m_isApprovalPending = false;
            sendResponse(socket, 200, "{\"ok\": true, \"status\": \"" + m_currentStatus + "\"}");
            socket->disconnectFromHost();
        }

        emit hookEventReceived();
    } else {
        sendResponse(socket, 200, "{\"service\": \"Aura Island Agent Hook\", \"status\": \"online\"}");
        socket->disconnectFromHost();
    }
}

void AgentHookServer::allowAction() {
    if (m_isApprovalPending) {
        m_isApprovalPending = false;
        if (m_pendingApprovalSocket) {
            sendResponse(m_pendingApprovalSocket, 200, "{\"decision\": \"allow\", \"allowed\": true}");
            m_pendingApprovalSocket->disconnectFromHost();
        }
        AppState::instance().setAgentStatus("working");
        AppState::instance().showToast("Action approved", "success");
        emit hookEventReceived();
    }
}

void AgentHookServer::denyAction() {
    if (m_isApprovalPending) {
        m_isApprovalPending = false;
        if (m_pendingApprovalSocket) {
            sendResponse(m_pendingApprovalSocket, 200, "{\"decision\": \"deny\", \"allowed\": false}");
            m_pendingApprovalSocket->disconnectFromHost();
        }
        AppState::instance().setAgentStatus("idle");
        AppState::instance().showToast("Action denied", "info");
        emit hookEventReceived();
    }
}

void AgentHookServer::sendResponse(QTcpSocket *socket, int statusCode, const QString &body, const QString &contentType) {
    if (!socket || !socket->isOpen()) return;

    QString statusText = (statusCode == 200) ? "OK" : (statusCode == 401 ? "Unauthorized" : "Not Found");
    QByteArray bodyUtf8 = body.toUtf8();

    QString header = QString("HTTP/1.1 %1 %2\r\n"
                             "Content-Type: %3\r\n"
                             "Content-Length: %4\r\n"
                             "Connection: close\r\n\r\n")
                         .arg(statusCode)
                         .arg(statusText)
                         .arg(contentType)
                         .arg(bodyUtf8.size());

    socket->write(header.toUtf8());
    socket->write(bodyUtf8);
    socket->flush();
}

} // namespace Aura
