#include "Logger.h"
#include "Paths.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <iostream>
#include <DbgHelp.h>

typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

typedef LONG(WINAPI *RTLGETVERSION)(PRTL_OSVERSIONINFOW);

namespace Aura {

QFile Logger::s_logFile;
QTextStream Logger::s_stream;
QMutex Logger::s_mutex;
bool Logger::s_initialized = false;
QString Logger::s_cachedLogPath;

static void messageOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    Logger::log(type, msg);
}

void Logger::initEarly() {
    QMutexLocker locker(&s_mutex);
    if (s_initialized) return;

    // 1. Install crash handler
    SetUnhandledExceptionFilter(unhandledCrashHandler);

    // 2. Resolve log path
    s_cachedLogPath = Paths::startupLogPath();
    rotateLogsIfNeeded();

    s_logFile.setFileName(s_cachedLogPath);
    if (s_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        s_stream.setDevice(&s_logFile);
        s_initialized = true;
    }
}

void Logger::initQtHandler() {
    qInstallMessageHandler(messageOutputHandler);
}

void Logger::cleanup() {
    QMutexLocker locker(&s_mutex);
    if (!s_initialized) return;
    s_stream.flush();
    s_logFile.close();
    s_initialized = false;
    qInstallMessageHandler(nullptr);
}

QString Logger::logFilePath() {
    if (s_cachedLogPath.isEmpty()) {
        s_cachedLogPath = Paths::startupLogPath();
    }
    return s_cachedLogPath;
}

void Logger::rotateLogsIfNeeded() {
    QString mainPath = s_cachedLogPath.isEmpty() ? Paths::startupLogPath() : s_cachedLogPath;
    QFileInfo fi(mainPath);

    constexpr qint64 maxSizeBytes = 1 * 1024 * 1024; // 1 MB
    if (fi.exists() && fi.size() > maxSizeBytes) {
        if (s_logFile.isOpen()) {
            s_stream.flush();
            s_logFile.close();
        }

        QDir dir(fi.dir());
        // Rotate up to 5 logs: 4 -> 5, 3 -> 4, 2 -> 3, 1 -> 2, current -> 1
        QString log5 = dir.filePath("startup.5.log");
        if (QFile::exists(log5)) QFile::remove(log5);

        for (int i = 4; i >= 1; --i) {
            QString src = dir.filePath(QString("startup.%1.log").arg(i));
            QString dst = dir.filePath(QString("startup.%1.log").arg(i + 1));
            if (QFile::exists(src)) {
                QFile::rename(src, dst);
            }
        }
        QFile::rename(mainPath, dir.filePath("startup.1.log"));
    }
}

void Logger::writeRawLog(const QString &formattedLine) {
    std::cout << formattedLine.toLocal8Bit().constData() << std::endl;
    if (s_initialized && s_logFile.isOpen()) {
        s_stream << formattedLine << "\n";
        s_stream.flush();
    }
}

void Logger::log(QtMsgType type, const QString &msg) {
    QMutexLocker locker(&s_mutex);

    const char *levelStr = "[INFO ]";
    switch (type) {
    case QtDebugMsg:    levelStr = "[DEBUG]"; break;
    case QtInfoMsg:     levelStr = "[INFO ]"; break;
    case QtWarningMsg:  levelStr = "[WARN ]"; break;
    case QtCriticalMsg: levelStr = "[ERROR]"; break;
    case QtFatalMsg:    levelStr = "[FATAL]"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString formatted = QString("%1 %2 %3").arg(timestamp, levelStr, msg);
    writeRawLog(formatted);
}

void Logger::logStep(const QString &stepName) {
    log(QtInfoMsg, QString(">>> [STARTUP STEP] %1").arg(stepName));
}

static BOOL CALLBACK monitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    Q_UNUSED(hdcMonitor);
    auto *logList = reinterpret_cast<QStringList*>(dwData);
    if (!lprcMonitor || !logList) return TRUE;

    MONITORINFOEXW mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(hMonitor, &mi)) {
        UINT dpiX = 96, dpiY = 96;
        HMODULE hShcore = GetModuleHandleW(L"SHCore.dll");
        if (hShcore) {
            typedef HRESULT(WINAPI *GETDPI)(HMONITOR, int, UINT*, UINT*);
            auto pGetDpi = (GETDPI)GetProcAddress(hShcore, "GetDpiForMonitor");
            if (pGetDpi) {
                pGetDpi(hMonitor, 0, &dpiX, &dpiY);
            }
        }
        QString monStr = QString("Monitor '%1'%2: Rect=[%3, %4, %5, %6], DPI=%7x%8")
            .arg(QString::fromWCharArray(mi.szDevice))
            .arg((mi.dwFlags & MONITORINFOF_PRIMARY) ? " (PRIMARY)" : "")
            .arg(mi.rcMonitor.left).arg(mi.rcMonitor.top)
            .arg(mi.rcMonitor.right).arg(mi.rcMonitor.bottom)
            .arg(dpiX).arg(dpiY);
        logList->append(monStr);
    }
    return TRUE;
}

void Logger::logDiagnosticInfo(const QString &versionStr) {
    logStep("Collecting environment & diagnostic info");

    // 1. Version
    log(QtInfoMsg, QString("Aura Island Version: %1").arg(versionStr));

    // 2. OS Build
    QString osString = "Windows (Unknown build)";
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        auto pRtlGetVersion = (RTLGETVERSION)GetProcAddress(hNtdll, "RtlGetVersion");
        if (pRtlGetVersion) {
            RTL_OSVERSIONINFOW ovi = { 0 };
            ovi.dwOSVersionInfoSize = sizeof(ovi);
            if (pRtlGetVersion(&ovi) == 0) {
                osString = QString("Windows %1.%2 (Build %3)")
                    .arg(ovi.dwMajorVersion)
                    .arg(ovi.dwMinorVersion)
                    .arg(ovi.dwBuildNumber);
            }
        }
    }
    log(QtInfoMsg, QString("OS: %1").arg(osString));

    // 3. System DPI
    UINT sysDpi = 96;
    HMODULE hUser32 = GetModuleHandleW(L"user32.dll");
    if (hUser32) {
        typedef UINT(WINAPI *GETSYS_DPI)();
        auto pGetDpi = (GETSYS_DPI)GetProcAddress(hUser32, "GetDpiForSystem");
        if (pGetDpi) {
            sysDpi = pGetDpi();
        }
    }
    log(QtInfoMsg, QString("System DPI: %1 (scale %2%)").arg(sysDpi).arg(sysDpi * 100 / 96));

    // 4. Monitors
    QStringList monList;
    EnumDisplayMonitors(NULL, NULL, monitorEnumProc, reinterpret_cast<LPARAM>(&monList));
    log(QtInfoMsg, QString("Detected %1 display monitor(s):").arg(monList.size()));
    for (const QString &m : monList) {
        log(QtInfoMsg, QString("  * %1").arg(m));
    }
}

LONG WINAPI Logger::unhandledCrashHandler(EXCEPTION_POINTERS *exceptionInfo) {
    QString crashDir = Paths::crashDumpDir();
    QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString dumpPath = QString("%1/crash_%2.dmp").arg(crashDir, timeStr);

    std::wstring dumpPathW = dumpPath.toStdWString();
    HANDLE hFile = CreateFileW(dumpPathW.c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    bool dumpWritten = false;
    if (hFile != INVALID_HANDLE_VALUE) {
        HMODULE hDbgHelp = LoadLibraryW(L"dbghelp.dll");
        if (hDbgHelp) {
            auto pDump = (MINIDUMPWRITEDUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
            if (pDump) {
                MINIDUMP_EXCEPTION_INFORMATION mei;
                mei.ThreadId = GetCurrentThreadId();
                mei.ExceptionPointers = exceptionInfo;
                mei.ClientPointers = FALSE;

                dumpWritten = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                                    MiniDumpWithProcessThreadData, &mei, NULL, NULL);
            }
            FreeLibrary(hDbgHelp);
        }
        CloseHandle(hFile);
    }

    QString logPath = Logger::logFilePath();
    QString errorMsg = QString("Aura Island encountered a fatal unhandled exception (Code: 0x%1).\n\n"
                               "%2"
                               "Diagnostic logs saved to:\n%3")
        .arg(exceptionInfo ? QString::number(exceptionInfo->ExceptionRecord->ExceptionCode, 16).toUpper() : "UNKNOWN")
        .arg(dumpWritten ? QString("Crash minidump created at:\n%1\n\n").arg(dumpPath) : "Could not create crash dump file.\n\n")
        .arg(logPath);

    std::cerr << errorMsg.toLocal8Bit().constData() << std::endl;

    // Show native message box to user
    MessageBoxW(NULL, errorMsg.toStdWString().c_str(), L"Aura Island - Critical Crash", MB_ICONERROR | MB_OK);

    return EXCEPTION_EXECUTE_HANDLER;
}

} // namespace Aura
