#include "Watchdog.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QProcess>
#include <QDateTime>
#include <QQueue>
#include <QThread>
#include <iostream>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Aura {

bool Watchdog::isWorkerProcess(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--worker") {
            return true;
        }
    }
    return false;
}

int Watchdog::runSupervisor(int argc, char *argv[]) {
    HANDLE hMutex = CreateMutexW(NULL, FALSE, L"Local\\AuraIsland_Supervisor_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "[Watchdog] Aura Island is already running. Signaling window to show..." << std::endl;
        UINT msg = RegisterWindowMessageW(L"WM_AURA_ISLAND_SHOW");
        if (msg != 0) {
            PostMessageW(HWND_BROADCAST, msg, 0, 0);
        }
        if (hMutex) CloseHandle(hMutex);
        return 0;
    }

    QCoreApplication app(argc, argv);
    app.setApplicationName("AuraIsland");
    app.setOrganizationName("Aura");
    Aura::Logger::init();

    QString appPath = QCoreApplication::applicationFilePath();
    QStringList workerArgs;
    for (int i = 1; i < argc; ++i) {
        workerArgs << QString::fromLocal8Bit(argv[i]);
    }
    workerArgs << "--worker";

    QQueue<qint64> restartTimestamps;
    constexpr int MAX_RESTARTS_PER_MINUTE = 3;
    constexpr qint64 ONE_MINUTE_MS = 60 * 1000;

    std::cout << "[Watchdog] Aura Island supervisor started. Monitoring worker..." << std::endl;

    while (true) {
        QProcess worker;
        worker.setProcessChannelMode(QProcess::ForwardedChannels);
        worker.start(appPath, workerArgs);

        if (!worker.waitForStarted(5000)) {
            std::cerr << "[Watchdog] Failed to start worker process!" << std::endl;
            return 1;
        }

        worker.waitForFinished(-1);

        int exitCode = worker.exitCode();
        QProcess::ExitStatus exitStatus = worker.exitStatus();

        // Normal exit requested by user
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            std::cout << "[Watchdog] Worker exited cleanly." << std::endl;
            return 0;
        }

        // Abnormal termination / crash detected
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        restartTimestamps.enqueue(now);

        // Prune older than 1 minute
        while (!restartTimestamps.isEmpty() && (now - restartTimestamps.head()) > ONE_MINUTE_MS) {
            restartTimestamps.dequeue();
        }

        if (restartTimestamps.size() > MAX_RESTARTS_PER_MINUTE) {
            std::cerr << "[Watchdog] Too many crashes (" << restartTimestamps.size()
                      << " in 1 minute). Halting restarts to protect system." << std::endl;
            return exitCode != 0 ? exitCode : 1;
        }

        std::cout << "[Watchdog] Worker crashed with exit code " << exitCode
                  << ". Restarting in 500ms... (" << restartTimestamps.size()
                  << "/" << MAX_RESTARTS_PER_MINUTE << " this minute)" << std::endl;
        QThread::msleep(500);
    }
}

} // namespace Aura
