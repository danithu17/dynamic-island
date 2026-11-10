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
    Q_PROPERTY(bool hasBrightness READ hasBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(bool isTouchpadEnabled READ isTouchpadEnabled NOTIFY touchpadChanged)
    Q_PROPERTY(bool isMicMuted READ isMicMuted NOTIFY micMuteChanged)
    Q_PROPERTY(bool isKeepAwake READ isKeepAwake NOTIFY keepAwakeChanged)
    Q_PROPERTY(bool isDarkMode READ isDarkMode NOTIFY darkModeChanged)

public:
    static SystemControlManager &instance();

    int volume() const { return m_volume; }
    void setVolume(int level);

    bool isMuted() const { return m_isMuted; }
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void setMuted(bool mute);

    int brightness() const { return m_brightness; }
    bool hasBrightness() const { return m_hasBrightness; }
    void setBrightness(int level);

    bool isTouchpadEnabled() const { return m_isTouchpadEnabled; }
    Q_INVOKABLE void toggleTouchpad();

    bool isMicMuted() const { return m_isMicMuted; }
    Q_INVOKABLE void toggleMicMute();

    bool isKeepAwake() const { return m_isKeepAwake; }
    Q_INVOKABLE void toggleKeepAwake();

    bool isDarkMode() const { return m_isDarkMode; }
    Q_INVOKABLE void toggleDarkMode();

    Q_INVOKABLE void toggleWifi();
    Q_INVOKABLE void toggleBluetooth();

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
    void micMuteChanged();
    void keepAwakeChanged();
    void darkModeChanged();

private slots:
    void pollSystemSettings();

private:
    explicit SystemControlManager(QObject *parent = nullptr);
    ~SystemControlManager();

    void initAudio();
    void initMic();
    void updateVolumeFromSystem();
    void updateBrightnessFromSystem();
    void updateTouchpadFromSystem();

    int m_volume = 50;
    bool m_isMuted = false;
    int m_brightness = 75;
    bool m_hasBrightness = true;
    bool m_isTouchpadEnabled = true;
    bool m_isMicMuted = false;
    bool m_isKeepAwake = false;
    bool m_isDarkMode = true;

    QTimer *m_pollTimer = nullptr;
    void *m_endpointVolume = nullptr; // IAudioEndpointVolume*
    void *m_micEndpointVolume = nullptr;
};

} // namespace Aura
