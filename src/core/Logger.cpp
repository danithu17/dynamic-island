#include "Logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <iostream>

namespace Aura {

QFile Logger::s_logFile;
QTextStream Logger::s_stream;
QMutex Logger::s_mutex;
bool Logger::s_initialized = false;

static void messageOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    Logger::log(type, msg);
}

void Logger::init() {
    QMutexLocker locker(&s_mutex);
    if (s_initialized) return;

    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir logDir(appData + "/logs");
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }

    QString mainPath = logDir.filePath("aura_island.log");
    rotateLogsIfNeeded();

    s_logFile.setFileName(mainPath);
    if (s_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        s_stream.setDevice(&s_logFile);
        s_initialized = true;
    }

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
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appData + "/logs/aura_island.log";
}

void Logger::rotateLogsIfNeeded() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir logDir(appData + "/logs");
    QString mainPath = logDir.filePath("aura_island.log");
    QFileInfo fi(mainPath);

    constexpr qint64 maxSizeBytes = 5 * 1024 * 1024; // 5 MB
    if (fi.exists() && fi.size() > maxSizeBytes) {
        if (s_logFile.isOpen()) {
            s_stream.flush();
            s_logFile.close();
        }

        // Shift old logs: log.3 -> deleted, log.2 -> log.3, log.1 -> log.2, current -> log.1
        QString log3 = logDir.filePath("aura_island.3.log");
        QString log2 = logDir.filePath("aura_island.2.log");
        QString log1 = logDir.filePath("aura_island.1.log");

        if (QFile::exists(log3)) QFile::remove(log3);
        if (QFile::exists(log2)) QFile::rename(log2, log3);
        if (QFile::exists(log1)) QFile::rename(log1, log2);
        QFile::rename(mainPath, log1);
    }
}

void Logger::log(QtMsgType type, const QString &msg) {
    QMutexLocker locker(&s_mutex);

    QString levelStr;
    switch (type) {
    case QtDebugMsg:    levelStr = "[DEBUG]"; break;
    case QtInfoMsg:     levelStr = "[INFO ]"; break;
    case QtWarningMsg:  levelStr = "[WARN ]"; break;
    case QtCriticalMsg: levelStr = "[ERROR]"; break;
    case QtFatalMsg:    levelStr = "[FATAL]"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString formatted = QString("%1 %2 %3").arg(timestamp, levelStr, msg);

    // Output to stdout / stderr for console or debugger
    std::cout << formatted.toStdString() << std::endl;

    if (s_initialized && s_logFile.isOpen()) {
        s_stream << formatted << "\n";
        s_stream.flush();
    }
}

} // namespace Aura
