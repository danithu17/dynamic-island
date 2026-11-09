#include "PomodoroTimer.h"
#include "../../core/AppState.h"
#include "../../core/Settings.h"
#include <windows.h>

namespace Aura {

PomodoroTimer::PomodoroTimer(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &PomodoroTimer::onOneSecondTick);
    applyModeDuration();
    AppState::instance().setPomodoroDisplay(timeDisplay());
}

QString PomodoroTimer::timeDisplay() const {
    int m = m_secondsRemaining / 60;
    int s = m_secondsRemaining % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void PomodoroTimer::applyModeDuration() {
    auto &settings = Settings::instance();
    if (m_mode == "work") {
        m_secondsRemaining = settings.pomodoroWorkMin() * 60;
    } else if (m_mode == "shortBreak") {
        m_secondsRemaining = settings.pomodoroShortBreakMin() * 60;
    } else {
        m_secondsRemaining = settings.pomodoroLongBreakMin() * 60;
    }
}

void PomodoroTimer::start() {
    if (!m_isRunning) {
        m_isRunning = true;
        m_timer->start(1000);
        emit stateChanged();
    }
}

void PomodoroTimer::pause() {
    if (m_isRunning) {
        m_isRunning = false;
        m_timer->stop();
        emit stateChanged();
    }
}

void PomodoroTimer::reset() {
    m_isRunning = false;
    m_timer->stop();
    applyModeDuration();
    AppState::instance().setPomodoroDisplay(timeDisplay());
    emit stateChanged();
    emit tick();
}

void PomodoroTimer::skipToNext() {
    if (m_mode == "work") {
        m_completedCycles++;
        if (m_completedCycles % 4 == 0) {
            m_mode = "longBreak";
        } else {
            m_mode = "shortBreak";
        }
    } else {
        m_mode = "work";
    }
    m_isRunning = false;
    m_timer->stop();
    applyModeDuration();
    AppState::instance().setPomodoroDisplay(timeDisplay());
    emit stateChanged();
    emit tick();
}

void PomodoroTimer::onOneSecondTick() {
    if (m_secondsRemaining > 0) {
        m_secondsRemaining--;
        QString disp = timeDisplay();
        AppState::instance().setPomodoroDisplay(disp);
        emit tick();
    }

    if (m_secondsRemaining <= 0) {
        // Mode finished!
        MessageBeep(MB_ICONASTERISK); // Native chime
        QString finishedMode = m_mode;
        if (m_mode == "work") {
            m_completedCycles++;
            AppState::instance().showToast("Pomodoro work cycle finished! Take a break.", "success");
            if (m_completedCycles % 4 == 0) {
                m_mode = "longBreak";
            } else {
                m_mode = "shortBreak";
            }
        } else {
            AppState::instance().showToast("Break finished! Time to focus.", "info");
            m_mode = "work";
        }

        m_isRunning = false;
        m_timer->stop();
        applyModeDuration();
        AppState::instance().setPomodoroDisplay(timeDisplay());
        emit cycleCompleted(finishedMode);
        emit stateChanged();
        emit tick();
    }
}

} // namespace Aura
