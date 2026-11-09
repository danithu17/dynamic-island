#pragma once

#include <QObject>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QNetworkAccessManager>
#include <QTimer>
#include "GmailModel.h"

namespace Aura {

class GmailService : public QObject {
    Q_OBJECT
    Q_PROPERTY(GmailModel* model READ model CONSTANT)
    Q_PROPERTY(bool isAuthenticated READ isAuthenticated NOTIFY authenticatedChanged)
    Q_PROPERTY(bool isPolling READ isPolling NOTIFY pollingStateChanged)
    Q_PROPERTY(QString userEmail READ userEmail NOTIFY userEmailChanged)

public:
    static GmailService &instance();

    GmailModel *model() { return &m_model; }
    bool isAuthenticated() const { return m_isAuthenticated; }
    bool isPolling() const { return m_isPolling; }
    QString userEmail() const { return m_userEmail; }

    Q_INVOKABLE void startAuth();
    Q_INVOKABLE void disconnectAccount();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void openMessage(const QString &id);
    Q_INVOKABLE void markAsRead(const QString &id);
    Q_INVOKABLE void archiveMessage(const QString &id);
    Q_INVOKABLE void createDraft(const QString &subject, const QString &body, const QString &attachmentPath = QString());

    void setPollingPaused(bool paused);

signals:
    void authenticatedChanged();
    void pollingStateChanged();
    void userEmailChanged();

private slots:
    void onAuthenticated();
    void onAuthFailed(const QString &error, const QString &errorDescription, const QUrl &uri);
    void fetchUnreadMessages();

private:
    explicit GmailService(QObject *parent = nullptr);
    void loadCredentials();
    void tryRestoreSession();
    void fetchUserProfile();
    void fetchMessageDetails(const QStringList &ids);

    GmailModel m_model;
    QOAuth2AuthorizationCodeFlow *m_oauth = nullptr;
    QOAuthHttpServerReplyHandler *m_replyHandler = nullptr;
    QNetworkAccessManager *m_network = nullptr;
    QTimer *m_pollTimer = nullptr;

    QString m_clientId;
    QString m_clientSecret;
    QString m_userEmail;
    bool m_isAuthenticated = false;
    bool m_isPolling = false;
};

} // namespace Aura
