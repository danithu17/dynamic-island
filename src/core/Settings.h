#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace Aura {

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString greeting READ greeting NOTIFY greetingChanged)
    Q_PROPERTY(bool onboardingCompleted READ onboardingCompleted WRITE setOnboardingCompleted NOTIFY onboardingCompletedChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString hotkey READ hotkey WRITE setHotkey NOTIFY hotkeyChanged)
    Q_PROPERTY(bool clipboardEnabled READ clipboardEnabled WRITE setClipboardEnabled NOTIFY clipboardEnabledChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(bool mascotEyesEnabled READ mascotEyesEnabled WRITE setMascotEyesEnabled NOTIFY mascotEyesEnabledChanged)
    Q_PROPERTY(QString islandStyle READ islandStyle WRITE setIslandStyle NOTIFY islandStyleChanged)
    Q_PROPERTY(bool reduceAnimations READ reduceAnimations WRITE setReduceAnimations NOTIFY reduceAnimationsChanged)
    Q_PROPERTY(QStringList homeChips READ homeChips WRITE setHomeChips NOTIFY homeChipsChanged)

    // Alerts toggles
    Q_PROPERTY(bool alertBattery READ alertBattery WRITE setAlertBattery NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertScreenshots READ alertScreenshots WRITE setAlertScreenshots NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertDownloads READ alertDownloads WRITE setAlertDownloads NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertTimer READ alertTimer WRITE setAlertTimer NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertGmail READ alertGmail WRITE setAlertGmail NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertAgent READ alertAgent WRITE setAlertAgent NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertUsb READ alertUsb WRITE setAlertUsb NOTIFY alertConfigChanged)
    Q_PROPERTY(bool alertVolume READ alertVolume WRITE setAlertVolume NOTIFY alertConfigChanged)

    // Developer / Pomodoro / Idle
    Q_PROPERTY(int pollIntervalMinutes READ pollIntervalMinutes WRITE setPollIntervalMinutes NOTIFY pollIntervalMinutesChanged)
    Q_PROPERTY(int pomodoroWorkMin READ pomodoroWorkMin WRITE setPomodoroWorkMin NOTIFY pomodoroWorkMinChanged)
    Q_PROPERTY(int pomodoroShortBreakMin READ pomodoroShortBreakMin WRITE setPomodoroShortBreakMin NOTIFY pomodoroShortBreakMinChanged)
    Q_PROPERTY(int pomodoroLongBreakMin READ pomodoroLongBreakMin WRITE setPomodoroLongBreakMin NOTIFY pomodoroLongBreakMinChanged)
    Q_PROPERTY(int agentServerPort READ agentServerPort WRITE setAgentServerPort NOTIFY agentServerPortChanged)
    Q_PROPERTY(QString agentServerToken READ agentServerToken WRITE setAgentServerToken NOTIFY agentServerTokenChanged)
    Q_PROPERTY(QString idleWidget READ idleWidget WRITE setIdleWidget NOTIFY idleWidgetChanged)
    Q_PROPERTY(bool autoPaste READ autoPaste WRITE setAutoPaste NOTIFY autoPasteChanged)
    Q_PROPERTY(bool pasteStackEnabled READ pasteStackEnabled WRITE setPasteStackEnabled NOTIFY pasteStackEnabledChanged)

public:
    static Settings &instance();

    QString displayName() const { return m_displayName; }
    void setDisplayName(const QString &val);

    QString greeting() const;

    bool onboardingCompleted() const { return m_onboardingCompleted; }
    void setOnboardingCompleted(bool val);

    QString accentColor() const { return m_accentColor; }
    void setAccentColor(const QString &val);

    QString hotkey() const { return m_hotkey; }
    void setHotkey(const QString &val);

    bool clipboardEnabled() const { return m_clipboardEnabled; }
    void setClipboardEnabled(bool val);

    bool autoStart() const { return m_autoStart; }
    void setAutoStart(bool val);

    bool mascotEyesEnabled() const { return m_mascotEyesEnabled; }
    void setMascotEyesEnabled(bool val);

    QString islandStyle() const { return m_islandStyle; }
    void setIslandStyle(const QString &val);

    bool reduceAnimations() const { return m_reduceAnimations; }
    void setReduceAnimations(bool val);

    QStringList homeChips() const { return m_homeChips; }
    void setHomeChips(const QStringList &chips);
    Q_INVOKABLE bool isHomeChipActive(const QString &chipId) const;
    Q_INVOKABLE void toggleHomeChip(const QString &chipId);

    bool alertBattery() const { return m_alertBattery; }
    void setAlertBattery(bool val);
    bool alertScreenshots() const { return m_alertScreenshots; }
    void setAlertScreenshots(bool val);
    bool alertDownloads() const { return m_alertDownloads; }
    void setAlertDownloads(bool val);
    bool alertTimer() const { return m_alertTimer; }
    void setAlertTimer(bool val);
    bool alertGmail() const { return m_alertGmail; }
    void setAlertGmail(bool val);
    bool alertAgent() const { return m_alertAgent; }
    void setAlertAgent(bool val);
    bool alertUsb() const { return m_alertUsb; }
    void setAlertUsb(bool val);
    bool alertVolume() const { return m_alertVolume; }
    void setAlertVolume(bool val);

    int pollIntervalMinutes() const { return m_pollIntervalMinutes; }
    void setPollIntervalMinutes(int val);

    int pomodoroWorkMin() const { return m_pomodoroWorkMin; }
    void setPomodoroWorkMin(int val);

    int pomodoroShortBreakMin() const { return m_pomodoroShortBreakMin; }
    void setPomodoroShortBreakMin(int val);

    int pomodoroLongBreakMin() const { return m_pomodoroLongBreakMin; }
    void setPomodoroLongBreakMin(int val);

    int agentServerPort() const { return m_agentServerPort; }
    void setAgentServerPort(int val);

    QString agentServerToken() const { return m_agentServerToken; }
    void setAgentServerToken(const QString &val);

    QString idleWidget() const { return m_idleWidget; }
    void setIdleWidget(const QString &val);

    bool autoPaste() const { return m_autoPaste; }
    void setAutoPaste(bool val);

    bool pasteStackEnabled() const { return m_pasteStackEnabled; }
    void setPasteStackEnabled(bool val);

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();

signals:
    void displayNameChanged();
    void greetingChanged();
    void onboardingCompletedChanged();
    void accentColorChanged();
    void hotkeyChanged();
    void clipboardEnabledChanged();
    void autoStartChanged();
    void mascotEyesEnabledChanged();
    void islandStyleChanged();
    void reduceAnimationsChanged();
    void homeChipsChanged();
    void alertConfigChanged();
    void pollIntervalMinutesChanged();
    void pomodoroWorkMinChanged();
    void pomodoroShortBreakMinChanged();
    void pomodoroLongBreakMinChanged();
    void agentServerPortChanged();
    void agentServerTokenChanged();
    void idleWidgetChanged();
    void autoPasteChanged();
    void pasteStackEnabledChanged();

private:
    explicit Settings(QObject *parent = nullptr);
    void updateAutoStartRegistry(bool enable);
    bool checkAutoStartRegistry() const;

    QString m_displayName;
    bool m_onboardingCompleted = false;
    QString m_accentColor = "#0A84FF";
    QString m_hotkey = "Ctrl+Alt+Space";
    bool m_clipboardEnabled = true;
    bool m_autoStart = false;
    bool m_mascotEyesEnabled = true;
    QString m_islandStyle = "notch";
    bool m_reduceAnimations = false;
    QStringList m_homeChips = { "tray", "clips", "mail", "next" };

    bool m_alertBattery = true;
    bool m_alertScreenshots = true;
    bool m_alertDownloads = true;
    bool m_alertTimer = true;
    bool m_alertGmail = true;
    bool m_alertAgent = true;
    bool m_alertUsb = true;
    bool m_alertVolume = true;

    int m_pollIntervalMinutes = 2;
    int m_pomodoroWorkMin = 25;
    int m_pomodoroShortBreakMin = 5;
    int m_pomodoroLongBreakMin = 15;
    int m_agentServerPort = 19827;
    QString m_agentServerToken;
    QString m_idleWidget = "clipboard";
    bool m_autoPaste = false;
    bool m_pasteStackEnabled = false;
};

} // namespace Aura
