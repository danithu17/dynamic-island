#pragma once

#include <QString>
#include <QImage>

namespace Aura {

struct MediaTrackInfo {
    QString title;
    QString artist;
    QString album;
    bool isPlaying = false;
    bool hasTrack = false;
};

class WinRtMediaBridge {
public:
    static WinRtMediaBridge &instance();

    bool init();
    MediaTrackInfo getCurrentTrack();
    bool play();
    bool pause();
    bool togglePlayPause();
    bool skipNext();
    bool skipPrevious();

private:
    WinRtMediaBridge();
    ~WinRtMediaBridge();

    bool m_initialized = false;
};

} // namespace Aura
