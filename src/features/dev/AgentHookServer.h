#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>
#include <QPointer>

namespace Aura {

class AgentHookServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isListening READ isListening NOTIFY serverStateChanged)
    Q_PROPERTY(int port READ port NOTIFY serverStateChanged)
    Q_PROPERTY(QString token READ token NOTIFY serverStateChanged)
    Q_PROPERTY(QString currentAgent READ currentAgent NOTIFY hookEventReceived)
    Q_PROPERTY(QString currentStatus READ currentStatus NOTIFY hookEventReceived)
    Q_PROPERTY(QString currentMessage READ currentMessage NOTIFY hookEventReceived)
    Q_PROPERTY(QString pendingAction READ pendingAction NOTIFY hookEventReceived)
    Q_PROPERTY(bool isApprovalPending READ isApprovalPending NOTIFY hookEventReceived)

public:
    explicit AgentHookServer(QObject *parent = nullptr);
    ~AgentHookServer() override;

    bool startServer(quint16 port = 19827, const QString &token = "");
    void stopServer();

    bool isListening() const;
    int port() const { return m_port; }
    QString token() const { return m_token; }

    QString currentAgent() const { return m_currentAgent; }
    QString currentStatus() const { return m_currentStatus; }
    QString currentMessage() const { return m_currentMessage; }
    QString pendingAction() const { return m_pendingAction; }
    bool isApprovalPending() const { return m_isApprovalPending; }

    Q_INVOKABLE void allowAction();
    Q_INVOKABLE void denyAction();

signals:
    void serverStateChanged();
    void hookEventReceived();
    void approvalDecision(bool allowed);

private slots:
    void onNewConnection();
    void handleSocketData(QTcpSocket *socket);

private:
    void sendResponse(QTcpSocket *socket, int statusCode, const QString &body, const QString &contentType = "application/json");

    QTcpServer *m_server = nullptr;
    quint16 m_port = 19827;
    QString m_token;

    QString m_currentAgent = "None";
    QString m_currentStatus = "idle";
    QString m_currentMessage;
    QString m_pendingAction;
    bool m_isApprovalPending = false;
    QPointer<QTcpSocket> m_pendingApprovalSocket;
};

} // namespace Aura
