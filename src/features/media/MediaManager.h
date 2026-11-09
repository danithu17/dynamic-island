#pragma once

#include <QObject>
#include <QTimer>
#include <QString>

namespace Aura {

class MediaManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString title READ title NOTIFY trackChanged)
    Q_PROPERTY(QString artist READ artist NOTIFY trackChanged)
    Q_PROPERTY(QString album READ album NOTIFY trackChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY trackChanged)
    Q_PROPERTY(bool hasTrack READ hasTrack NOTIFY trackChanged)

public:
    static MediaManager &instance();

    QString title() const { return m_title; }
    QString artist() const { return m_artist; }
    QString album() const { return m_album; }
    bool isPlaying() const { return m_isPlaying; }
    bool hasTrack() const { return m_hasTrack; }

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void togglePlayPause();
    Q_INVOKABLE void skipNext();
    Q_INVOKABLE void skipPrevious();
    Q_INVOKABLE void refresh();

    void setPollingPaused(bool paused);

signals:
    void trackChanged();

private slots:
    void pollTrack();

private:
    explicit MediaManager(QObject *parent = nullptr);
    void applyTrackInfo(const QString &title, const QString &artist, const QString &album, bool isPlaying, bool hasTrack);

    QString m_title = "No media playing";
    QString m_artist = "Idle";
    QString m_album;
    bool m_isPlaying = false;
    bool m_hasTrack = false;
    bool m_isPolling = false;
    QTimer *m_timer = nullptr;
};

} // namespace Aura
