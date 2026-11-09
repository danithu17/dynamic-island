#include "WinRtMediaBridge.h"
#include <QDebug>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt;
using namespace winrt::Windows::Media::Control;

namespace Aura {

WinRtMediaBridge &WinRtMediaBridge::instance() {
    static WinRtMediaBridge s_instance;
    return s_instance;
}

WinRtMediaBridge::WinRtMediaBridge() {
    init();
}

WinRtMediaBridge::~WinRtMediaBridge() {
    // Apartment uninit if needed
}

bool WinRtMediaBridge::init() {
    if (m_initialized) return true;
    try {
        winrt::init_apartment();
        m_initialized = true;
    } catch (...) {
        // Apartment may already be initialized by COM / Qt, which is completely fine
        m_initialized = true;
    }
    return m_initialized;
}

MediaTrackInfo WinRtMediaBridge::getCurrentTrack() {
    MediaTrackInfo info;
    info.hasTrack = false;
    info.isPlaying = false;

    try {
        auto asyncOp = GlobalSystemMediaTransportControlsSessionManager::RequestAsync();
        auto manager = asyncOp.get();
        if (!manager) return info;

        auto session = manager.GetCurrentSession();
        if (!session) return info;

        auto playbackInfo = session.GetPlaybackInfo();
        if (playbackInfo) {
            info.isPlaying = (playbackInfo.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing);
        }

        auto mediaPropsOp = session.TryGetMediaPropertiesAsync();
        auto mediaProperties = mediaPropsOp.get();
        if (mediaProperties) {
            info.title = QString::fromStdWString(std::wstring(mediaProperties.Title()));
            info.artist = QString::fromStdWString(std::wstring(mediaProperties.Artist()));
            info.album = QString::fromStdWString(std::wstring(mediaProperties.AlbumTitle()));
            info.hasTrack = !info.title.isEmpty() || !info.artist.isEmpty();
        }
    } catch (const winrt::hresult_error &ex) {
        qDebug() << "WinRT media error:" << QString::fromStdWString(std::wstring(ex.message()));
    } catch (...) {
        qDebug() << "WinRT media unknown error";
    }

    return info;
}

bool WinRtMediaBridge::play() {
    try {
        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (manager) {
            auto session = manager.GetCurrentSession();
            if (session) {
                return session.TryPlayAsync().get();
            }
        }
    } catch (...) {}
    return false;
}

bool WinRtMediaBridge::pause() {
    try {
        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (manager) {
            auto session = manager.GetCurrentSession();
            if (session) {
                return session.TryPauseAsync().get();
            }
        }
    } catch (...) {}
    return false;
}

bool WinRtMediaBridge::togglePlayPause() {
    try {
        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (manager) {
            auto session = manager.GetCurrentSession();
            if (session) {
                return session.TryTogglePlayPauseAsync().get();
            }
        }
    } catch (...) {}
    return false;
}

bool WinRtMediaBridge::skipNext() {
    try {
        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (manager) {
            auto session = manager.GetCurrentSession();
            if (session) {
                return session.TrySkipNextAsync().get();
            }
        }
    } catch (...) {}
    return false;
}

bool WinRtMediaBridge::skipPrevious() {
    try {
        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (manager) {
            auto session = manager.GetCurrentSession();
            if (session) {
                return session.TrySkipPreviousAsync().get();
            }
        }
    } catch (...) {}
    return false;
}

} // namespace Aura
