#include "MediaManager.h"
#include "WinRtMediaBridge.h"
#include "../../core/AppState.h"
#include <QDebug>

namespace Aura {

MediaManager &MediaManager::instance() {
    static MediaManager s_instance;
    return s_instance;
}

MediaManager::MediaManager(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &MediaManager::pollTrack);
    m_timer->setInterval(1500); // Poll media every 1.5s
    m_timer->start();
    pollTrack();
}

void MediaManager::setPollingPaused(bool paused) {
    if (paused) {
        m_timer->stop();
    } else {
        if (!m_timer->isActive()) {
            m_timer->start();
            pollTrack();
        }
    }
}

void MediaManager::pollTrack() {
    auto info = WinRtMediaBridge::instance().getCurrentTrack();

    bool changed = (m_title != info.title ||
                    m_artist != info.artist ||
                    m_album != info.album ||
                    m_isPlaying != info.isPlaying ||
                    m_hasTrack != info.hasTrack);

    if (changed) {
        m_title = info.hasTrack ? info.title : "No media playing";
        m_artist = info.hasTrack ? info.artist : "Aura Island";
        m_album = info.album;
        m_isPlaying = info.isPlaying;
        m_hasTrack = info.hasTrack;

        AppState::instance().setNowPlayingTitle(m_title);
        AppState::instance().setNowPlayingArtist(m_artist);
        AppState::instance().setIsPlaying(m_isPlaying);

        emit trackChanged();
    }
}

void MediaManager::play() {
    WinRtMediaBridge::instance().play();
    QTimer::singleShot(200, this, &MediaManager::pollTrack);
}

void MediaManager::pause() {
    WinRtMediaBridge::instance().pause();
    QTimer::singleShot(200, this, &MediaManager::pollTrack);
}

void MediaManager::togglePlayPause() {
    WinRtMediaBridge::instance().togglePlayPause();
    QTimer::singleShot(200, this, &MediaManager::pollTrack);
}

void MediaManager::skipNext() {
    WinRtMediaBridge::instance().skipNext();
    QTimer::singleShot(300, this, &MediaManager::pollTrack);
}

void MediaManager::skipPrevious() {
    WinRtMediaBridge::instance().skipPrevious();
    QTimer::singleShot(300, this, &MediaManager::pollTrack);
}

void MediaManager::refresh() {
    pollTrack();
}

} // namespace Aura
