#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QVariant>

namespace Aura {

struct AlertItem {
    QString id;
    QString type;
    int priority = 0;
    QString icon;
    QString iconColor;
    QString title;
    QString subtitle;
    QString action1Text;
    QString action1Id;
    QString action2Text;
    QString action2Id;
    int durationMs = 4000;
    bool isSticky = false;
    QString targetTab;
    QString filePath;
};

class AlertManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool hasActiveAlert READ hasActiveAlert NOTIFY alertChanged)
    Q_PROPERTY(QString currentType READ currentType NOTIFY alertChanged)
    Q_PROPERTY(QString currentIcon READ currentIcon NOTIFY alertChanged)
    Q_PROPERTY(QString currentIconColor READ currentIconColor NOTIFY alertChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY alertChanged)
    Q_PROPERTY(QString currentSubtitle READ currentSubtitle NOTIFY alertChanged)
    Q_PROPERTY(QString action1Text READ action1Text NOTIFY alertChanged)
    Q_PROPERTY(QString action1Id READ action1Id NOTIFY alertChanged)
    Q_PROPERTY(QString action2Text READ action2Text NOTIFY alertChanged)
    Q_PROPERTY(QString action2Id READ action2Id NOTIFY alertChanged)
    Q_PROPERTY(bool isCurrentSticky READ isCurrentSticky NOTIFY alertChanged)
    Q_PROPERTY(QString targetTab READ targetTab NOTIFY alertChanged)

public:
    static AlertManager &instance();

    bool hasActiveAlert() const { return m_hasActiveAlert; }
    QString currentType() const { return m_currentAlert.type; }
    QString currentIcon() const { return m_currentAlert.icon; }
    QString currentIconColor() const { return m_currentAlert.iconColor; }
    QString currentTitle() const { return m_currentAlert.title; }
    QString currentSubtitle() const { return m_currentAlert.subtitle; }
    QString action1Text() const { return m_currentAlert.action1Text; }
    QString action1Id() const { return m_currentAlert.action1Id; }
    QString action2Text() const { return m_currentAlert.action2Text; }
    QString action2Id() const { return m_currentAlert.action2Id; }
    bool isCurrentSticky() const { return m_currentAlert.isSticky; }
    QString targetTab() const { return m_currentAlert.targetTab; }

    Q_INVOKABLE void postAlert(const QString &type,
                               const QString &title,
                               const QString &subtitle,
                               const QString &icon,
                               const QString &iconColor = "#0A84FF",
                               const QString &action1Text = "",
                               const QString &action1Id = "",
                               const QString &action2Text = "",
                               const QString &action2Id = "",
                               int durationMs = 4000,
                               bool isSticky = false,
                               const QString &targetTab = "",
                               const QString &filePath = "");

    Q_INVOKABLE void dismissCurrentAlert();
    Q_INVOKABLE void triggerAction(const QString &actionId);
    Q_INVOKABLE void pauseTimer();
    Q_INVOKABLE void resumeTimer();

signals:
    void alertChanged();

private slots:
    void onAlertTimeout();
    void onScreenshotsDirChanged(const QString &path);
    void onDownloadsDirChanged(const QString &path);

private:
    explicit AlertManager(QObject *parent = nullptr);
    void showNextAlert();
    void initFolderWatchers();

    QList<AlertItem> m_queue;
    AlertItem m_currentAlert;
    bool m_hasActiveAlert = false;
    QTimer *m_dismissTimer = nullptr;
    int m_remainingMs = 0;

    QFileSystemWatcher *m_folderWatcher = nullptr;
    QString m_screenshotsPath;
    QString m_downloadsPath;
    QStringList m_knownScreenshots;
    QStringList m_knownDownloads;
};

} // namespace Aura
