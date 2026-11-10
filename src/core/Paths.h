#pragma once

#include <QString>

namespace Aura {

class Paths {
public:
    // Base data directory: %APPDATA%\AuraIsland
    static QString appDataDir();

    // Specific sub-paths
    static QString logsDir();
    static QString startupLogPath();
    static QString crashDumpDir();
    static QString databasePath();
    static QString settingsJsonPath();

    // Ensure directory exists
    static bool ensureDirExists(const QString &dirPath);
};

} // namespace Aura
