#include "GmailService.h"
#include "CredentialStore.h"
#include "../../core/AppState.h"
#include "../../core/Settings.h"
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QDebug>

namespace Aura {

static const char* GMAIL_REFRESH_TOKEN_KEY = "AuraIsland_Gmail_RefreshToken";

GmailService &GmailService::instance() {
    static GmailService s_instance;
    return s_instance;
}

GmailService::GmailService(QObject *parent)
    : QObject(parent),
      m_oauth(new QOAuth2AuthorizationCodeFlow(this)),
      m_network(new QNetworkAccessManager(this)),
      m_pollTimer(new QTimer(this)) {

    loadCredentials();

    m_oauth->setAuthorizationUrl(QUrl("https://accounts.google.com/o/oauth2/v2/auth"));
    m_oauth->setAccessTokenUrl(QUrl("https://oauth2.googleapis.com/token"));
    m_oauth->setScope("https://www.googleapis.com/auth/gmail.readonly https://www.googleapis.com/auth/gmail.modify https://www.googleapis.com/auth/gmail.compose");

    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::granted, this, &GmailService::onAuthenticated);
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::error, this, &GmailService::onAuthFailed);
    connect(m_oauth, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });

    connect(m_pollTimer, &QTimer::timeout, this, &GmailService::fetchUnreadMessages);

    tryRestoreSession();
}

void GmailService::loadCredentials() {
    // 1. Try environment variables
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains("GMAIL_CLIENT_ID") && env.contains("GMAIL_CLIENT_SECRET")) {
        m_clientId = env.value("GMAIL_CLIENT_ID");
        m_clientSecret = env.value("GMAIL_CLIENT_SECRET");
        m_oauth->setClientIdentifier(m_clientId);
        m_oauth->setClientIdentifierSharedKey(m_clientSecret);
        return;
    }

    // 2. Try %APPDATA%/AuraIsland/gmail_credentials.json
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QFile credFile(appData + "/gmail_credentials.json");
    if (!credFile.exists()) {
        credFile.setFileName("gmail_credentials.json");
    }

    if (credFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(credFile.readAll());
        QJsonObject root = doc.object();
        QJsonObject installed = root.contains("installed") ? root["installed"].toObject() : root;

        m_clientId = installed.value("client_id").toString();
        m_clientSecret = installed.value("client_secret").toString();
        m_oauth->setClientIdentifier(m_clientId);
        m_oauth->setClientIdentifierSharedKey(m_clientSecret);
    }
}

void GmailService::tryRestoreSession() {
    QString storedToken = CredentialStore::readSecret(GMAIL_REFRESH_TOKEN_KEY);
    if (!storedToken.isEmpty() && !m_clientId.isEmpty()) {
        m_oauth->setRefreshToken(storedToken);
        m_oauth->refreshAccessToken();
        m_isAuthenticated = true;
        emit authenticatedChanged();

        fetchUserProfile();
        fetchUnreadMessages();
        m_pollTimer->start(Settings::instance().pollIntervalMinutes() * 60 * 1000);
    }
}

void GmailService::startAuth() {
    if (m_clientId.isEmpty() || m_clientSecret.isEmpty()) {
        loadCredentials();
        if (m_clientId.isEmpty()) {
            AppState::instance().showToast("Gmail: Missing Client ID/Secret. Check README.", "warning");
            return;
        }
    }

    if (!m_replyHandler) {
        m_replyHandler = new QOAuthHttpServerReplyHandler(19826, this);
        m_oauth->setReplyHandler(m_replyHandler);
    }

    m_oauth->grant();
}

void GmailService::disconnectAccount() {
    m_pollTimer->stop();
    m_isAuthenticated = false;
    m_userEmail.clear();
    CredentialStore::deleteSecret(GMAIL_REFRESH_TOKEN_KEY);
    m_model.setMessages({});
    AppState::instance().setUnreadMailCount(0);
    emit authenticatedChanged();
    emit userEmailChanged();
    AppState::instance().showToast("Gmail disconnected", "info");
}

void GmailService::onAuthenticated() {
    m_isAuthenticated = true;
    emit authenticatedChanged();

    QString refreshToken = m_oauth->refreshToken();
    if (!refreshToken.isEmpty()) {
        CredentialStore::storeSecret(GMAIL_REFRESH_TOKEN_KEY, refreshToken);
    }

    AppState::instance().showToast("Gmail connected successfully", "success");
    fetchUserProfile();
    fetchUnreadMessages();
    m_pollTimer->start(Settings::instance().pollIntervalMinutes() * 60 * 1000);
}

void GmailService::onAuthFailed(const QString &error, const QString &errorDescription, const QUrl &uri) {
    Q_UNUSED(uri);
    qWarning() << "Gmail OAuth error:" << error << "-" << errorDescription;
    AppState::instance().showToast("Gmail auth failed: " + error, "error");
}

void GmailService::setPollingPaused(bool paused) {
    if (paused) {
        m_pollTimer->stop();
    } else if (m_isAuthenticated && !m_pollTimer->isActive()) {
        m_pollTimer->start(Settings::instance().pollIntervalMinutes() * 60 * 1000);
        fetchUnreadMessages();
    }
}

void GmailService::refresh() {
    if (m_isAuthenticated) {
        fetchUnreadMessages();
    }
}

void GmailService::fetchUserProfile() {
    if (!m_isAuthenticated) return;

    QUrl url("https://gmail.googleapis.com/gmail/v1/users/me/profile");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + m_oauth->token().toUtf8());

    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            m_userEmail = doc.object().value("emailAddress").toString();
            emit userEmailChanged();
        }
    });
}

void GmailService::fetchUnreadMessages() {
    if (!m_isAuthenticated) return;

    // Fetch up to 5 unread messages
    QUrl url("https://gmail.googleapis.com/gmail/v1/users/me/messages?maxResults=5&q=is:unread");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + m_oauth->token().toUtf8());

    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            int estimatedResultSize = obj.value("resultSizeEstimate").toInt();
            AppState::instance().setUnreadMailCount(estimatedResultSize);

            QJsonArray messagesArr = obj.value("messages").toArray();
            QStringList ids;
            for (const auto &item : messagesArr) {
                ids << item.toObject().value("id").toString();
            }
            if (!ids.isEmpty()) {
                fetchMessageDetails(ids);
            } else {
                m_model.setMessages({});
            }
        }
    });
}

void GmailService::fetchMessageDetails(const QStringList &ids) {
    auto messageList = std::make_shared<std::vector<GmailMessage>>();
    auto remaining = std::make_shared<int>(ids.size());

    for (const QString &id : ids) {
        QUrl url("https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "?format=metadata&metadataHeaders=From&metadataHeaders=Subject&metadataHeaders=Date");
        QNetworkRequest request(url);
        request.setRawHeader("Authorization", "Bearer " + m_oauth->token().toUtf8());

        QNetworkReply *reply = m_network->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, id, messageList, remaining]() {
            reply->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
                QJsonObject root = doc.object();

                GmailMessage msg;
                msg.id = id;
                msg.threadId = root.value("threadId").toString();
                msg.snippet = root.value("snippet").toString();
                msg.isUnread = true;

                QJsonArray headers = root.value("payload").toObject().value("headers").toArray();
                for (const auto &hVal : headers) {
                    QJsonObject hObj = hVal.toObject();
                    QString name = hObj.value("name").toString().toLower();
                    QString val = hObj.value("value").toString();
                    if (name == "from") msg.sender = val;
                    else if (name == "subject") msg.subject = val;
                    else if (name == "date") msg.date = val;
                }

                messageList->push_back(msg);
            }

            (*remaining)--;
            if (*remaining == 0) {
                m_model.setMessages(*messageList);
            }
        });
    }
}

void GmailService::openMessage(const QString &id) {
    QUrl url("https://mail.google.com/mail/u/0/#inbox/" + id);
    QDesktopServices::openUrl(url);
}

void GmailService::markAsRead(const QString &id) {
    if (!m_isAuthenticated) return;

    QUrl url("https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "/modify");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + m_oauth->token().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    QJsonArray removeLabels;
    removeLabels.append("UNREAD");
    body["removeLabelIds"] = removeLabels;

    QNetworkReply *reply = m_network->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, id]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            m_model.removeMessageById(id);
            int current = AppState::instance().unreadMailCount();
            if (current > 0) AppState::instance().setUnreadMailCount(current - 1);
            AppState::instance().showToast("Marked as read", "info");
        }
    });
}

void GmailService::archiveMessage(const QString &id) {
    if (!m_isAuthenticated) return;

    QUrl url("https://gmail.googleapis.com/gmail/v1/users/me/messages/" + id + "/modify");
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + m_oauth->token().toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    QJsonArray removeLabels;
    removeLabels.append("INBOX");
    body["removeLabelIds"] = removeLabels;

    QNetworkReply *reply = m_network->post(request, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, id]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            m_model.removeMessageById(id);
            int current = AppState::instance().unreadMailCount();
            if (current > 0) AppState::instance().setUnreadMailCount(current - 1);
            AppState::instance().showToast("Email archived", "info");
        }
    });
}

void GmailService::createDraft(const QString &subject, const QString &body, const QString &attachmentPath) {
    if (!m_isAuthenticated) {
        // Fallback: Open web compose in default browser
        QUrl url("https://mail.google.com/mail/?view=cm&fs=1");
        QUrlQuery query;
        if (!subject.isEmpty()) query.addQueryItem("su", subject);
        if (!body.isEmpty()) query.addQueryItem("body", body);
        url.setQuery(query);
        QDesktopServices::openUrl(url);
        return;
    }

    // Compose RFC 2822 email message
    QString rawEmail;
    if (attachmentPath.isEmpty() || !QFile::exists(attachmentPath)) {
        rawEmail = QString("Subject: %1\r\nContent-Type: text/plain; charset=\"UTF-8\"\r\n\r\n%2")
                       .arg(subject.isEmpty() ? "Shared via Aura Island" : subject)
                       .arg(body);
    } else {
        QFile attachFile(attachmentPath);
        QByteArray fileBytes;
        if (attachFile.open(QIODevice::ReadOnly)) {
            fileBytes = attachFile.readAll();
        }
        QString fileName = QFileInfo(attachmentPath).fileName();
        QString boundary = "===AuraIslandBoundary===";
        rawEmail = QString("Subject: %1\r\nMIME-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=\"%2\"\r\n\r\n"
                           "--%2\r\nContent-Type: text/plain; charset=\"UTF-8\"\r\n\r\n%3\r\n\r\n"
                           "--%2\r\nContent-Type: application/octet-stream; name=\"%4\"\r\n"
                           "Content-Disposition: attachment; filename=\"%4\"\r\n"
                           "Content-Transfer-Encoding: base64\r\n\r\n%5\r\n\r\n--%2--")
                       .arg(subject.isEmpty() ? ("Attached: " + fileName) : subject)
                       .arg(boundary)
                       .arg(body)
                       .arg(fileName)
                       .arg(QString::fromLatin1(fileBytes.toBase64()));
    }

    // URL-safe base64 encode
    QByteArray base64Url = rawEmail.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QJsonObject messageObj;
    messageObj["raw"] = QString::fromLatin1(base64Url);

    QJsonObject draftObj;
    draftObj["message"] = messageObj;

    QNetworkRequest request(QUrl("https://gmail.googleapis.com/gmail/v1/users/me/drafts"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_oauth->token().toUtf8());

    QNetworkReply *reply = m_network->post(request, QJsonDocument(draftObj).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            qInfo() << "Draft successfully created in Gmail.";
            QDesktopServices::openUrl(QUrl("https://mail.google.com/mail/u/0/#drafts"));
        } else {
            qWarning() << "Failed to create draft in Gmail API:" << reply->errorString();
            QDesktopServices::openUrl(QUrl("https://mail.google.com/mail/u/0/#drafts"));
        }
    });
}

} // namespace Aura
