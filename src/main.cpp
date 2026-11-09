#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QDebug>

#include "core/Logger.h"
#include "core/Watchdog.h"
#include "core/DatabaseManager.h"
#include "core/Settings.h"
#include "core/AppState.h"
#include "core/IslandWindow.h"
#include "core/NativeEventFilter.h"
#include "core/FullscreenDetector.h"

#include "features/shelf/ShelfManager.h"
#include "features/clipboard/ClipboardManager.h"
#include "features/notes/NotesManager.h"
#include "features/gmail/GmailService.h"
#include "features/dev/DevManager.h"
#include "features/media/MediaManager.h"

int main(int argc, char *argv[]) {
    // 1. Crash-safe supervisor watchdog
    if (!Aura::Watchdog::isWorkerProcess(argc, argv)) {
        return Aura::Watchdog::runSupervisor(argc, argv);
    }

    QApplication app(argc, argv);
    app.setApplicationName("AuraIsland");
    app.setApplicationDisplayName("Aura Island");
    app.setOrganizationName("Aura");
    app.setQuitOnLastWindowClosed(false);

    // 2. Initialize rotated file logger in %APPDATA%/Aura/AuraIsland/logs
    Aura::Logger::init();
    qInfo() << "========================================";
    qInfo() << "Starting Aura Island - Liquid Glass Hub";
    qInfo() << "========================================";

    // 3. Initialize SQLite Database
    if (!Aura::DatabaseManager::instance().init()) {
        qCritical() << "Failed to initialize SQLite database!";
    }

    // 4. Native Event Filter for Win32 Messages (Hotkey, Clipboard, DPI)
    auto *nativeFilter = new Aura::NativeEventFilter(&app);
    app.installNativeEventFilter(nativeFilter);

    // Connect hotkey (Ctrl+Alt+Space) to toggle island expansion
    QObject::connect(nativeFilter, &Aura::NativeEventFilter::hotkeyTriggered, []() {
        Aura::AppState::instance().toggleExpanded();
    });

    // Connect Quick Capture hotkey (Ctrl+Alt+N)
    QObject::connect(nativeFilter, &Aura::NativeEventFilter::quickNoteTriggered, []() {
        Aura::AppState::instance().openQuickCapture();
    });

    // Connect Paste Stack hotkey (Ctrl+Alt+V)
    QObject::connect(nativeFilter, &Aura::NativeEventFilter::pasteStackTriggered, []() {
        Aura::ClipboardManager::instance().popPasteStack();
    });

    // Connect clipboard update notification
    QObject::connect(nativeFilter, &Aura::NativeEventFilter::clipboardChanged, []() {
        Aura::ClipboardManager::instance().onNativeClipboardNotification();
    });

    // 5. Register QML Types
    qmlRegisterType<Aura::IslandWindow>("Aura", 1, 0, "IslandWindow");

    // 6. Setup QML Engine & Root Context
    QQmlApplicationEngine engine;
    QQmlContext *context = engine.rootContext();
    context->setContextProperty("appState", &Aura::AppState::instance());
    context->setContextProperty("settings", &Aura::Settings::instance());
    context->setContextProperty("shelfManager", &Aura::ShelfManager::instance());
    context->setContextProperty("clipboardManager", &Aura::ClipboardManager::instance());
    context->setContextProperty("notesManager", &Aura::NotesManager::instance());
    context->setContextProperty("gmailService", &Aura::GmailService::instance());
    context->setContextProperty("devManager", &Aura::DevManager::instance());
    context->setContextProperty("mediaManager", &Aura::MediaManager::instance());

    // 7. Load QML Interface
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

    // 8. Fullscreen Detection (Auto-hide during fullscreen games, videos, presentations)
    auto *fullscreenDetector = new Aura::FullscreenDetector(&app);
    if (islandWindow) {
        QObject::connect(fullscreenDetector, &Aura::FullscreenDetector::fullscreenStateChanged,
                         islandWindow, &Aura::IslandWindow::onFullscreenChanged);
    }
    fullscreenDetector->start(1500);

    // 9. System Tray Icon Setup
    QSystemTrayIcon trayIcon(QIcon(":/resources/icons/app-icon.svg"), &app);
    trayIcon.setToolTip("Aura Island - Liquid Glass Productivity Hub");

    QMenu trayMenu;
    QAction *toggleAction = trayMenu.addAction("Show / Toggle Island (Ctrl+Alt+Space)");
    QObject::connect(toggleAction, &QAction::triggered, []() {
        Aura::AppState::instance().toggleExpanded();
    });

    trayMenu.addSeparator();
    QAction *shelfAction = trayMenu.addAction("Open Shelf");
    QObject::connect(shelfAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("shelf");
    });

    QAction *mailAction = trayMenu.addAction("Open Mail");
    QObject::connect(mailAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("mail");
    });

    QAction *clipAction = trayMenu.addAction("Open Clipboard History");
    QObject::connect(clipAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("clipboard");
    });

    QAction *notesAction = trayMenu.addAction("Open Notes & Snippets");
    QObject::connect(notesAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("notes");
    });

    QAction *devAction = trayMenu.addAction("Open Dev Hub");
    QObject::connect(devAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("dev");
    });

    QAction *mediaAction = trayMenu.addAction("Open Media Player");
    QObject::connect(mediaAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("media");
    });

    trayMenu.addSeparator();
    QAction *settingsAction = trayMenu.addAction("Settings");
    QObject::connect(settingsAction, &QAction::triggered, []() {
        Aura::AppState::instance().expandTab("settings");
    });

    trayMenu.addSeparator();
    QAction *quitAction = trayMenu.addAction("Quit Aura Island");
    QObject::connect(quitAction, &QAction::triggered, &app, &QCoreApplication::quit);

    trayIcon.setContextMenu(&trayMenu);
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            Aura::AppState::instance().toggleExpanded();
        }
    });
    trayIcon.show();

    // Connect AppState quit request to app quit
    QObject::connect(&Aura::AppState::instance(), &Aura::AppState::quitRequested,
                     &app, &QCoreApplication::quit);

    int result = app.exec();

    Aura::Logger::cleanup();
    return result;
}
