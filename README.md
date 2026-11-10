# 🏝️ Aura Island — Dynamic Island for Windows

<div align="center">

![Aura Island Icon](resources/icons/app-icon.svg)

**A native Windows Dynamic Island productivity hub designed with solid black notch aesthetics, fluid spring physics, and Apple-style interactive onboarding.**  
Built with **C++20**, **Qt 6.8+** (Qt Quick / QML), **Direct2D / GDI+**, and **Win32 DWM**.

[![Build Status](https://github.com/USER/aura-island/actions/workflows/build.yml/badge.svg)](https://github.com/USER/aura-island/actions/workflows/build.yml)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011%20(x64)-blue.svg)](#)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B)](#)
[![Qt](https://img.shields.io/badge/Qt-6.8%2B-41CD52?logo=qt)](#)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](#)

</div>

---

## 📦 Download & Installation

### Deliverable: `AuraIsland-Setup-<version>.exe`
The installer itself **is the pill**:
- **Pill Setup Experience**: A native Win32 window (`AuraSetupStub`) rendered as a solid black notch with spring animations (stiffness 250, damping 24), blinking mascot eyes, and zero external runtime dependencies (`/MT`).
- **No Early Admin Elevation**: Runs without admin rights on start (`asInvoker`). Elevation (`runas`) only prompts when you click **Install** to deploy into `C:\Program Files\Aura Island`.
- **Seamless Transition**: After setup completes, the stub launches `AuraIsland.exe --first-run` as a standard non-admin user and hands over directly to the island window at the same position and size.

### How to Install
1. Download `AuraIsland-Setup-<version>.exe` from the latest [GitHub Release](../../releases) or Actions workflow artifacts.
2. Double-click the file. The black pill drops down at the top center of your screen.
3. Click **Install** (or tap **Options** to customize the install folder or auto-start shortcuts).
4. Approve the Windows UAC admin prompt when the engine installs.
5. On completion, the pill transitions to "All set" and launches the installed application.

> [!NOTE]
> **Windows SmartScreen Note**: Because the executable is built directly in GitHub Actions and unsigned by an expensive commercial certificate, Windows SmartScreen may show an initial warning.
> Click **"More info"** and then **"Run anyway"**.

---

## 🚀 First-Run Onboarding Experience

When Aura Island launches for the first time (or when selecting **"Run setup again"** from the system tray menu), the island guides you step-by-step through an Apple-style setup flow (keyboard Tab/Enter and mouse friendly, up to 480x252):

1. **"What should I call you?"**: Enter your preferred display name. All greetings ("Good morning, <name>", "Good evening, <name>") use this.
2. **"Pick your style"**: Live preview toggle between **Notch (MacBook)** and **Floating (iPhone)**. Selecting one immediately morphs the island in real time.
3. **"Alerts"**: Granular toggles for Charging/battery, Screenshots, Downloads, Focus Timer, Gmail, AI Agent, USB & devices, and Volume HUD.
4. **"Your Home"**: Choose up to 4 quick chips (Tray, Clips, Mail, Next event, Focus timer, Now playing).
5. **"Make it yours"**: Accent color dots, Mascot eyes toggle, Hotkey (`Ctrl+Alt+Space`), Windows auto-start toggle, and local Clipboard history toggle.
6. **"Connect Google (optional)"**: Connect your Gmail & Calendar via secure local OAuth (read-only, tokens encrypted with DPAPI).
7. **"Try it"**: Drag a file to the top of your screen to see the island swallow animation, then collapse back to idle.

Progress is automatically saved after each screen into `%APPDATA%\AuraIsland\settings.json`.

---

## 🛡️ Reliability & Startup Troubleshooting

Aura Island is engineered to start reliably every single time:

- **Startup Logging**: The very first lines of `main()` initialize log rotation at `%APPDATA%\AuraIsland\logs\startup.log` (rotates at 1 MB, preserves 5 logs). It logs application version, OS build, system DPI, all connected monitor bounds, and each initialization step.
- **Minidump on Crash**: An unhandled exception filter writes full minidumps to `%APPDATA%\AuraIsland\crash\` and presents a native message box with the diagnostic path.
- **Never Writes to Install Directory**: Database (`aura_island.db`), settings (`settings.json`), logs, and caches all reside strictly in `%APPDATA%\AuraIsland`. The app runs completely unprivileged under `C:\Program Files\Aura Island`.
- **Proper Single-Instance Management**: Named mutex + named pipe IPC server. If you double-click `AuraIsland.exe` while it is already running, the second process wakes the existing island (expanding with "Aura Island is running" for 2 seconds) and exits. Stale locks from crashed instances are automatically detected and recovered.
- **Headless Selftest**: Verify everything anytime with:
  ```powershell
  AuraIsland.exe --selftest
  ```
  Initializes offscreen QML, SQLite database, and configuration, returning exit code 0 on success.

### What to do if the app doesn't start
If double-clicking the app fails:
1. Open PowerShell and view the log:
   ```powershell
   Get-Content "$env:APPDATA\AuraIsland\logs\startup.log" -Tail 50
   ```
2. Check if a crash dump was generated:
   ```powershell
   Get-ChildItem "$env:APPDATA\AuraIsland\crash"
   ```
3. Run the selftest command:
   ```powershell
   & "C:\Program Files\Aura Island\AuraIsland.exe" --selftest
   ```

---

## 🗑️ How to Uninstall

1. Open Windows **Settings** > **Apps** > **Installed apps** (or `appwiz.cpl`).
2. Search for **Aura Island** and click **Uninstall**.
3. The uninstaller will display a confirmation dialog:
   > *"Do you also want to delete your personal notes, settings, and clipboard database?"*
   - Select **No** (default) to keep your notes and database safe for future updates.
   - Select **Yes** to completely remove all files in `%APPDATA%\AuraIsland`.
4. Windows auto-start registry entries are cleanly removed.

---

## 🏗️ Architecture & Development

```
├── .github/workflows/build.yml     # Release build, windeployqt, dependency verify, selftest, Inno setup, stub build, smoke test
├── verify_dependencies.ps1         # Recursive PE/dumpbin dependency verification script
├── src/
│   ├── main.cpp                    # Robust entrypoint, logging first, crash filter, tray icon, IPC client
│   ├── core/
│   │   ├── Paths.h/cpp             # Centralized %APPDATA%\AuraIsland path management
│   │   ├── Logger.h/cpp            # Rotated startup logger (1MB, 5 keep), diagnostic logging, minidump handler
│   │   ├── SingleInstance.h/cpp    # Named mutex + IPC named pipe server & stale lock recovery
│   │   ├── Watchdog.h/cpp          # Supervisor process (max 3 restarts/min, never starts 2 windows)
│   │   ├── Settings.h/cpp          # settings.json + SQLite store, onboarding state, greeting generator
│   │   └── IslandWindow.h/cpp      # Notch clipping, work-area bounds safety, per-monitor DPI
├── qml/
│   ├── Island.qml                  # Unified notch shell with spring physics (stiffness 250, damping 24)
│   ├── components/
│   │   └── OnboardingView.qml      # 7-step Apple-style setup wizard inside the island
│   └── tabs/
│       ├── HomeTab.qml             # Dynamic greeting with user's name ("Good evening, Alex")
│       └── SettingsTab.qml         # "Run setup again" button & full customization
└── installer/
    ├── AuraEngine.iss              # Inno Setup 6 64-bit silent engine (admin required)
    └── stub/
        ├── main.cpp                # Native Win32 GDI+ pill installer (asInvoker, /MT, zero dependencies)
        ├── stub.rc                 # RCDATA resource embedding AuraEngine.exe
        ├── manifest.xml            # asInvoker + PerMonitorV2 manifest
        └── CMakeLists.txt          # Builds AuraIsland-Setup-<version>.exe
```

---

## 📄 License
MIT License. Created for native Windows productivity.
