#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <iostream>
#include <exception>

#include "core/Logger.h"
#include "core/Paths.h"
#include "core/SingleInstance.h"
#include "core/Watchdog.h"
#include "core/DatabaseManager.h"
#include "core/Settings.h"
#include "core/AppState.h"
#include "core/IslandWindow.h"
#include "core/NativeEventFilter.h"
#include "core/FullscreenDetector.h"
#include "core/AlertManager.h"

#include "features/shelf/ShelfManager.h"
#include "features/clipboard/ClipboardManager.h"
#include "features/notes/NotesManager.h"
#include "features/gmail/GmailService.h"
#include "features/dev/DevManager.h"
#include "features/media/MediaManager.h"
#include "features/drop/DropManager.h"
#include "features/system/SystemControlManager.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static QString getAppVersion() {
    return QStringLiteral("1.0.8");
}

static int runUninstallCleanup() {
    QString appData = Aura::Paths::appDataDir();
    Aura::Logger::log(QtInfoMsg, QString("Performing uninstall cleanup for: %1").arg(appData));
    QDir dir(appData);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    std::cout << "[UninstallCleanup] Successfully removed app data directory." << std::endl;
    return 0;
}

static int runSelftest(int argc, char *argv[]) {
    std::cout << "[Selftest] Initializing headless selftest with QT_QPA_PLATFORM=offscreen..." << std::endl;
    qputenv("QT_QPA_PLATFORM", QByteArray("offscreen"));

    QApplication app(argc, argv);
    app.setApplicationName("AuraIsland");
    app.setApplicationDisplayName("Aura Island Selftest");
    app.setOrganizationName("Aura");

    Aura::Logger::initQtHandler();
    Aura::Logger::logStep("Selftest: Initializing database");

    if (!Aura::DatabaseManager::instance().init()) {
        std::cerr << "[Selftest ERROR] DatabaseManager failed to initialize!" << std::endl;
        return 1;
    }

    Aura::Logger::logStep("Selftest: Registering QML types");
    qmlRegisterType<Aura::IslandWindow>("Aura", 1, 0, "IslandWindow");
    qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/Theme.qml")), "Aura", 1, 0, "Theme");

    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("appState", &Aura::AppState::instance());
    context->setContextProperty("alertManager", &Aura::AlertManager::instance());
    context->setContextProperty("settings", &Aura::Settings::instance());
    context->setContextProperty("shelfManager", &Aura::ShelfManager::instance());
    context->setContextProperty("clipboardManager", &Aura::ClipboardManager::instance());
    context->setContextProperty("notesManager", &Aura::NotesManager::instance());
    context->setContextProperty("gmailService", &Aura::GmailService::instance());
    context->setContextProperty("devManager", &Aura::DevManager::instance());
    context->setContextProperty("mediaManager", &Aura::MediaManager::instance());
    context->setContextProperty("dropManager", &Aura::DropManager::instance());
    context->setContextProperty("systemControlManager", &Aura::SystemControlManager::instance());

    Aura::Logger::logStep("Selftest: Loading QML engine");
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty()) {
        std::cerr << "[Selftest ERROR] Failed to load QML root objects!" << std::endl;
        return 2;
    }

    std::cout << "[Selftest SUCCESS] Offscreen QML loaded and database initialized properly. Exit code 0." << std::endl;
    return 0;
}

int main(int argc, char *argv[]) {
    // 1. Logging first: the very first lines of main() open log file in %APPDATA%\AuraIsland\logs\startup.log
    Aura::Logger::initEarly();
    QString appVersion = getAppVersion();
    Aura::Logger::logDiagnosticInfo(appVersion);

    try {
        Aura::Logger::logStep("Parsing command line arguments");
        QStringList args;
        for (int i = 1; i < argc; ++i) {
            args << QString::fromLocal8Bit(argv[i]);
        }

        // Section A.9: Command line handling
        if (args.contains("--selftest")) {
            return runSelftest(argc, argv);
        }

        if (args.contains("--uninstall-cleanup")) {
            return runUninstallCleanup();
        }

        // 2. Supervisor watchdog
        if (!Aura::Watchdog::isWorkerProcess(argc, argv) && !Aura::Watchdog::shouldBypassWatchdog(argc, argv)) {
            return Aura::Watchdog::runSupervisor(argc, argv);
        }

        // 3. Single instance check & IPC server registration
        if (!Aura::SingleInstance::instance().checkAndAcquire(args)) {
            // Wake or show message was successfully sent to the running instance
            return 0;
        }

        Aura::Logger::logStep("Initializing Qt Application");
        QApplication app(argc, argv);
        app.setApplicationName("AuraIsland");
        app.setApplicationDisplayName("Aura Island");
        app.setOrganizationName("Aura");
        app.setQuitOnLastWindowClosed(false);

        // Section A.3: Resources and plugins found relative to exe path (applicationDirPath)
        QString appDir = QCoreApplication::applicationDirPath();
        QCoreApplication::addLibraryPath(appDir);
        QCoreApplication::addLibraryPath(appDir + "/plugins");

        Aura::Logger::initQtHandler();

        // 4. Section A.5: Always visible feedback - show system tray icon right away
        Aura::Logger::logStep("Setting up system tray icon");
        QSystemTrayIcon trayIcon(QIcon(":/resources/icons/app-icon.svg"), &app);
        trayIcon.setToolTip("Aura Island");

        QMenu trayMenu;
        QAction *openAction = trayMenu.addAction("Open");
        QObject::connect(openAction, &QAction::triggered, []() {
            Aura::AppState::instance().expandTab("home");
        });

        QAction *settingsAction = trayMenu.addAction("Settings");
        QObject::connect(settingsAction, &QAction::triggered, []() {
            Aura::AppState::instance().expandTab("settings");
        });

        QAction *setupAction = trayMenu.addAction("Run setup again");
        QObject::connect(setupAction, &QAction::triggered, []() {
            Aura::AppState::instance().startOnboarding();
        });

        trayMenu.addSeparator();
        QAction *viewLogAction = trayMenu.addAction("View log");
        QObject::connect(viewLogAction, &QAction::triggered, []() {
            QString logPath = Aura::Paths::startupLogPath();
            QDesktopServices::openUrl(QUrl::fromLocalFile(logPath));
        });

        trayMenu.addSeparator();
        QAction *quitAction = trayMenu.addAction("Quit");
        QObject::connect(quitAction, &QAction::triggered, &app, &QCoreApplication::quit);

        trayIcon.setContextMenu(&trayMenu);
        QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
                Aura::AppState::instance().toggleExpanded();
            }
        });
        trayIcon.show();

        // 5. Connect SingleInstance IPC signals to AppState
        QObject::connect(&Aura::SingleInstance::instance(), &Aura::SingleInstance::wakeRequested,
                         [](const QString &msg) {
            Aura::AppState::instance().wake(msg, 2000);
        });
        QObject::connect(&Aura::SingleInstance::instance(), &Aura::SingleInstance::showRequested,
                         []() {
            Aura::AppState::instance().expandTab("home");
        });
        QObject::connect(&Aura::SingleInstance::instance(), &Aura::SingleInstance::setupRequested,
                         []() {
            Aura::AppState::instance().startOnboarding();
        });

        // 6. Initialize SQLite Database in %APPDATA%\AuraIsland\aura_island.db
        Aura::Logger::logStep("Initializing database");
        if (!Aura::DatabaseManager::instance().init()) {
            qCritical() << "Failed to initialize SQLite database!";
        }

        // 7. Native Event Filter for Win32 Messages (Hotkey, Clipboard, DPI)
        Aura::Logger::logStep("Installing native event filter");
        auto *nativeFilter = new Aura::NativeEventFilter(&app);
        app.installNativeEventFilter(nativeFilter);

        // Connect hotkeys & clipboard notifications
        QObject::connect(nativeFilter, &Aura::NativeEventFilter::hotkeyTriggered, []() {
            Aura::AppState::instance().togglePinned();
        });
        QObject::connect(nativeFilter, &Aura::NativeEventFilter::quickNoteTriggered, []() {
            Aura::AppState::instance().openQuickCapture();
        });
        QObject::connect(nativeFilter, &Aura::NativeEventFilter::pasteStackTriggered, []() {
            Aura::ClipboardManager::instance().popPasteStack();
        });
        QObject::connect(nativeFilter, &Aura::NativeEventFilter::clipboardChanged, []() {
            Aura::ClipboardManager::instance().onNativeClipboardNotification();
        });

        // 8. Register QML Types
        qmlRegisterType<Aura::IslandWindow>("Aura", 1, 0, "IslandWindow");
        qmlRegisterSingletonType(QUrl(QStringLiteral("qrc:/qml/Theme.qml")), "Aura", 1, 0, "Theme");

        // 9. Setup QML Engine & Root Context
        Aura::Logger::logStep("Setting up QML Engine");
        QQmlApplicationEngine engine;
        engine.addImportPath(appDir + "/qml");

        QQmlContext *context = engine.rootContext();
        context->setContextProperty("appState", &Aura::AppState::instance());
        context->setContextProperty("alertManager", &Aura::AlertManager::instance());
        context->setContextProperty("settings", &Aura::Settings::instance());
        context->setContextProperty("shelfManager", &Aura::ShelfManager::instance());
        context->setContextProperty("clipboardManager", &Aura::ClipboardManager::instance());
        context->setContextProperty("notesManager", &Aura::NotesManager::instance());
        context->setContextProperty("gmailService", &Aura::GmailService::instance());
        context->setContextProperty("devManager", &Aura::DevManager::instance());
        context->setContextProperty("mediaManager", &Aura::MediaManager::instance());
        context->setContextProperty("dropManager", &Aura::DropManager::instance());
        context->setContextProperty("systemControlManager", &Aura::SystemControlManager::instance());

        QQmlComponent themeComponent(&engine, QUrl(QStringLiteral("qrc:/qml/Theme.qml")));
        QObject *themeObject = themeComponent.create(context);
        if (themeObject) {
            context->setContextProperty("Theme", themeObject);
        } else {
            qWarning() << "Failed to instantiate Theme:" << themeComponent.errorString();
        }

        // 10. Load QML Interface
        Aura::Logger::logStep("Loading Main.qml");
        const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url, nativeFilter](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qCritical() << "Failed to load Main.qml!";
                QCoreApplication::exit(-1);
                return;
            }

            auto *window = qobject_cast<Aura::IslandWindow *>(obj);
            if (window) {
                auto onHwndReady = [window, nativeFilter](HWND hwnd) {
                    nativeFilter->registerGlobalHotkey(hwnd);
                    nativeFilter->registerClipboardListener(hwnd);
                };

                if (window->hwnd()) {
                    onHwndReady(window->hwnd());
                } else {
                    QObject::connect(window, &Aura::IslandWindow::nativeWindowReady, onHwndReady);
                }

                QObject::connect(nativeFilter, &Aura::NativeEventFilter::displayConfigurationChanged,
                                 window, &Aura::IslandWindow::onDisplayConfigurationChanged);
            }
        }, Qt::QueuedConnection);

        engine.load(url);

        if (engine.rootObjects().isEmpty()) {
            qCritical() << "Fatal: QML engine created no root objects.";
            return -1;
        }

        auto *islandWindow = qobject_cast<Aura::IslandWindow *>(engine.rootObjects().first());
        if (islandWindow) {
            islandWindow->initializeNativeWindow();
            islandWindow->show();
        }

        // 11. Fullscreen Detection
        auto *fullscreenDetector = new Aura::FullscreenDetector(&app);
        if (islandWindow) {
            QObject::connect(fullscreenDetector, &Aura::FullscreenDetector::fullscreenStateChanged,
                             islandWindow, &Aura::IslandWindow::onFullscreenChanged);
        }
        fullscreenDetector->start(1500);

        // 12. Connect AppState quit request to app quit
        QObject::connect(&Aura::AppState::instance(), &Aura::AppState::quitRequested,
                         &app, &QCoreApplication::quit);

        // 13. Initial presentation / First run handling
        if (args.contains("--first-run") || !Aura::Settings::instance().onboardingCompleted()) {
            Aura::Logger::logStep("First run requested: triggering onboarding");
            Aura::AppState::instance().startOnboarding();
        } else if (args.contains("--show")) {
            Aura::AppState::instance().expandTab("home");
        } else {
            // Requirement A.5: Play 2-second "wake" animation on launch
            Aura::Logger::logStep("Playing launch wake animation");
            Aura::AppState::instance().wake("Aura Island is running", 2000);
        }

        Aura::Logger::logStep("Application initialized successfully. Starting event loop.");
        int result = app.exec();

        Aura::SingleInstance::instance().release();
        Aura::Logger::cleanup();
        return result;

    } catch (const std::exception &e) {
        QString errorMsg = QString("Fatal exception in main(): %1\n\nDiagnostic log located at:\n%2")
            .arg(e.what(), Aura::Paths::startupLogPath());
        Aura::Logger::log(QtFatalMsg, errorMsg);
        MessageBoxW(NULL, errorMsg.toStdWString().c_str(), L"Aura Island - Startup Error", MB_ICONERROR | MB_OK);
        return 1;
    } catch (...) {
        QString errorMsg = QString("Unknown fatal exception in main().\n\nDiagnostic log located at:\n%2")
            .arg(Aura::Paths::startupLogPath());
        Aura::Logger::log(QtFatalMsg, errorMsg);
        MessageBoxW(NULL, errorMsg.toStdWString().c_str(), L"Aura Island - Startup Error", MB_ICONERROR | MB_OK);
        return 2;
    }
}
