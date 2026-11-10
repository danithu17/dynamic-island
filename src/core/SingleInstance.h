#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Aura {

class SingleInstance : public QObject {
    Q_OBJECT

public:
    static SingleInstance &instance();

    // Checks if another instance is running.
    // If running, sends message to it and returns false.
    // If not running, acquires lock and returns true.
    bool checkAndAcquire(const QStringList &args);

    // Starts background named pipe server for listening to other instances
    void startServer();

    // Release handles
    void release();

signals:
    void wakeRequested(const QString &message);
    void showRequested();
    void setupRequested();

private:
    explicit SingleInstance(QObject *parent = nullptr);
    ~SingleInstance() override;

    HANDLE m_hMutex = nullptr;
    HANDLE m_hServerThread = nullptr;
    HANDLE m_hStopEvent = nullptr;
    bool m_isServerRunning = false;

    static DWORD WINAPI serverThreadProc(LPVOID param);
};

} // namespace Aura
