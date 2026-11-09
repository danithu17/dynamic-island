#pragma once

#include <QObject>
#include <QTimer>
#include <QString>

namespace Aura {

class PomodoroTimer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString mode READ mode NOTIFY stateChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY stateChanged)
    Q_PROPERTY(int secondsRemaining READ secondsRemaining NOTIFY tick)
    Q_PROPERTY(QString timeDisplay READ timeDisplay NOTIFY tick)
    Q_PROPERTY(int completedCycles READ completedCycles NOTIFY stateChanged)

public:
    explicit PomodoroTimer(QObject *parent = nullptr);

    QString mode() const { return m_mode; } // "work", "shortBreak", "longBreak"
    bool isRunning() const { return m_isRunning; }
    int secondsRemaining() const { return m_secondsRemaining; }
    QString timeDisplay() const;
    int completedCycles() const { return m_completedCycles; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();
    Q_INVOKABLE void skipToNext();

signals:
    void tick();
    void stateChanged();
    void cycleCompleted(const QString &mode);

private slots:
    void onOneSecondTick();

private:
    void applyModeDuration();

    QString m_mode = "work";
    bool m_isRunning = false;
    int m_secondsRemaining = 25 * 60;
    int m_completedCycles = 0;
    QTimer *m_timer = nullptr;
};

} // namespace Aura
