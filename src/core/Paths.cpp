#include "Paths.h"
#include <QStandardPaths>
#include <QDir>
#include <cstdlib>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shlobj.h>

namespace Aura {

QString Paths::appDataDir() {
    // 1. Try SHGetKnownFolderPath for FOLDERID_RoamingAppData
    PWSTR path = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path))) {
        QString appData = QString::fromWCharArray(path);
        CoTaskMemFree(path);
        QString target = appData + "/AuraIsland";
        ensureDirExists(target);
        return target;
    }

    // 2. Fallback to APPDATA environment variable
    const char *envAppData = std::getenv("APPDATA");
    if (envAppData && envAppData[0] != '\0') {
        QString target = QString::fromLocal8Bit(envAppData) + "/AuraIsland";
        ensureDirExists(target);
        return target;
    }

    // 3. Fallback to QStandardPaths
    QString target = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/AuraIsland";
    ensureDirExists(target);
    return target;
}

QString Paths::logsDir() {
    QString dir = appDataDir() + "/logs";
    ensureDirExists(dir);
    return dir;
}

QString Paths::startupLogPath() {
    return logsDir() + "/startup.log";
}

QString Paths::crashDumpDir() {
    QString dir = appDataDir() + "/crash";
    ensureDirExists(dir);
    return dir;
}

QString Paths::databasePath() {
    return appDataDir() + "/aura_island.db";
}

QString Paths::settingsJsonPath() {
    return appDataDir() + "/settings.json";
}

bool Paths::ensureDirExists(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

} // namespace Aura
