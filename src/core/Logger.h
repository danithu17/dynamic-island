#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Aura {

class Logger {
public:
    static void initEarly();
    static void initQtHandler();
    static void cleanup();

    static void log(QtMsgType type, const QString &msg);
    static void logStep(const QString &stepName);
    static void logDiagnosticInfo(const QString &versionStr);

    static QString logFilePath();

    static LONG WINAPI unhandledCrashHandler(EXCEPTION_POINTERS *exceptionInfo);

private:
    static void rotateLogsIfNeeded();
    static void writeRawLog(const QString &formattedLine);

    static QFile s_logFile;
    static QTextStream s_stream;
    static QMutex s_mutex;
    static bool s_initialized;
    static QString s_cachedLogPath;
};

} // namespace Aura
