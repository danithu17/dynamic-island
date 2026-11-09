#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

namespace Aura {

class SystemControlManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool isMuted READ isMuted NOTIFY muteChanged)
    Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(bool isTouchpadEnabled READ isTouchpadEnabled NOTIFY touchpadChanged)

public:
    static SystemControlManager &instance();

    int volume() const { return m_volume; }
    void setVolume(int level);

    bool isMuted() const { return m_isMuted; }
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setMuted(bool mute);

    int brightness() const { return m_brightness; }
    void setBrightness(int level);

    bool isTouchpadEnabled() const { return m_isTouchpadEnabled; }
    Q_INVOKABLE void toggleTouchpad();

    // Power Actions
    Q_INVOKABLE void lockScreen();
    Q_INVOKABLE void sleepPC();
    Q_INVOKABLE void restartPC();
    Q_INVOKABLE void shutdownPC();

    Q_INVOKABLE void refreshAll();

signals:
    void volumeChanged();
    void muteChanged();
    void brightnessChanged();
    void touchpadChanged();

private slots:
    void pollSystemSettings();

private:
    explicit SystemControlManager(QObject *parent = nullptr);
    ~SystemControlManager();

    void initAudio();
    void updateVolumeFromSystem();
    void updateBrightnessFromSystem();
    void updateTouchpadFromSystem();

    int m_volume = 50;
    bool m_isMuted = false;
    int m_brightness = 75;
    bool m_isTouchpadEnabled = true;

    QTimer *m_pollTimer = nullptr;
    void *m_endpointVolume = nullptr; // IAudioEndpointVolume*
};

} // namespace Aura
