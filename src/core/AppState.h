#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

namespace Aura {

class AppState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString islandState READ islandState WRITE setIslandState NOTIFY islandStateChanged)
    Q_PROPERTY(QString currentTab READ currentTab WRITE setCurrentTab NOTIFY currentTabChanged)
    Q_PROPERTY(bool isPinned READ isPinned WRITE setIsPinned NOTIFY isPinnedChanged)
    Q_PROPERTY(int unreadMailCount READ unreadMailCount WRITE setUnreadMailCount NOTIFY unreadMailCountChanged)
    Q_PROPERTY(QString nowPlayingTitle READ nowPlayingTitle WRITE setNowPlayingTitle NOTIFY nowPlayingTitleChanged)
    Q_PROPERTY(QString nowPlayingArtist READ nowPlayingArtist WRITE setNowPlayingArtist NOTIFY nowPlayingArtistChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying WRITE setIsPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(QString pomodoroDisplay READ pomodoroDisplay WRITE setPomodoroDisplay NOTIFY pomodoroDisplayChanged)
    Q_PROPERTY(QString agentStatus READ agentStatus WRITE setAgentStatus NOTIFY agentStatusChanged)
    Q_PROPERTY(QString agentMessage READ agentMessage WRITE setAgentMessage NOTIFY agentMessageChanged)
    Q_PROPERTY(QString gitBranch READ gitBranch WRITE setGitBranch NOTIFY gitBranchChanged)
    Q_PROPERTY(int gitUncommittedCount READ gitUncommittedCount WRITE setGitUncommittedCount NOTIFY gitUncommittedCountChanged)
    Q_PROPERTY(QString toastMessage READ toastMessage NOTIFY toastChanged)
    Q_PROPERTY(QString toastType READ toastType NOTIFY toastChanged)
    Q_PROPERTY(bool hasToast READ hasToast NOTIFY toastChanged)
    Q_PROPERTY(QString compactEventMessage READ compactEventMessage NOTIFY compactEventMessageChanged)
    Q_PROPERTY(bool isMusicMorphActive READ isMusicMorphActive NOTIFY musicMorphChanged)
    Q_PROPERTY(bool isQuickCaptureActive READ isQuickCaptureActive WRITE setIsQuickCaptureActive NOTIFY quickCaptureActiveChanged)

    // Step 2: System Status & Mascot Properties
    Q_PROPERTY(int batteryPercent READ batteryPercent NOTIFY systemStatusChanged)
    Q_PROPERTY(bool isBatteryCharging READ isBatteryCharging NOTIFY systemStatusChanged)
    Q_PROPERTY(QString batteryTimeRemaining READ batteryTimeRemaining NOTIFY systemStatusChanged)
    Q_PROPERTY(bool isBatteryLow READ isBatteryLow NOTIFY systemStatusChanged)
    Q_PROPERTY(bool isWifiConnected READ isWifiConnected NOTIFY systemStatusChanged)
    Q_PROPERTY(bool isBluetoothConnected READ isBluetoothConnected NOTIFY systemStatusChanged)
    Q_PROPERTY(QString currentDateString READ currentDateString NOTIFY systemStatusChanged)
    Q_PROPERTY(QString currentTimeString READ currentTimeString NOTIFY systemStatusChanged)
    Q_PROPERTY(QString mascotMood READ mascotMood NOTIFY mascotMoodChanged)
    Q_PROPERTY(qreal mascotPupilX READ mascotPupilX NOTIFY mascotTrackingChanged)
    Q_PROPERTY(qreal mascotPupilY READ mascotPupilY NOTIFY mascotTrackingChanged)
    Q_PROPERTY(QString poyaDayInfo READ poyaDayInfo NOTIFY calendarChanged)
    Q_PROPERTY(bool isPoyaToday READ isPoyaToday NOTIFY calendarChanged)
    Q_PROPERTY(QString holidayInfo READ holidayInfo NOTIFY calendarChanged)
    Q_PROPERTY(bool isHolidayToday READ isHolidayToday NOTIFY calendarChanged)

public:
    static AppState &instance();

    QString islandState() const { return m_islandState; }
    Q_INVOKABLE void setIslandState(const QString &state);

    QString currentTab() const { return m_currentTab; }
    Q_INVOKABLE void setCurrentTab(const QString &tab);

    bool isPinned() const { return m_isPinned; }
    Q_INVOKABLE void setIsPinned(bool pinned);

    int unreadMailCount() const { return m_unreadMailCount; }
    Q_INVOKABLE void setUnreadMailCount(int count);

    QString nowPlayingTitle() const { return m_nowPlayingTitle; }
    Q_INVOKABLE void setNowPlayingTitle(const QString &title);

    QString nowPlayingArtist() const { return m_nowPlayingArtist; }
    Q_INVOKABLE void setNowPlayingArtist(const QString &artist);

    bool isPlaying() const { return m_isPlaying; }
    Q_INVOKABLE void setIsPlaying(bool playing);

    QString pomodoroDisplay() const { return m_pomodoroDisplay; }
    Q_INVOKABLE void setPomodoroDisplay(const QString &disp);

    QString agentStatus() const { return m_agentStatus; }
    Q_INVOKABLE void setAgentStatus(const QString &status);

    QString agentMessage() const { return m_agentMessage; }
    void setAgentMessage(const QString &msg);

    QString gitBranch() const { return m_gitBranch; }
    void setGitBranch(const QString &branch);

    int gitUncommittedCount() const { return m_gitUncommittedCount; }
    void setGitUncommittedCount(int count);

    QString toastMessage() const { return m_toastMessage; }
    QString toastType() const { return m_toastType; }
    bool hasToast() const { return !m_toastMessage.isEmpty(); }

    QString compactEventMessage() const { return m_compactEventMessage; }
    bool isMusicMorphActive() const { return m_isMusicMorphActive; }
    bool isQuickCaptureActive() const { return m_isQuickCaptureActive; }
    void setIsQuickCaptureActive(bool active);

    int batteryPercent() const { return m_batteryPercent; }
    bool isBatteryCharging() const { return m_isBatteryCharging; }
    QString batteryTimeRemaining() const { return m_batteryTimeRemaining; }
    bool isBatteryLow() const { return m_isBatteryLow; }
    bool isWifiConnected() const { return m_isWifiConnected; }
    bool isBluetoothConnected() const { return m_isBluetoothConnected; }
    QString currentDateString() const { return m_currentDateString; }
    QString currentTimeString() const { return m_currentTimeString; }
    QString mascotMood() const { return m_mascotMood; }
    qreal mascotPupilX() const { return m_mascotPupilX; }
    qreal mascotPupilY() const { return m_mascotPupilY; }
    QString poyaDayInfo() const { return m_poyaDayInfo; }
    bool isPoyaToday() const { return m_isPoyaToday; }
    QString holidayInfo() const { return m_holidayInfo; }
    bool isHolidayToday() const { return m_isHolidayToday; }

    Q_INVOKABLE void setMascotMood(const QString &mood);
    Q_INVOKABLE void updateMascotTracking(qreal eyeDx, qreal eyeDy);
    Q_INVOKABLE void refreshSystemStatus();

    Q_INVOKABLE void toggleExpanded();
    Q_INVOKABLE void collapseToIdle();
    Q_INVOKABLE void collapseToCompact();
    Q_INVOKABLE void expandTab(const QString &tab);
    Q_INVOKABLE void showToast(const QString &msg, const QString &type = "info");
    Q_INVOKABLE void dismissToast();
    Q_INVOKABLE void triggerEventCompact(const QString &msg, int durationMs = 3000);
    Q_INVOKABLE void triggerMusicMorph(const QString &title, const QString &artist, int durationMs = 3500);
    Q_INVOKABLE void openQuickCapture();
    Q_INVOKABLE void closeQuickCapture();
    Q_INVOKABLE void quitApp();

signals:
    void islandStateChanged();
    void currentTabChanged();
    void isPinnedChanged();
    void unreadMailCountChanged();
    void nowPlayingTitleChanged();
    void nowPlayingArtistChanged();
    void isPlayingChanged();
    void pomodoroDisplayChanged();
    void agentStatusChanged();
    void agentMessageChanged();
    void gitBranchChanged();
    void gitUncommittedCountChanged();
    void toastChanged();
    void compactEventMessageChanged();
    void musicMorphChanged();
    void quickCaptureActiveChanged();
    void systemStatusChanged();
    void mascotMoodChanged();
    void mascotTrackingChanged();
    void calendarChanged();
    void requestWindowResize(int width, int height, int radius);
    void quitRequested();

private:
    explicit AppState(QObject *parent = nullptr);

    QString m_islandState = "idle"; // "idle", "compact", "expanded", "drop"
    QString m_currentTab = "shelf";
    bool m_isPinned = false;

    int m_unreadMailCount = 0;
    QString m_nowPlayingTitle;
    QString m_nowPlayingArtist;
    bool m_isPlaying = false;

    QString m_pomodoroDisplay = "25:00";
    QString m_agentStatus = "idle";
    QString m_agentMessage;
    QString m_gitBranch = "main";
    int m_gitUncommittedCount = 0;

    QString m_toastMessage;
    QString m_toastType = "info";
    QTimer *m_toastTimer = nullptr;

    QString m_compactEventMessage;
    bool m_isMusicMorphActive = false;
    QTimer *m_eventTimer = nullptr;
    bool m_isQuickCaptureActive = false;

    // Step 2 members
    int m_batteryPercent = 100;
    bool m_isBatteryCharging = false;
    int m_prevAcLineStatus = -1;
    bool m_hasLowBatteryWarned = false;
    QString m_batteryTimeRemaining;
    bool m_isBatteryLow = false;
    bool m_isWifiConnected = true;
    bool m_isBluetoothConnected = true;
    QString m_currentDateString;
    QString m_currentTimeString;
    QString m_mascotMood = "normal";
    qreal m_mascotPupilX = 0.0;
    qreal m_mascotPupilY = 0.0;
    QString m_poyaDayInfo;
    bool m_isPoyaToday = false;
    QString m_holidayInfo;
    bool m_isHolidayToday = false;
    QTimer *m_systemStatusTimer = nullptr;
    void updateSystemStatus();
    void calculateSriLankaCalendar();
};

} // namespace Aura
