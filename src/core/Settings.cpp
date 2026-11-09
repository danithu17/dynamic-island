#include "Settings.h"
#include "DatabaseManager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QUuid>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

namespace Aura {

static const char* REG_RUN_PATH = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char* APP_NAME = "AuraIsland";

Settings &Settings::instance() {
    static Settings s_instance;
    return s_instance;
}

Settings::Settings(QObject *parent)
    : QObject(parent) {
    m_agentServerToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
    load();
    m_autoStart = checkAutoStartRegistry();
}

void Settings::setAccentColor(const QString &val) {
    if (m_accentColor != val) {
        m_accentColor = val;
        emit accentColorChanged();
        save();
    }
}

void Settings::setHotkey(const QString &val) {
    if (m_hotkey != val) {
        m_hotkey = val;
        emit hotkeyChanged();
        save();
    }
}

void Settings::setClipboardEnabled(bool val) {
    if (m_clipboardEnabled != val) {
        m_clipboardEnabled = val;
        emit clipboardEnabledChanged();
        save();
    }
}

void Settings::setAutoStart(bool val) {
    if (m_autoStart != val) {
        m_autoStart = val;
        updateAutoStartRegistry(val);
        emit autoStartChanged();
        save();
    }
}

void Settings::setPollIntervalMinutes(int val) {
    if (m_pollIntervalMinutes != val) {
        m_pollIntervalMinutes = val;
        emit pollIntervalMinutesChanged();
        save();
    }
}

void Settings::setPomodoroWorkMin(int val) {
    if (m_pomodoroWorkMin != val) {
        m_pomodoroWorkMin = val;
        emit pomodoroWorkMinChanged();
        save();
    }
}

void Settings::setPomodoroShortBreakMin(int val) {
    if (m_pomodoroShortBreakMin != val) {
        m_pomodoroShortBreakMin = val;
        emit pomodoroShortBreakMinChanged();
        save();
    }
}

void Settings::setPomodoroLongBreakMin(int val) {
    if (m_pomodoroLongBreakMin != val) {
        m_pomodoroLongBreakMin = val;
        emit pomodoroLongBreakMinChanged();
        save();
    }
}

void Settings::setAgentServerPort(int val) {
    if (m_agentServerPort != val) {
        m_agentServerPort = val;
        emit agentServerPortChanged();
        save();
    }
}

void Settings::setAgentServerToken(const QString &val) {
    if (m_agentServerToken != val) {
        m_agentServerToken = val;
        emit agentServerTokenChanged();
        save();
    }
}

void Settings::updateAutoStartRegistry(bool enable) {
    QSettings runSettings(REG_RUN_PATH, QSettings::NativeFormat);
    if (enable) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        runSettings.setValue(APP_NAME, QString("\"%1\"").arg(appPath));
    } else {
        runSettings.remove(APP_NAME);
    }
}

bool Settings::checkAutoStartRegistry() const {
    QSettings runSettings(REG_RUN_PATH, QSettings::NativeFormat);
    return runSettings.contains(APP_NAME);
}

void Settings::save() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    auto setVal = [&](const QString &k, const QString &v) {
        QSqlQuery q(db);
        q.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:k, :v)");
        q.bindValue(":k", k);
        q.bindValue(":v", v);
        q.exec();
    };

    setVal("accentColor", m_accentColor);
    setVal("hotkey", m_hotkey);
    setVal("clipboardEnabled", m_clipboardEnabled ? "1" : "0");
    setVal("autoStart", m_autoStart ? "1" : "0");
    setVal("pollIntervalMinutes", QString::number(m_pollIntervalMinutes));
    setVal("pomodoroWorkMin", QString::number(m_pomodoroWorkMin));
    setVal("pomodoroShortBreakMin", QString::number(m_pomodoroShortBreakMin));
    setVal("pomodoroLongBreakMin", QString::number(m_pomodoroLongBreakMin));
    setVal("agentServerPort", QString::number(m_agentServerPort));
    setVal("agentServerToken", m_agentServerToken);
}

void Settings::load() {
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT key, value FROM settings", db);
    while (q.next()) {
        QString k = q.value(0).toString();
        QString v = q.value(1).toString();

        if (k == "accentColor") m_accentColor = v;
        else if (k == "hotkey") m_hotkey = v;
        else if (k == "clipboardEnabled") m_clipboardEnabled = (v == "1");
        else if (k == "autoStart") m_autoStart = (v == "1");
        else if (k == "pollIntervalMinutes") m_pollIntervalMinutes = v.toInt();
        else if (k == "pomodoroWorkMin") m_pomodoroWorkMin = v.toInt();
        else if (k == "pomodoroShortBreakMin") m_pomodoroShortBreakMin = v.toInt();
        else if (k == "pomodoroLongBreakMin") m_pomodoroLongBreakMin = v.toInt();
        else if (k == "agentServerPort") m_agentServerPort = v.toInt();
        else if (k == "agentServerToken") m_agentServerToken = v;
    }
}

} // namespace Aura
