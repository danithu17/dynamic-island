#include "FullscreenDetector.h"
#include <windows.h>
#include <shellapi.h>
#include <QDebug>

namespace Aura {

FullscreenDetector::FullscreenDetector(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &FullscreenDetector::checkState);
}

FullscreenDetector::~FullscreenDetector() {
    stop();
}

void FullscreenDetector::start(int intervalMs) {
    checkState();
    m_timer->start(intervalMs);
}

void FullscreenDetector::stop() {
    m_timer->stop();
}

void FullscreenDetector::checkState() {
    QUERY_USER_NOTIFICATION_STATE state;
    HRESULT hr = SHQueryUserNotificationState(&state);
    if (SUCCEEDED(hr)) {
        // Only suppress during actual exclusive Direct3D fullscreen games (QUNS_RUNNING_D3D_FULL_SCREEN)
        // Never hide during QUNS_BUSY or normal desktop windows, so Island is always visible at the top!
        bool fullscreen = (state == QUNS_RUNNING_D3D_FULL_SCREEN);

        if (fullscreen != m_isFullscreen) {
            m_isFullscreen = fullscreen;
            emit fullscreenStateChanged(m_isFullscreen);
        }
    }
}

} // namespace Aura
