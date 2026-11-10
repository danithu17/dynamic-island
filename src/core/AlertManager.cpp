#include "AlertManager.h"
#include "AppState.h"
#include "Settings.h"
#include "../features/shelf/ShelfManager.h"
#include "../features/clipboard/ClipboardManager.h"
#include "../features/dev/DevManager.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QClipboard>
#include <QImage>
#include <QGuiApplication>
#include <QMimeData>
#include <QDebug>

namespace Aura {

AlertManager &AlertManager::instance() {
    static AlertManager s_instance;
    return s_instance;
}

AlertManager::AlertManager(QObject *parent)
    : QObject(parent)
    , m_dismissTimer(new QTimer(this))
    , m_folderWatcher(new QFileSystemWatcher(this)) {
    m_dismissTimer->setSingleShot(true);
    connect(m_dismissTimer, &QTimer::timeout, this, &AlertManager::onAlertTimeout);

    initFolderWatchers();
}

void AlertManager::initFolderWatchers() {
    // 1. Pictures\Screenshots
    QString picturesPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    m_screenshotsPath = picturesPath + "/Screenshots";
    QDir ssDir(m_screenshotsPath);
    if (ssDir.exists()) {
        m_folderWatcher->addPath(m_screenshotsPath);
        m_knownScreenshots = ssDir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files);
    }
    connect(m_folderWatcher, &QFileSystemWatcher::directoryChanged,
            this, &AlertManager::onScreenshotsDirChanged);

    // 2. Downloads folder
    m_downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir dlDir(m_downloadsPath);
    if (dlDir.exists()) {
        m_folderWatcher->addPath(m_downloadsPath);
        m_knownDownloads = dlDir.entryList(QDir::Files);
    }
    connect(m_folderWatcher, &QFileSystemWatcher::directoryChanged,
            this, &AlertManager::onDownloadsDirChanged);
}

void AlertManager::postAlert(const QString &type,
                             const QString &title,
                             const QString &subtitle,
                             const QString &icon,
                             const QString &iconColor,
                             const QString &action1Text,
                             const QString &action1Id,
                             const QString &action2Text,
                             const QString &action2Id,
                             int durationMs,
                             bool isSticky,
                             const QString &targetTab,
                             const QString &filePath) {
    // Do Not Disturb filter (Focus mode active): only allow sticky alerts and timers
    if (AppState::instance().isFocusModeActive() && !isSticky && type != "timer") {
        return;
    }

    AlertItem item;
    item.id = type + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    item.type = type;
    item.title = title;
    item.subtitle = subtitle;
    item.icon = icon;
    item.iconColor = iconColor.isEmpty() ? "#0A84FF" : iconColor;
    item.action1Text = action1Text;
    item.action1Id = action1Id;
    item.action2Text = action2Text;
    item.action2Id = action2Id;
    item.durationMs = durationMs;
    item.isSticky = isSticky;
    item.targetTab = targetTab;
    item.filePath = filePath;

    // Priority mapping
    if (isSticky || type == "agent") item.priority = 100;
    else if (type == "timer") item.priority = 90;
    else if (type == "battery") item.priority = 80;
    else if (type == "screenshot") item.priority = 70;
    else if (type == "download") item.priority = 60;
    else if (type == "gmail") item.priority = 50;
    else item.priority = 40;

    // Merge same-type duplicates: remove older pending alert of same type
    for (int i = 0; i < m_queue.size(); ++i) {
        if (m_queue[i].type == type) {
            m_queue.removeAt(i);
            break;
        }
    }

    // Insert sorted by priority
    int insertIdx = 0;
    while (insertIdx < m_queue.size() && m_queue[insertIdx].priority >= item.priority) {
        insertIdx++;
    }
    m_queue.insert(insertIdx, item);

    if (!m_hasActiveAlert) {
        showNextAlert();
    }
}

void AlertManager::showNextAlert() {
    if (m_queue.isEmpty()) {
        m_hasActiveAlert = false;
        AppState::instance().setStickyAlertShowing(false);
        emit alertChanged();

        // Collapse to idle if island was opened just for the alert
        if (AppState::instance().islandState() == "alert") {
            AppState::instance().collapseToIdle();
        }
        return;
    }

    m_currentAlert = m_queue.takeFirst();
    m_hasActiveAlert = true;
    AppState::instance().setStickyAlertShowing(m_currentAlert.isSticky);
    emit alertChanged();

    // Shape changes to Alert size (360x76, radius 28)
    if (AppState::instance().islandState() != "expanded") {
        AppState::instance().setIslandState("alert");
    }

    if (!m_currentAlert.isSticky && m_currentAlert.durationMs > 0) {
        m_remainingMs = m_currentAlert.durationMs;
        m_dismissTimer->start(m_remainingMs);
    } else {
        m_dismissTimer->stop();
    }
}

void AlertManager::dismissCurrentAlert() {
    m_dismissTimer->stop();
    showNextAlert();
}

void AlertManager::pauseTimer() {
    if (m_dismissTimer->isActive()) {
        m_remainingMs = m_dismissTimer->remainingTime();
        m_dismissTimer->stop();
    }
}

void AlertManager::resumeTimer() {
    if (!m_currentAlert.isSticky && m_remainingMs > 0) {
        m_dismissTimer->start(m_remainingMs);
    }
}

void AlertManager::triggerAction(const QString &actionId) {
    if (actionId == "screenshot_copy") {
        if (!m_currentAlert.filePath.isEmpty()) {
            QClipboard *cb = QGuiApplication::clipboard();
            QImage img(m_currentAlert.filePath);
            if (!img.isNull()) {
                cb->setImage(img);
                AppState::instance().showToast("Screenshot copied to clipboard", "success");
            }
        }
    } else if (actionId == "screenshot_tray") {
        if (!m_currentAlert.filePath.isEmpty()) {
            ShelfManager::instance().addFile(m_currentAlert.filePath);
            AppState::instance().showToast("Screenshot saved to Tray", "success");
        }
    } else if (actionId == "download_open") {
        if (!m_currentAlert.filePath.isEmpty()) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_currentAlert.filePath));
        }
    } else if (actionId == "download_folder") {
        if (!m_currentAlert.filePath.isEmpty()) {
            QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(m_currentAlert.filePath));
        }
    } else if (actionId == "timer_break") {
        DevManager::instance().pomodoro()->skipToNext();
    } else if (actionId == "timer_again") {
        DevManager::instance().pomodoro()->start();
    } else if (actionId == "agent_allow") {
        AppState::instance().approveAgentAction();
    } else if (actionId == "agent_deny") {
        AppState::instance().rejectAgentAction();
    } else if (actionId == "gmail_open") {
        AppState::instance().expandTab("mail");
    }

    dismissCurrentAlert();
}

void AlertManager::onAlertTimeout() {
    showNextAlert();
}

void AlertManager::onScreenshotsDirChanged(const QString &path) {
    Q_UNUSED(path);
    QDir dir(m_screenshotsPath);
    if (!dir.exists()) return;

    QStringList current = dir.entryList(QStringList() << "*.png" << "*.jpg" << "*.jpeg", QDir::Files, QDir::Time);
    for (const QString &file : current) {
        if (!m_knownScreenshots.contains(file)) {
            m_knownScreenshots.append(file);
            QString fullPath = dir.absoluteFilePath(file);
            postAlert("screenshot",
                      "New Screenshot",
                      file,
                      "qrc:/resources/icons/copy.svg",
                      "#0A84FF",
                      "Copy", "screenshot_copy",
                      "To Tray", "screenshot_tray",
                      5000,
                      false,
                      "shelf",
                      fullPath);
            break;
        }
    }
}

void AlertManager::onDownloadsDirChanged(const QString &path) {
    Q_UNUSED(path);
    QDir dir(m_downloadsPath);
    if (!dir.exists()) return;

    QStringList current = dir.entryList(QDir::Files, QDir::Time);
    for (const QString &file : current) {
        // Skip browser download temp files
        if (file.endsWith(".crdownload") || file.endsWith(".tmp") || file.endsWith(".part")) {
            continue;
        }
        if (!m_knownDownloads.contains(file)) {
            m_knownDownloads.append(file);
            QString fullPath = dir.absoluteFilePath(file);
            QFileInfo fi(fullPath);
            QString sizeStr = QString::number(fi.size() / 1024) + " KB";
            postAlert("download",
                      "Download Complete",
                      file + " (" + sizeStr + ")",
                      "qrc:/resources/icons/drop.svg",
                      "#30D158",
                      "Open", "download_open",
                      "Show in folder", "download_folder",
                      5000,
                      false,
                      "",
                      fullPath);
            break;
        }
    }
}

} // namespace Aura
