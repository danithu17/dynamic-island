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

public:
    static AppState &instance();

    QString islandState() const { return m_islandState; }
    void setIslandState(const QString &state);

    QString currentTab() const { return m_currentTab; }
    void setCurrentTab(const QString &tab);

    bool isPinned() const { return m_isPinned; }
    void setIsPinned(bool pinned);

    int unreadMailCount() const { return m_unreadMailCount; }
    void setUnreadMailCount(int count);

    QString nowPlayingTitle() const { return m_nowPlayingTitle; }
    void setNowPlayingTitle(const QString &title);

    QString nowPlayingArtist() const { return m_nowPlayingArtist; }
    void setNowPlayingArtist(const QString &artist);

    bool isPlaying() const { return m_isPlaying; }
    void setIsPlaying(bool playing);

    QString pomodoroDisplay() const { return m_pomodoroDisplay; }
    void setPomodoroDisplay(const QString &disp);

    QString agentStatus() const { return m_agentStatus; }
    void setAgentStatus(const QString &status);

    QString agentMessage() const { return m_agentMessage; }
    void setAgentMessage(const QString &msg);

    QString gitBranch() const { return m_gitBranch; }
    void setGitBranch(const QString &branch);

    int gitUncommittedCount() const { return m_gitUncommittedCount; }
    void setGitUncommittedCount(int count);

    QString toastMessage() const { return m_toastMessage; }
    QString toastType() const { return m_toastType; }
    bool hasToast() const { return !m_toastMessage.isEmpty(); }

    Q_INVOKABLE void toggleExpanded();
    Q_INVOKABLE void collapseToIdle();
    Q_INVOKABLE void collapseToCompact();
    Q_INVOKABLE void expandTab(const QString &tab);
    Q_INVOKABLE void showToast(const QString &msg, const QString &type = "info");
    Q_INVOKABLE void dismissToast();
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
    QString m_gitBranch;
    int m_gitUncommittedCount = 0;

    QString m_toastMessage;
    QString m_toastType = "info";
    QTimer *m_toastTimer = nullptr;
};

} // namespace Aura
