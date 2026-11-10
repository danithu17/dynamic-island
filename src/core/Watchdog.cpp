#include "Watchdog.h"
#include "Logger.h"
#include "SingleInstance.h"
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

bool Watchdog::shouldBypassWatchdog(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--selftest" || arg == "--uninstall-cleanup" || arg == "/silent" || arg == "--help") {
            return true;
        }
    }
    return false;
}

int Watchdog::runSupervisor(int argc, char *argv[]) {
    QStringList args;
    for (int i = 1; i < argc; ++i) {
        args << QString::fromLocal8Bit(argv[i]);
    }

    // 1. Single instance check before starting supervisor loop
    if (!SingleInstance::instance().checkAndAcquire(args)) {
        // Notified existing instance and safe to exit
        return 0;
    }
    // Release mutex so the worker child process can hold the official single-instance mutex
    SingleInstance::instance().release();

    QCoreApplication app(argc, argv);
    app.setApplicationName("AuraIsland");
    app.setOrganizationName("Aura");

    QString appPath = QCoreApplication::applicationFilePath();
    QStringList workerArgs = args;
    workerArgs << "--worker";

    QQueue<qint64> restartTimestamps;
    constexpr int MAX_RESTARTS_PER_MINUTE = 3;
    constexpr qint64 ONE_MINUTE_MS = 60 * 1000;

    Logger::log(QtInfoMsg, "[Watchdog] Aura Island supervisor started. Monitoring worker process...");

    while (true) {
        QProcess worker;
        worker.setProcessChannelMode(QProcess::ForwardedChannels);
        worker.start(appPath, workerArgs);

        if (!worker.waitForStarted(5000)) {
            std::cerr << "[Watchdog] Failed to start worker process!" << std::endl;
            return 1;
        }

        // Section A.8: Supervisor must never start a second window while the first is alive
        worker.waitForFinished(-1);

        int exitCode = worker.exitCode();
        QProcess::ExitStatus exitStatus = worker.exitStatus();

        // Normal exit requested by user
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            Logger::log(QtInfoMsg, "[Watchdog] Worker process exited cleanly.");
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
            Logger::log(QtCriticalMsg, QString("[Watchdog] Max restart limit reached (%1/min). Halting.").arg(MAX_RESTARTS_PER_MINUTE));
            return exitCode != 0 ? exitCode : 1;
        }

        Logger::log(QtWarningMsg, QString("[Watchdog] Worker crashed with exit code %1. Restarting in 500ms... (%2/%3 this minute)")
                                      .arg(exitCode)
                                      .arg(restartTimestamps.size())
                                      .arg(MAX_RESTARTS_PER_MINUTE));
        QThread::msleep(500);
    }
}

} // namespace Aura
