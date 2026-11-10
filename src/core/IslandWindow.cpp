#include "IslandWindow.h"
#include "WindowTweaks.h"
#include "AppState.h"
#include "Settings.h"
#include <QGuiApplication>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QDebug>

namespace Aura {

IslandWindow::IslandWindow(QWindow *parent)
    : QQuickWindow(parent)
    , m_proximityTimer(new QTimer(this))
    , m_hoverExpandTimer(new QTimer(this))
    , m_collapseDelayTimer(new QTimer(this)) {
    
    // Window configuration: frameless, topmost, toolwindow, non-focus-stealing
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setColor(QColor(Qt::transparent));
    setVisible(true);

    m_hoverExpandTimer->setSingleShot(true);
    connect(m_hoverExpandTimer, &QTimer::timeout, this, &IslandWindow::onHoverExpandTriggered);

    m_collapseDelayTimer->setSingleShot(true);
    connect(m_collapseDelayTimer, &QTimer::timeout, this, &IslandWindow::onCollapseTimerTriggered);

    // Connect to AppState window resize requests
    connect(&AppState::instance(), &AppState::requestWindowResize,
            this, &IslandWindow::onWindowResizeRequested);

    // Connect to Settings islandStyle changes
    connect(&Settings::instance(), &Settings::islandStyleChanged, this, [this]() {
        repositionToTopCenter(m_targetWidth, m_targetHeight);
    });

    // Initial size: Idle notch (148x34)
    repositionToTopCenter(148, 34);

    // Proximity timer initially at 40ms to monitor mouse hover / outside desktop clicks
    connect(m_proximityTimer, &QTimer::timeout, this, &IslandWindow::checkCursorProximity);
    m_proximityTimer->start(40);

    // Watch for screen geometry changes
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        connect(screen, &QScreen::geometryChanged, this, &IslandWindow::onDisplayConfigurationChanged);
    }
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, [this](QScreen *newScreen) {
        if (newScreen) {
            connect(newScreen, &QScreen::geometryChanged, this, &IslandWindow::onDisplayConfigurationChanged);
            onDisplayConfigurationChanged();
        }
    });
}

IslandWindow::~IslandWindow() {
    m_proximityTimer->stop();
    m_hoverExpandTimer->stop();
    m_collapseDelayTimer->stop();
}

void IslandWindow::requestWindowFocus() {
    if (m_hwnd) {
        HWND curFore = GetForegroundWindow();
        if (curFore != m_hwnd) {
            m_prevForegroundHwnd = curFore;
        }
        SetForegroundWindow(m_hwnd);
        SetActiveWindow(m_hwnd);
    }
}

void IslandWindow::releaseWindowFocus() {
    if (m_prevForegroundHwnd && IsWindow(m_prevForegroundHwnd)) {
        SetForegroundWindow(m_prevForegroundHwnd);
        m_prevForegroundHwnd = nullptr;
    }
}

void IslandWindow::handleEscKey() {
    AppState::instance().collapseToIdle();
    releaseWindowFocus();
}

void IslandWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        handleEscKey();
        event->accept();
        return;
    }
    QQuickWindow::keyPressEvent(event);
}

void IslandWindow::exposeEvent(QExposeEvent *event) {
    QQuickWindow::exposeEvent(event);
    if (!m_isExposedFirstTime && isExposed()) {
        m_isExposedFirstTime = true;
        initializeNativeWindow();
    }
}

void IslandWindow::resizeEvent(QResizeEvent *event) {
    QQuickWindow::resizeEvent(event);
    if (m_hwnd) {
        bool isNotch = (Settings::instance().islandStyle() == "notch");
        WindowTweaks::updateWindowShape(m_hwnd, m_targetWidth, m_targetHeight, m_currentRadius, isNotch);
    }
}

void IslandWindow::initializeNativeWindow() {
    if (!m_hwnd) {
        m_hwnd = reinterpret_cast<HWND>(winId());
    }
    if (m_hwnd) {
        WindowTweaks::applyIslandWindowStyles(m_hwnd);
        WindowTweaks::applyGlassEffect(m_hwnd);
        bool isNotch = (Settings::instance().islandStyle() == "notch");
        WindowTweaks::updateWindowShape(m_hwnd, m_targetWidth, m_targetHeight, m_currentRadius, isNotch);
        WindowTweaks::setTopmostNoActivate(m_hwnd);
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
        emit nativeWindowReady(m_hwnd);
    }
}

void IslandWindow::repositionToTopCenter(int targetWidth, int targetHeight) {
    QScreen *targetScreen = screen();
    if (!targetScreen) targetScreen = QGuiApplication::primaryScreen();
    if (!targetScreen) return;

    QRect screenGeo = targetScreen->geometry();
    int winW = (targetWidth > 0) ? targetWidth : 148;
    int winH = (targetHeight > 0) ? targetHeight : 34;

    bool isNotch = (Settings::instance().islandStyle() == "notch");
    int flareW = isNotch ? 10 : 0;
    int totalW = winW + flareW * 2;

    int x = screenGeo.x() + (screenGeo.width() - winW) / 2 - flareW;
    int y = screenGeo.y() + (isNotch ? 0 : 10);

    setGeometry(x, y, totalW, winH);

    if (m_hwnd) {
        WindowTweaks::updateWindowShape(m_hwnd, winW, winH, m_currentRadius, isNotch);
        WindowTweaks::setTopmostNoActivate(m_hwnd);
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    }
}

void IslandWindow::onWindowResizeRequested(int width, int height, int radius) {
    m_currentRadius = radius;
    m_targetWidth = width;
    m_targetHeight = height;

    if (width <= 160) {
        // Collapsing to idle: DELAY native window resize by 280ms
        // so the spring collapse animation plays smoothly without clipping
        m_collapseDelayTimer->stop();
        QTimer::singleShot(280, this, [this, width, height]() {
            if (AppState::instance().islandState() == "idle") {
                repositionToTopCenter(width, height);
            }
        });
    } else {
        // Expanding: Resize immediately so expanding pill can grow freely
        repositionToTopCenter(width, height);
    }

    if (!isVisible()) {
        show();
    }
    if (m_hwnd) {
        WindowTweaks::setTopmostNoActivate(m_hwnd);
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    }
}

void IslandWindow::onFullscreenChanged(bool isFullscreen) {
    m_isFullscreenSuppressed = isFullscreen;
    if (isFullscreen) {
        hide();
    } else {
        show();
        if (m_hwnd) {
            WindowTweaks::setTopmostNoActivate(m_hwnd);
        }
    }
}

void IslandWindow::onDisplayConfigurationChanged() {
    repositionToTopCenter(m_targetWidth, m_targetHeight);
}

void IslandWindow::checkCursorProximity() {
    if (!m_hwnd || m_isFullscreenSuppressed) return;

    POINT pt;
    if (!GetCursorPos(&pt)) return;

    // Adaptive cursor polling: 30-60ms in top 80px, otherwise 250ms
    if (pt.y <= 80) {
        if (m_proximityTimer->interval() != 40) m_proximityTimer->setInterval(40);
    } else {
        if (m_proximityTimer->interval() != 250) m_proximityTimer->setInterval(250);
    }

    bool isMouseButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
                             (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ||
                             (GetAsyncKeyState(VK_MBUTTON) & 0x8000);

    QString state = AppState::instance().islandState();

    if (state == "idle") {
        QScreen *targetScreen = screen();
        if (!targetScreen) targetScreen = QGuiApplication::primaryScreen();
        if (!targetScreen) return;
        QRect screenGeo = targetScreen->geometry();

        bool isNotch = (Settings::instance().islandStyle() == "notch");
        int centerX = screenGeo.x() + screenGeo.width() / 2;
        int idleLeft = centerX - 148 / 2;
        int idleRight = centerX + 148 / 2;
        int idleTop = screenGeo.y() + (isNotch ? 0 : 10);
        int idleBottom = idleTop + 34;

        // Spec Section 9: 360x140 catch zone at top center with left button down
        int catchLeft = centerX - 180;
        int catchRight = centerX + 180;
        int catchTop = screenGeo.y();
        int catchBottom = screenGeo.y() + 140;
        bool inCatchZone = (pt.x >= catchLeft && pt.x <= catchRight && pt.y >= catchTop && pt.y <= catchBottom);

        if (isMouseButtonDown && inCatchZone) {
            m_hoverExpandTimer->stop();
            AppState::instance().setDragActive(true);
            AppState::instance().setIslandState("drop");
            WindowTweaks::setClickThrough(m_hwnd, false);
            return;
        }

        bool overIdle = (pt.x >= idleLeft && pt.x <= idleRight && pt.y >= idleTop && pt.y <= idleBottom);

        if (overIdle) {
            if (!m_isHovered) {
                m_isHovered = true;
                emit hoverChanged();
            }
            if (!isMouseButtonDown && !m_hoverExpandTimer->isActive()) {
                m_hoverExpandTimer->start(180);
            }
        } else {
            m_hoverExpandTimer->stop();
            if (m_isHovered) {
                m_isHovered = false;
                emit hoverChanged();
            }
        }
    } else {
        // Expanded or other active states
        RECT rc;
        GetWindowRect(m_hwnd, &rc);
        // Inflate by 24px as specified in spec Section 4
        RECT rcInflated = { rc.left - 24, rc.top - 24, rc.right + 24, rc.bottom + 24 };
        bool insideInflated = PtInRect(&rcInflated, pt);

        if (insideInflated) {
            m_collapseDelayTimer->stop();
            if (!m_isHovered) {
                m_isHovered = true;
                emit hoverChanged();
            }
        } else {
            if (m_isHovered) {
                m_isHovered = false;
                emit hoverChanged();
            }

            // Outside desktop click auto-collapse
            if (isMouseButtonDown && !AppState::instance().isPinned()) {
                if (AppState::instance().canCollapse()) {
                    AppState::instance().collapseToIdle();
                    releaseWindowFocus();
                    return;
                }
            }

            // Collapse after 350ms outside inflated bounds
            if (!m_collapseDelayTimer->isActive()) {
                m_collapseDelayTimer->start(350);
            }
        }
    }
}

void IslandWindow::onHoverExpandTriggered() {
    POINT pt;
    if (GetCursorPos(&pt)) {
        bool isMouseButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ||
                                 (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ||
                                 (GetAsyncKeyState(VK_MBUTTON) & 0x8000);
        if (isMouseButtonDown) return; // Cancelled by mouse button
    }

    if (AppState::instance().islandState() == "idle") {
        HWND curFore = GetForegroundWindow();
        if (curFore != m_hwnd) {
            m_prevForegroundHwnd = curFore;
        }
        AppState::instance().setCurrentTab("home");
        AppState::instance().setIslandState("expanded");
    }
}

void IslandWindow::onCollapseTimerTriggered() {
    if (AppState::instance().islandState() != "idle") {
        if (AppState::instance().canCollapse()) {
            AppState::instance().collapseToIdle();
            releaseWindowFocus();
        }
    }
}

} // namespace Aura
