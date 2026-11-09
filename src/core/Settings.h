#pragma once

#include <QObject>
#include <QString>

namespace Aura {

class Settings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString hotkey READ hotkey WRITE setHotkey NOTIFY hotkeyChanged)
    Q_PROPERTY(bool clipboardEnabled READ clipboardEnabled WRITE setClipboardEnabled NOTIFY clipboardEnabledChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(int pollIntervalMinutes READ pollIntervalMinutes WRITE setPollIntervalMinutes NOTIFY pollIntervalMinutesChanged)
    Q_PROPERTY(int pomodoroWorkMin READ pomodoroWorkMin WRITE setPomodoroWorkMin NOTIFY pomodoroWorkMinChanged)
    Q_PROPERTY(int pomodoroShortBreakMin READ pomodoroShortBreakMin WRITE setPomodoroShortBreakMin NOTIFY pomodoroShortBreakMinChanged)
    Q_PROPERTY(int pomodoroLongBreakMin READ pomodoroLongBreakMin WRITE setPomodoroLongBreakMin NOTIFY pomodoroLongBreakMinChanged)
    Q_PROPERTY(int agentServerPort READ agentServerPort WRITE setAgentServerPort NOTIFY agentServerPortChanged)
    Q_PROPERTY(QString agentServerToken READ agentServerToken WRITE setAgentServerToken NOTIFY agentServerTokenChanged)

public:
    static Settings &instance();

    QString accentColor() const { return m_accentColor; }
    void setAccentColor(const QString &val);

    QString hotkey() const { return m_hotkey; }
    void setHotkey(const QString &val);

    bool clipboardEnabled() const { return m_clipboardEnabled; }
    void setClipboardEnabled(bool val);

    bool autoStart() const { return m_autoStart; }
    void setAutoStart(bool val);

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

    Q_INVOKABLE void save();
    Q_INVOKABLE void load();

signals:
    void accentColorChanged();
    void hotkeyChanged();
    void clipboardEnabledChanged();
    void autoStartChanged();
    void pollIntervalMinutesChanged();
    void pomodoroWorkMinChanged();
    void pomodoroShortBreakMinChanged();
    void pomodoroLongBreakMinChanged();
    void agentServerPortChanged();
    void agentServerTokenChanged();

private:
    explicit Settings(QObject *parent = nullptr);
    void updateAutoStartRegistry(bool enable);
    bool checkAutoStartRegistry() const;

    QString m_accentColor = "#0A84FF";
    QString m_hotkey = "Ctrl+Alt+Space";
    bool m_clipboardEnabled = true;
    bool m_autoStart = false;
    int m_pollIntervalMinutes = 2;
    int m_pomodoroWorkMin = 25;
    int m_pomodoroShortBreakMin = 5;
    int m_pomodoroLongBreakMin = 15;
    int m_agentServerPort = 19827;
    QString m_agentServerToken;
};

} // namespace Aura
