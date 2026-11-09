#include "SystemControlManager.h"
#include "../../core/AppState.h"
#include <QDebug>
#include <QProcess>
#include <algorithm>
#include <cmath>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <highlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <powrprof.h>

namespace Aura {

SystemControlManager &SystemControlManager::instance() {
    static SystemControlManager s_instance;
    return s_instance;
}

SystemControlManager::SystemControlManager(QObject *parent)
    : QObject(parent), m_pollTimer(new QTimer(this)) {
    initAudio();
    updateVolumeFromSystem();
    updateBrightnessFromSystem();
    updateTouchpadFromSystem();

    connect(m_pollTimer, &QTimer::timeout, this, &SystemControlManager::pollSystemSettings);
    m_pollTimer->setInterval(2000); // Check every 2s for external changes
    m_pollTimer->start();
}

SystemControlManager::~SystemControlManager() {
    if (m_endpointVolume) {
        auto *pVol = static_cast<IAudioEndpointVolume *>(m_endpointVolume);
        pVol->Release();
        m_endpointVolume = nullptr;
    }
}

void SystemControlManager::initAudio() {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    (void)hr;

    IMMDeviceEnumerator *deviceEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER,
                          __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
    if (SUCCEEDED(hr) && deviceEnumerator) {
        IMMDevice *defaultDevice = nullptr;
        hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &defaultDevice);
        if (SUCCEEDED(hr) && defaultDevice) {
            IAudioEndpointVolume *endpointVolume = nullptr;
            hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER,
                                         nullptr, (LPVOID *)&endpointVolume);
            if (SUCCEEDED(hr) && endpointVolume) {
                m_endpointVolume = endpointVolume;
            }
            defaultDevice->Release();
        }
        deviceEnumerator->Release();
    }
}

void SystemControlManager::updateVolumeFromSystem() {
    if (!m_endpointVolume) {
        initAudio();
        if (!m_endpointVolume) return;
    }

    auto *pVol = static_cast<IAudioEndpointVolume *>(m_endpointVolume);
    float currentVol = 0.5f;
    BOOL isMuted = FALSE;

    if (SUCCEEDED(pVol->GetMasterVolumeLevelScalar(&currentVol))) {
        int volInt = static_cast<int>(std::round(currentVol * 100.0f));
        if (m_volume != volInt) {
            m_volume = volInt;
            emit volumeChanged();
        }
    }

    if (SUCCEEDED(pVol->GetMute(&isMuted))) {
        bool muteBool = (isMuted != FALSE);
        if (m_isMuted != muteBool) {
            m_isMuted = muteBool;
            emit muteChanged();
        }
    }
}

void SystemControlManager::setVolume(int level) {
    level = std::clamp(level, 0, 100);
    if (!m_endpointVolume) initAudio();

    if (m_endpointVolume) {
        auto *pVol = static_cast<IAudioEndpointVolume *>(m_endpointVolume);
        float scalar = static_cast<float>(level) / 100.0f;
        pVol->SetMasterVolumeLevelScalar(scalar, nullptr);
        if (m_isMuted && level > 0) {
            pVol->SetMute(FALSE, nullptr);
            m_isMuted = false;
            emit muteChanged();
        }
    }

    if (m_volume != level) {
        m_volume = level;
        emit volumeChanged();
        AppState::instance().triggerEventCompact(QStringLiteral("Volume %1%").arg(level), 1800);
    }
}

void SystemControlManager::toggleMute() {
    setMuted(!m_isMuted);
}

void SystemControlManager::setMuted(bool mute) {
    if (!m_endpointVolume) initAudio();

    if (m_endpointVolume) {
        auto *pVol = static_cast<IAudioEndpointVolume *>(m_endpointVolume);
        pVol->SetMute(mute ? TRUE : FALSE, nullptr);
    }

    m_isMuted = mute;
    emit muteChanged();
    AppState::instance().triggerEventCompact(mute ? QStringLiteral("Muted") : QStringLiteral("Unmuted"), 1800);
}

void SystemControlManager::updateBrightnessFromSystem() {
    HMONITOR hMon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    DWORD numMonitors = 0;
    if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMon, &numMonitors) && numMonitors > 0) {
        std::vector<PHYSICAL_MONITOR> monitors(numMonitors);
        if (GetPhysicalMonitorsFromHMONITOR(hMon, numMonitors, monitors.data())) {
            DWORD minB = 0, curB = 50, maxB = 100;
            if (GetMonitorBrightness(monitors[0].hPhysicalMonitor, &minB, &curB, &maxB)) {
                if (maxB > minB) {
                    int b = static_cast<int>((curB - minB) * 100 / (maxB - minB));
                    if (m_brightness != b) {
                        m_brightness = b;
                        emit brightnessChanged();
                    }
                }
            }
            DestroyPhysicalMonitors(numMonitors, monitors.data());
        }
    }
}

void SystemControlManager::setBrightness(int level) {
    level = std::clamp(level, 0, 100);
    m_brightness = level;
    emit brightnessChanged();

    // 1. Hardware Dxva2 Set
    bool dxvaSuccess = false;
    HMONITOR hMon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
    DWORD numMonitors = 0;
    if (GetNumberOfPhysicalMonitorsFromHMONITOR(hMon, &numMonitors) && numMonitors > 0) {
        std::vector<PHYSICAL_MONITOR> monitors(numMonitors);
        if (GetPhysicalMonitorsFromHMONITOR(hMon, numMonitors, monitors.data())) {
            DWORD minB = 0, curB = 50, maxB = 100;
            if (GetMonitorBrightness(monitors[0].hPhysicalMonitor, &minB, &curB, &maxB)) {
                DWORD target = minB + ((maxB - minB) * level) / 100;
                dxvaSuccess = (SetMonitorBrightness(monitors[0].hPhysicalMonitor, target) != FALSE);
            }
            DestroyPhysicalMonitors(numMonitors, monitors.data());
        }
    }

    // 2. Laptop WMI Fallback if Dxva2 is not supported by monitor driver
    if (!dxvaSuccess) {
        QString cmd = QStringLiteral("(Get-WmiObject -Namespace root/wmi -Class WmiMonitorBrightnessMethods).WmiSetBrightness(1, %1)").arg(level);
        QProcess::startDetached(QStringLiteral("powershell.exe"), {QStringLiteral("-NoProfile"), QStringLiteral("-Command"), cmd});
    }

    AppState::instance().triggerEventCompact(QStringLiteral("Brightness %1%").arg(level), 1800);
}

void SystemControlManager::updateTouchpadFromSystem() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\PrecisionTouchPad\\Status", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD val = 1;
        DWORD size = sizeof(val);
        if (RegQueryValueExW(hKey, L"Enabled", nullptr, nullptr, (LPBYTE)&val, &size) == ERROR_SUCCESS) {
            bool enabled = (val != 0);
            if (m_isTouchpadEnabled != enabled) {
                m_isTouchpadEnabled = enabled;
                emit touchpadChanged();
            }
        }
        RegCloseKey(hKey);
    }
}

void SystemControlManager::toggleTouchpad() {
    m_isTouchpadEnabled = !m_isTouchpadEnabled;
    emit touchpadChanged();

    HKEY hKey;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\PrecisionTouchPad\\Status",
                        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
        DWORD val = m_isTouchpadEnabled ? 1 : 0;
        RegSetValueExW(hKey, L"Enabled", 0, REG_DWORD, (const BYTE*)&val, sizeof(val));
        RegCloseKey(hKey);
    }

    // Notify Windows settings
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)L"Windows.System.Settings");

    AppState::instance().triggerEventCompact(m_isTouchpadEnabled ? QStringLiteral("Touchpad Enabled") : QStringLiteral("Touchpad Disabled"), 2000);
}

void SystemControlManager::lockScreen() {
    AppState::instance().triggerEventCompact(QStringLiteral("Locking PC..."), 1500);
    QTimer::singleShot(500, []() {
        LockWorkStation();
    });
}

void SystemControlManager::sleepPC() {
    AppState::instance().triggerEventCompact(QStringLiteral("Entering Sleep..."), 1500);
    QTimer::singleShot(800, []() {
        SetSuspendState(FALSE, FALSE, FALSE);
    });
}

void SystemControlManager::restartPC() {
    AppState::instance().triggerEventCompact(QStringLiteral("Restarting PC..."), 2000);
    QTimer::singleShot(1000, []() {
        QProcess::startDetached(QStringLiteral("shutdown.exe"), {QStringLiteral("/r"), QStringLiteral("/t"), QStringLiteral("0")});
    });
}

void SystemControlManager::shutdownPC() {
    AppState::instance().triggerEventCompact(QStringLiteral("Shutting Down PC..."), 2000);
    QTimer::singleShot(1000, []() {
        QProcess::startDetached(QStringLiteral("shutdown.exe"), {QStringLiteral("/s"), QStringLiteral("/t"), QStringLiteral("0")});
    });
}

void SystemControlManager::refreshAll() {
    updateVolumeFromSystem();
    updateBrightnessFromSystem();
    updateTouchpadFromSystem();
}

void SystemControlManager::pollSystemSettings() {
    updateVolumeFromSystem();
}

} // namespace Aura
