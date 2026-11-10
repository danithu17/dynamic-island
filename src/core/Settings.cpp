#include "Settings.h"
#include "DatabaseManager.h"
#include "Paths.h"
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QUuid>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QTime>
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

QString Settings::greeting() const {
    int hour = QTime::currentTime().hour();
    QString timeGreeting;
    if (hour >= 5 && hour < 12) {
        timeGreeting = "Good morning";
    } else if (hour >= 12 && hour < 17) {
        timeGreeting = "Good afternoon";
    } else {
        timeGreeting = "Good evening";
    }

    if (!m_displayName.trimmed().isEmpty()) {
        return QString("%1, %2").arg(timeGreeting, m_displayName.trimmed());
    }
    return timeGreeting;
}

void Settings::setDisplayName(const QString &val) {
    if (m_displayName != val) {
        m_displayName = val;
        emit displayNameChanged();
        emit greetingChanged();
        save();
    }
}

void Settings::setOnboardingCompleted(bool val) {
    if (m_onboardingCompleted != val) {
        m_onboardingCompleted = val;
        emit onboardingCompletedChanged();
        save();
    }
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

void Settings::setMascotEyesEnabled(bool val) {
    if (m_mascotEyesEnabled != val) {
        m_mascotEyesEnabled = val;
        emit mascotEyesEnabledChanged();
        save();
    }
}

void Settings::setIslandStyle(const QString &val) {
    if (m_islandStyle != val) {
        m_islandStyle = val;
        emit islandStyleChanged();
        save();
    }
}

void Settings::setReduceAnimations(bool val) {
    if (m_reduceAnimations != val) {
        m_reduceAnimations = val;
        emit reduceAnimationsChanged();
        save();
    }
}

void Settings::setHomeChips(const QStringList &chips) {
    if (m_homeChips != chips) {
        m_homeChips = chips;
        emit homeChipsChanged();
        save();
    }
}

bool Settings::isHomeChipActive(const QString &chipId) const {
    return m_homeChips.contains(chipId);
}

void Settings::toggleHomeChip(const QString &chipId) {
    QStringList updated = m_homeChips;
    if (updated.contains(chipId)) {
        if (updated.size() > 1) { // keep at least 1
            updated.removeAll(chipId);
        }
    } else {
        if (updated.size() < 4) { // maximum 4 chips
            updated.append(chipId);
        }
    }
    setHomeChips(updated);
}

void Settings::setAlertBattery(bool val) {
    if (m_alertBattery != val) {
        m_alertBattery = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertScreenshots(bool val) {
    if (m_alertScreenshots != val) {
        m_alertScreenshots = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertDownloads(bool val) {
    if (m_alertDownloads != val) {
        m_alertDownloads = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertTimer(bool val) {
    if (m_alertTimer != val) {
        m_alertTimer = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertGmail(bool val) {
    if (m_alertGmail != val) {
        m_alertGmail = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertAgent(bool val) {
    if (m_alertAgent != val) {
        m_alertAgent = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertUsb(bool val) {
    if (m_alertUsb != val) {
        m_alertUsb = val;
        emit alertConfigChanged();
        save();
    }
}

void Settings::setAlertVolume(bool val) {
    if (m_alertVolume != val) {
        m_alertVolume = val;
        emit alertConfigChanged();
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

void Settings::setIdleWidget(const QString &val) {
    if (m_idleWidget != val) {
        m_idleWidget = val;
        emit idleWidgetChanged();
        save();
    }
}

void Settings::setAutoPaste(bool val) {
    if (m_autoPaste != val) {
        m_autoPaste = val;
        emit autoPasteChanged();
        save();
    }
}

void Settings::setPasteStackEnabled(bool val) {
    if (m_pasteStackEnabled != val) {
        m_pasteStackEnabled = val;
        emit pasteStackEnabledChanged();
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
    // 1. Save to %APPDATA%\AuraIsland\settings.json
    QString jsonPath = Paths::settingsJsonPath();
    QJsonObject rootObj;
    rootObj["displayName"] = m_displayName;
    rootObj["onboardingCompleted"] = m_onboardingCompleted;
    rootObj["accentColor"] = m_accentColor;
    rootObj["hotkey"] = m_hotkey;
    rootObj["clipboardEnabled"] = m_clipboardEnabled;
    rootObj["autoStart"] = m_autoStart;
    rootObj["mascotEyesEnabled"] = m_mascotEyesEnabled;
    rootObj["islandStyle"] = m_islandStyle;
    rootObj["reduceAnimations"] = m_reduceAnimations;

    QJsonArray chipsArr;
    for (const QString &c : m_homeChips) {
        chipsArr.append(c);
    }
    rootObj["homeChips"] = chipsArr;

    QJsonObject alertsObj;
    alertsObj["battery"] = m_alertBattery;
    alertsObj["screenshots"] = m_alertScreenshots;
    alertsObj["downloads"] = m_alertDownloads;
    alertsObj["timer"] = m_alertTimer;
    alertsObj["gmail"] = m_alertGmail;
    alertsObj["agent"] = m_alertAgent;
    alertsObj["usb"] = m_alertUsb;
    alertsObj["volume"] = m_alertVolume;
    rootObj["alerts"] = alertsObj;

    QJsonObject pomodoroObj;
    pomodoroObj["workMin"] = m_pomodoroWorkMin;
    pomodoroObj["shortBreakMin"] = m_pomodoroShortBreakMin;
    pomodoroObj["longBreakMin"] = m_pomodoroLongBreakMin;
    rootObj["pomodoro"] = pomodoroObj;

    rootObj["agentServerPort"] = m_agentServerPort;
    rootObj["agentServerToken"] = m_agentServerToken;
    rootObj["idleWidget"] = m_idleWidget;
    rootObj["autoPaste"] = m_autoPaste;
    rootObj["pasteStackEnabled"] = m_pasteStackEnabled;
    rootObj["updatedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QFile file(jsonPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonDocument doc(rootObj);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }

    // 2. Also keep SQLite settings in sync if DB is open
    auto db = DatabaseManager::instance().database();
    if (db.isOpen()) {
        auto setVal = [&](const QString &k, const QString &v) {
            QSqlQuery q(db);
            q.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (:k, :v)");
            q.bindValue(":k", k);
            q.bindValue(":v", v);
            q.exec();
        };

        setVal("displayName", m_displayName);
        setVal("onboardingCompleted", m_onboardingCompleted ? "1" : "0");
        setVal("accentColor", m_accentColor);
        setVal("hotkey", m_hotkey);
        setVal("clipboardEnabled", m_clipboardEnabled ? "1" : "0");
        setVal("autoStart", m_autoStart ? "1" : "0");
        setVal("mascotEyesEnabled", m_mascotEyesEnabled ? "1" : "0");
        setVal("islandStyle", m_islandStyle);
        setVal("reduceAnimations", m_reduceAnimations ? "1" : "0");
        setVal("homeChips", m_homeChips.join(","));
    }
}

void Settings::load() {
    // 1. Try reading %APPDATA%\AuraIsland\settings.json
    QString jsonPath = Paths::settingsJsonPath();
    QFile file(jsonPath);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (doc.isObject()) {
            QJsonObject root = doc.object();
            if (root.contains("displayName")) m_displayName = root["displayName"].toString();
            if (root.contains("onboardingCompleted")) m_onboardingCompleted = root["onboardingCompleted"].toBool();
            if (root.contains("accentColor")) m_accentColor = root["accentColor"].toString();
            if (root.contains("hotkey")) m_hotkey = root["hotkey"].toString();
            if (root.contains("clipboardEnabled")) m_clipboardEnabled = root["clipboardEnabled"].toBool();
            if (root.contains("autoStart")) m_autoStart = root["autoStart"].toBool();
            if (root.contains("mascotEyesEnabled")) m_mascotEyesEnabled = root["mascotEyesEnabled"].toBool();
            if (root.contains("islandStyle")) m_islandStyle = root["islandStyle"].toString();
            if (root.contains("reduceAnimations")) m_reduceAnimations = root["reduceAnimations"].toBool();

            if (root.contains("homeChips") && root["homeChips"].isArray()) {
                m_homeChips.clear();
                for (const auto &v : root["homeChips"].toArray()) {
                    m_homeChips.append(v.toString());
                }
            }

            if (root.contains("alerts") && root["alerts"].isObject()) {
                QJsonObject al = root["alerts"].toObject();
                if (al.contains("battery")) m_alertBattery = al["battery"].toBool();
                if (al.contains("screenshots")) m_alertScreenshots = al["screenshots"].toBool();
                if (al.contains("downloads")) m_alertDownloads = al["downloads"].toBool();
                if (al.contains("timer")) m_alertTimer = al["timer"].toBool();
                if (al.contains("gmail")) m_alertGmail = al["gmail"].toBool();
                if (al.contains("agent")) m_alertAgent = al["agent"].toBool();
                if (al.contains("usb")) m_alertUsb = al["usb"].toBool();
                if (al.contains("volume")) m_alertVolume = al["volume"].toBool();
            }

            if (root.contains("pomodoro") && root["pomodoro"].isObject()) {
                QJsonObject po = root["pomodoro"].toObject();
                if (po.contains("workMin")) m_pomodoroWorkMin = po["workMin"].toInt();
                if (po.contains("shortBreakMin")) m_pomodoroShortBreakMin = po["shortBreakMin"].toInt();
                if (po.contains("longBreakMin")) m_pomodoroLongBreakMin = po["longBreakMin"].toInt();
            }

            if (root.contains("agentServerPort")) m_agentServerPort = root["agentServerPort"].toInt();
            if (root.contains("agentServerToken")) m_agentServerToken = root["agentServerToken"].toString();
            if (root.contains("idleWidget")) m_idleWidget = root["idleWidget"].toString();
            if (root.contains("autoPaste")) m_autoPaste = root["autoPaste"].toBool();
            if (root.contains("pasteStackEnabled")) m_pasteStackEnabled = root["pasteStackEnabled"].toBool();
            return;
        }
    }

    // 2. Fallback to SQLite settings table if JSON did not exist yet
    auto db = DatabaseManager::instance().database();
    if (!db.isOpen()) return;

    QSqlQuery q("SELECT key, value FROM settings", db);
    while (q.next()) {
        QString k = q.value(0).toString();
        QString v = q.value(1).toString();

        if (k == "displayName") m_displayName = v;
        else if (k == "onboardingCompleted") m_onboardingCompleted = (v == "1");
        else if (k == "accentColor") m_accentColor = v;
        else if (k == "hotkey") m_hotkey = v;
        else if (k == "clipboardEnabled") m_clipboardEnabled = (v == "1");
        else if (k == "autoStart") m_autoStart = (v == "1");
        else if (k == "mascotEyesEnabled") m_mascotEyesEnabled = (v == "1");
        else if (k == "pollIntervalMinutes") m_pollIntervalMinutes = v.toInt();
        else if (k == "pomodoroWorkMin") m_pomodoroWorkMin = v.toInt();
        else if (k == "pomodoroShortBreakMin") m_pomodoroShortBreakMin = v.toInt();
        else if (k == "pomodoroLongBreakMin") m_pomodoroLongBreakMin = v.toInt();
        else if (k == "agentServerPort") m_agentServerPort = v.toInt();
        else if (k == "agentServerToken") m_agentServerToken = v;
        else if (k == "idleWidget") m_idleWidget = v;
        else if (k == "autoPaste") m_autoPaste = (v == "1");
        else if (k == "pasteStackEnabled") m_pasteStackEnabled = (v == "1");
        else if (k == "islandStyle") m_islandStyle = v;
        else if (k == "reduceAnimations") m_reduceAnimations = (v == "1");
        else if (k == "homeChips") m_homeChips = v.split(",", Qt::SkipEmptyParts);
    }
}

} // namespace Aura
