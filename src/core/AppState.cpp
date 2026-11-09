#include "AppState.h"
#include <QCoreApplication>

namespace Aura {

AppState &AppState::instance() {
    static AppState s_instance;
    return s_instance;
}

AppState::AppState(QObject *parent)
    : QObject(parent),
      m_toastTimer(new QTimer(this)),
      m_eventTimer(new QTimer(this)) {
    m_toastTimer->setSingleShot(true);
    connect(m_toastTimer, &QTimer::timeout, this, &AppState::dismissToast);

    m_eventTimer->setSingleShot(true);
    connect(m_eventTimer, &QTimer::timeout, this, [this]() {
        if (m_islandState == "compact") {
            collapseToIdle();
        }
        m_compactEventMessage.clear();
        emit compactEventMessageChanged();
    });
}

void AppState::triggerEventCompact(const QString &msg, int durationMs) {
    m_compactEventMessage = msg;
    emit compactEventMessageChanged();

    // If currently idle, morph to compact for 3 seconds then return to idle
    if (m_islandState == "idle") {
        setIslandState("compact");
    }
    m_eventTimer->start(durationMs);
}

void AppState::setIsQuickCaptureActive(bool active) {
    if (m_isQuickCaptureActive != active) {
        m_isQuickCaptureActive = active;
        emit quickCaptureActiveChanged();
        if (active) {
            setIslandState("compact");
        }
    }
}

void AppState::openQuickCapture() {
    setIsQuickCaptureActive(true);
}

void AppState::closeQuickCapture() {
    setIsQuickCaptureActive(false);
    collapseToIdle();
}

void AppState::setIslandState(const QString &state) {
    if (m_islandState != state) {
        m_islandState = state;
        emit islandStateChanged();

        if (m_islandState == "idle") {
            emit requestWindowResize(120, 32, 16);
        } else if (m_islandState == "compact") {
            emit requestWindowResize(260, 36, 18);
        } else if (m_islandState == "expanded") {
            int h = (m_currentTab == "home") ? 270 : 300;
            emit requestWindowResize(480, h, 36);
        } else if (m_islandState == "drop") {
            emit requestWindowResize(480, 200, 24);
        }
    }
}

void AppState::setCurrentTab(const QString &tab) {
    if (m_currentTab != tab) {
        m_currentTab = tab;
        emit currentTabChanged();
        if (m_islandState == "expanded") {
            int h = (m_currentTab == "home") ? 270 : 300;
            emit requestWindowResize(480, h, 36);
        }
    }
}

void AppState::setIsPinned(bool pinned) {
    if (m_isPinned != pinned) {
        m_isPinned = pinned;
        emit isPinnedChanged();
    }
}

void AppState::setUnreadMailCount(int count) {
    if (m_unreadMailCount != count) {
        m_unreadMailCount = count;
        emit unreadMailCountChanged();
    }
}

void AppState::setNowPlayingTitle(const QString &title) {
    if (m_nowPlayingTitle != title) {
        m_nowPlayingTitle = title;
        emit nowPlayingTitleChanged();
    }
}

void AppState::setNowPlayingArtist(const QString &artist) {
    if (m_nowPlayingArtist != artist) {
        m_nowPlayingArtist = artist;
        emit nowPlayingArtistChanged();
    }
}

void AppState::setIsPlaying(bool playing) {
    if (m_isPlaying != playing) {
        m_isPlaying = playing;
        emit isPlayingChanged();
    }
}

void AppState::setPomodoroDisplay(const QString &disp) {
    if (m_pomodoroDisplay != disp) {
        m_pomodoroDisplay = disp;
        emit pomodoroDisplayChanged();
    }
}

void AppState::setAgentStatus(const QString &status) {
    if (m_agentStatus != status) {
        m_agentStatus = status;
        emit agentStatusChanged();
    }
}

void AppState::setAgentMessage(const QString &msg) {
    if (m_agentMessage != msg) {
        m_agentMessage = msg;
        emit agentMessageChanged();
    }
}

void AppState::setGitBranch(const QString &branch) {
    if (m_gitBranch != branch) {
        m_gitBranch = branch;
        emit gitBranchChanged();
    }
}

void AppState::setGitUncommittedCount(int count) {
    if (m_gitUncommittedCount != count) {
        m_gitUncommittedCount = count;
        emit gitUncommittedCountChanged();
    }
}

void AppState::toggleExpanded() {
    if (m_islandState == "expanded") {
        collapseToIdle();
    } else {
        setIslandState("expanded");
    }
}

void AppState::collapseToIdle() {
    setIslandState("idle");
}

void AppState::collapseToCompact() {
    setIslandState("compact");
}

void AppState::expandTab(const QString &tab) {
    setCurrentTab(tab);
    setIslandState("expanded");
}

void AppState::showToast(const QString &msg, const QString &type) {
    m_toastMessage = msg;
    m_toastType = type;
    emit toastChanged();
    m_toastTimer->start(3500); // auto-hide after 3.5s
}

void AppState::dismissToast() {
    if (!m_toastMessage.isEmpty()) {
        m_toastMessage.clear();
        emit toastChanged();
    }
}

void AppState::quitApp() {
    emit quitRequested();
}

} // namespace Aura
