#include "MediaManager.h"
#include "WinRtMediaBridge.h"
#include "../../core/AppState.h"
#include <QDebug>
#include <QThreadPool>

#include <winrt/base.h>

namespace Aura {

MediaManager &MediaManager::instance() {
    static MediaManager s_instance;
    return s_instance;
}

MediaManager::MediaManager(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &MediaManager::pollTrack);
    m_timer->setInterval(1000); // Poll media every 1.0s for instant song pickup

    // Start polling after 1s so UI and window initialize first without blocking!
    QTimer::singleShot(1000, this, [this]() {
        m_timer->start();
        pollTrack();
    });
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
    if (m_isPolling) return;
    m_isPolling = true;

    // Run WinRT media query in a background thread so it NEVER blocks the Qt GUI thread!
    QThreadPool::globalInstance()->start([this]() {
        try {
            winrt::init_apartment(winrt::apartment_type::multi_threaded);
        } catch (...) {}

        auto info = WinRtMediaBridge::instance().getCurrentTrack();

        QMetaObject::invokeMethod(this, [this, info]() {
            m_isPolling = false;
            applyTrackInfo(info.title, info.artist, info.album, info.isPlaying, info.hasTrack);
        }, Qt::QueuedConnection);
    });
}

void MediaManager::applyTrackInfo(const QString &title, const QString &artist, const QString &album, bool isPlaying, bool hasTrack) {
    bool changed = (m_title != title ||
                    m_artist != artist ||
                    m_album != album ||
                    m_isPlaying != isPlaying ||
                    m_hasTrack != hasTrack);

    if (changed) {
        bool isNewSong = (hasTrack && isPlaying && (m_title != title || !m_isPlaying));

        m_title = hasTrack ? title : QStringLiteral("No media playing");
        m_artist = hasTrack ? artist : QStringLiteral("Aura Island");
        m_album = album;
        m_isPlaying = isPlaying;
        m_hasTrack = hasTrack;

        AppState::instance().setNowPlayingTitle(m_title);
        AppState::instance().setNowPlayingArtist(m_artist);
        AppState::instance().setIsPlaying(m_isPlaying);

        if (isNewSong) {
            AppState::instance().triggerMusicMorph(m_title, m_artist, 3500);
        }

        emit trackChanged();
    }
}

void MediaManager::play() {
    QThreadPool::globalInstance()->start([this]() {
        try { winrt::init_apartment(winrt::apartment_type::multi_threaded); } catch (...) {}
        WinRtMediaBridge::instance().play();
        QMetaObject::invokeMethod(this, [this]() {
            QTimer::singleShot(250, this, &MediaManager::pollTrack);
        }, Qt::QueuedConnection);
    });
}

void MediaManager::pause() {
    QThreadPool::globalInstance()->start([this]() {
        try { winrt::init_apartment(winrt::apartment_type::multi_threaded); } catch (...) {}
        WinRtMediaBridge::instance().pause();
        QMetaObject::invokeMethod(this, [this]() {
            QTimer::singleShot(250, this, &MediaManager::pollTrack);
        }, Qt::QueuedConnection);
    });
}

void MediaManager::togglePlayPause() {
    QThreadPool::globalInstance()->start([this]() {
        try { winrt::init_apartment(winrt::apartment_type::multi_threaded); } catch (...) {}
        WinRtMediaBridge::instance().togglePlayPause();
        QMetaObject::invokeMethod(this, [this]() {
            QTimer::singleShot(250, this, &MediaManager::pollTrack);
        }, Qt::QueuedConnection);
    });
}

void MediaManager::skipNext() {
    QThreadPool::globalInstance()->start([this]() {
        try { winrt::init_apartment(winrt::apartment_type::multi_threaded); } catch (...) {}
        WinRtMediaBridge::instance().skipNext();
        QMetaObject::invokeMethod(this, [this]() {
            QTimer::singleShot(350, this, &MediaManager::pollTrack);
        }, Qt::QueuedConnection);
    });
}

void MediaManager::skipPrevious() {
    QThreadPool::globalInstance()->start([this]() {
        try { winrt::init_apartment(winrt::apartment_type::multi_threaded); } catch (...) {}
        WinRtMediaBridge::instance().skipPrevious();
        QMetaObject::invokeMethod(this, [this]() {
            QTimer::singleShot(350, this, &MediaManager::pollTrack);
        }, Qt::QueuedConnection);
    });
}

void MediaManager::refresh() {
    pollTrack();
}

} // namespace Aura
