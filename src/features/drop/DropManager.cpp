#include "DropManager.h"
#include "../../core/AppState.h"
#include "../../core/Settings.h"
#include "../gmail/GmailService.h"
#include "../shelf/ShelfManager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDebug>

namespace Aura {

DropManager &DropManager::instance() {
    static DropManager s_instance;
    return s_instance;
}

DropManager::DropManager(QObject *parent)
    : QObject(parent) {
    connect(&m_shareServer, &LocalShareServer::runningChanged, this, &DropManager::qrActiveChanged);
    connect(&m_shareServer, &LocalShareServer::downloadCompleted, this, [this]() {
        AppState::instance().showToast("Downloaded to phone!", "success");
        emit qrActiveChanged();
        QTimer::singleShot(1500, []() {
            AppState::instance().collapseToIdle();
        });
    });
}

void DropManager::handleDropMail(const QString &data, const QString &type, const QString &name) {
    QString subject = name.isEmpty() ? "Shared via Aura Island" : name;
    QString body;
    QString attachmentPath;

    if (type == "file" || QFile::exists(data)) {
        attachmentPath = data;
        body = QString("Sharing file: %1").arg(QFileInfo(data).fileName());
    } else {
        body = data;
    }

    GmailService::instance().createDraft(subject, body, attachmentPath);
    m_lastActionMessage = QStringLiteral("Draft ready");
    emit lastActionMessageChanged();
    emit dropActionCompleted("mail", m_lastActionMessage);

    AppState::instance().showToast("Draft ready in Gmail", "success");
    QTimer::singleShot(1500, []() {
        AppState::instance().collapseToIdle();
    });
}

void DropManager::handleDropSave(const QString &data, const QString &type, const QString &name) {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString trayDir = appData + "/tray";
    QDir().mkpath(trayDir);

    QString action = "copy"; // default

    if (type == "file" || QFile::exists(data)) {
        QFileInfo fi(data);
        QString destPath = trayDir + "/" + fi.fileName();

        // Handle duplicates
        if (QFile::exists(destPath)) {
            QString base = fi.completeBaseName();
            QString ext = fi.suffix().isEmpty() ? "" : "." + fi.suffix();
            destPath = trayDir + "/" + base + "_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ext;
        }

        if (action == "move") {
            QFile::rename(data, destPath);
        } else if (action == "link") {
            QFile::link(data, destPath + ".lnk");
            destPath = destPath + ".lnk";
        } else {
            QFile::copy(data, destPath);
        }

        ShelfManager::instance().addItem(destPath, fi.fileName(), "file");
    } else {
        // Text or Link
        qint64 ts = QDateTime::currentMSecsSinceEpoch();
        QString fileName = (type == "url") ? QString("link_%1.url").arg(ts) : QString("snippet_%1.txt").arg(ts);
        QString destPath = trayDir + "/" + fileName;

        QFile f(destPath);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            if (type == "url") {
                f.write(QString("[InternetShortcut]\nURL=%1\n").arg(data).toUtf8());
            } else {
                f.write(data.toUtf8());
            }
            f.close();
        }

        ShelfManager::instance().addItem(destPath, name.isEmpty() ? fileName : name, type);
    }

    m_lastActionMessage = QStringLiteral("Saved");
    emit lastActionMessageChanged();
    emit dropActionCompleted("save", m_lastActionMessage);

    AppState::instance().showToast("Saved to Tray", "success");
    QTimer::singleShot(1500, []() {
        AppState::instance().collapseToIdle();
    });
}

void DropManager::handleDropPhone(const QString &data, const QString &type, const QString &name) {
    QString filePath;
    QString textData;
    QString displayName = name;

    if (type == "file" || QFile::exists(data)) {
        filePath = data;
        if (displayName.isEmpty()) displayName = QFileInfo(data).fileName();
    } else {
        textData = data;
        if (displayName.isEmpty()) displayName = (type == "url") ? "link.txt" : "shared-note.txt";
    }

    bool started = m_shareServer.startShare(filePath, textData, displayName);
    if (started) {
        m_lastActionMessage = QStringLiteral("Scan to download");
        emit lastActionMessageChanged();
        emit dropActionCompleted("phone", m_lastActionMessage);

        AppState::instance().showToast("Scan QR to download", "info");
        // Note: For QR code, per requirement: "QR stays until I dismiss it or it expires"
    } else {
        AppState::instance().showToast("Failed to start Wi-Fi share", "danger");
    }
}

void DropManager::dismissQr() {
    m_shareServer.stopShare();
    emit qrActiveChanged();
    AppState::instance().collapseToIdle();
}

} // namespace Aura
