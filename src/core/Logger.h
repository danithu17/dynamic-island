#pragma once

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>

namespace Aura {

class Logger {
public:
    static void init();
    static void cleanup();
    static void log(QtMsgType type, const QString &msg);
    static QString logFilePath();

private:
    static void rotateLogsIfNeeded();

    static QFile s_logFile;
    static QTextStream s_stream;
    static QMutex s_mutex;
    static bool s_initialized;
};

} // namespace Aura
