#include "AppState.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QNetworkInterface>
#include <cmath>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Aura {

AppState &AppState::instance() {
    static AppState s_instance;
    return s_instance;
}

AppState::AppState(QObject *parent)
    : QObject(parent),
      m_toastTimer(new QTimer(this)),
      m_eventTimer(new QTimer(this)),
      m_systemStatusTimer(new QTimer(this)) {
    m_currentTab = QStringLiteral("home");
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

    connect(m_systemStatusTimer, &QTimer::timeout, this, &AppState::updateSystemStatus);
    m_systemStatusTimer->start(1000);
    updateSystemStatus();
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

void AppState::setMascotMood(const QString &mood) {
    if (m_mascotMood != mood) {
        m_mascotMood = mood;
        emit mascotMoodChanged();
    }
}

void AppState::updateMascotTracking(qreal eyeDx, qreal eyeDy) {
    qreal clampedX = std::max<qreal>(-4.0, std::min<qreal>(4.0, eyeDx));
    qreal clampedY = std::max<qreal>(-4.0, std::min<qreal>(4.0, eyeDy));
    if (std::abs(m_mascotPupilX - clampedX) > 0.05 || std::abs(m_mascotPupilY - clampedY) > 0.05) {
        m_mascotPupilX = clampedX;
        m_mascotPupilY = clampedY;
        emit mascotTrackingChanged();
    }
}

void AppState::refreshSystemStatus() {
    updateSystemStatus();
}

void AppState::updateSystemStatus() {
    // 1. Time and Date strings
    QDateTime now = QDateTime::currentDateTime();
    m_currentDateString = now.toString(QStringLiteral("ddd dd MMM")).toUpper();
    m_currentTimeString = now.toString(QStringLiteral("h:mm AP"));

    // 2. Battery Status via Win32 GetSystemPowerStatus
    SYSTEM_POWER_STATUS sps;
    if (GetSystemPowerStatus(&sps)) {
        if (sps.BatteryLifePercent != 255) {
            m_batteryPercent = static_cast<int>(sps.BatteryLifePercent);
        } else {
            m_batteryPercent = 100;
        }
        m_isBatteryCharging = (sps.ACLineStatus == 1);
        m_isBatteryLow = (!m_isBatteryCharging && m_batteryPercent <= 20);

        if (sps.BatteryLifeTime != (DWORD)-1 && sps.BatteryLifeTime > 0) {
            int hrs = sps.BatteryLifeTime / 3600;
            int mins = (sps.BatteryLifeTime % 3600) / 60;
            m_batteryTimeRemaining = QString("%1h %2m").arg(hrs).arg(mins);
        } else {
            m_batteryTimeRemaining = m_isBatteryCharging ? QStringLiteral("Charging") : QString();
        }
    }

    // 3. Network Interfaces check
    bool hasNetwork = false;
    const auto ifaces = QNetworkInterface::allInterfaces();
    for (const auto &iface : ifaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            hasNetwork = true;
            break;
        }
    }
    m_isWifiConnected = hasNetwork;
    m_isBluetoothConnected = true;

    // 4. Calculate Sri Lanka Calendar, Poya Days & Public Holidays
    calculateSriLankaCalendar();

    emit systemStatusChanged();
}

void AppState::calculateSriLankaCalendar() {
    QDate today = QDate::currentDate();
    int month = today.month();
    int day = today.day();

    // Sinhala Poya Names per month
    static const QString poyaNames[] = {
        "",
        "Duruthu Full Moon Poya",
        "Navam Full Moon Poya",
        "Medin Full Moon Poya",
        "Bak Full Moon Poya",
        "Vesak Full Moon Poya",
        "Poson Full Moon Poya",
        "Esala Full Moon Poya",
        "Nikini Full Moon Poya",
        "Binara Full Moon Poya",
        "Vap Full Moon Poya",
        "Il Full Moon Poya",
        "Unduvap Full Moon Poya"
    };

    // Calculate approximate full moon day using synodic lunar cycle
    // Reference new moon epoch: Jan 18, 2026 00:00 UTC (Julian day approx)
    qint64 daysSinceEpoch = QDate(2026, 1, 18).daysTo(today);
    double lunarCycle = 29.53058770576;
    double phase = fmod(static_cast<double>(daysSinceEpoch), lunarCycle);
    if (phase < 0) phase += lunarCycle;

    // Full moon occurs at phase ~14.76 days (within ~0.9 days tolerance)
    m_isPoyaToday = (phase >= 13.9 && phase <= 15.6);
    if (month >= 1 && month <= 12) {
        m_poyaDayInfo = poyaNames[month];
    } else {
        m_poyaDayInfo = QStringLiteral("Full Moon Poya Day");
    }

    // Key Sri Lanka Public Holidays
    m_isHolidayToday = false;
    m_holidayInfo.clear();

    if (month == 1 && day == 1) {
        m_isHolidayToday = true; m_holidayInfo = "New Year's Day";
    } else if (month == 1 && day == 15) {
        m_isHolidayToday = true; m_holidayInfo = "Tamil Thai Pongal";
    } else if (month == 2 && day == 4) {
        m_isHolidayToday = true; m_holidayInfo = "National Independence Day";
    } else if (month == 4 && (day == 13 || day == 14)) {
        m_isHolidayToday = true; m_holidayInfo = "Sinhala & Tamil New Year";
    } else if (month == 5 && day == 1) {
        m_isHolidayToday = true; m_holidayInfo = "May Day";
    } else if (month == 12 && day == 25) {
        m_isHolidayToday = true; m_holidayInfo = "Christmas Day";
    } else if (m_isPoyaToday) {
        m_isHolidayToday = true;
        m_holidayInfo = m_poyaDayInfo;
    }

    emit calendarChanged();
}

void AppState::quitApp() {
    emit quitRequested();
}

} // namespace Aura
