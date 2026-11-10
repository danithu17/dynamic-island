#include "IslandWindow.h"
#include "WindowTweaks.h"
#include "AppState.h"
#include <QGuiApplication>
#include <QResizeEvent>
#include <QDebug>

namespace Aura {

IslandWindow::IslandWindow(QWindow *parent)
    : QQuickWindow(parent)
    , m_proximityTimer(new QTimer(this))
    , m_collapseDelayTimer(new QTimer(this)) {
    // Window configuration: frameless, topmost, toolwindow, non-focus-stealing
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setColor(QColor(Qt::transparent));
    setVisible(true);

    m_collapseDelayTimer->setSingleShot(true);
    connect(m_collapseDelayTimer, &QTimer::timeout, this, [this]() {
        repositionToTopCenter(m_targetWidth, m_targetHeight);
    });

    // Connect to AppState window resize requests
    connect(&AppState::instance(), &AppState::requestWindowResize,
            this, &IslandWindow::onWindowResizeRequested);

    // Initial size: Idle pill (134x36)
    repositionToTopCenter(134, 36);

    // Proximity timer at 30ms to monitor mouse hover / outside desktop clicks
    connect(m_proximityTimer, &QTimer::timeout, this, &IslandWindow::checkCursorProximity);
    m_proximityTimer->start(30);

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
    m_collapseDelayTimer->stop();
}

void IslandWindow::requestWindowFocus() {
    if (m_hwnd) {
        SetForegroundWindow(m_hwnd);
        SetActiveWindow(m_hwnd);
    }
}

void IslandWindow::releaseWindowFocus() {
    if (m_hwnd) {
        // Return focus
    }
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
        WindowTweaks::updateWindowShape(m_hwnd, width(), height(), m_currentRadius);
    }
}

void IslandWindow::initializeNativeWindow() {
    if (!m_hwnd) {
        m_hwnd = reinterpret_cast<HWND>(winId());
    }
    if (m_hwnd) {
        WindowTweaks::applyIslandWindowStyles(m_hwnd);
        WindowTweaks::applyGlassEffect(m_hwnd);
        WindowTweaks::updateWindowShape(m_hwnd, width(), height(), m_currentRadius);
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
    int winWidth = (targetWidth > 0) ? targetWidth : 134;
    int winHeight = (targetHeight > 0) ? targetHeight : 36;

    int x = screenGeo.x() + (screenGeo.width() - winWidth) / 2;
    int y = screenGeo.y() + 8; // Top aligned 8px from screen edge

    setGeometry(x, y, winWidth, winHeight);

    if (m_hwnd) {
        WindowTweaks::setTopmostNoActivate(m_hwnd);
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    }
}

void IslandWindow::onWindowResizeRequested(int width, int height, int radius) {
    m_currentRadius = radius;
    m_targetWidth = width;
    m_targetHeight = height;

    if (width <= 140) {
        // Collapsing to idle: DELAY native window resize by 280ms
        // so the QML spring collapse animation (260ms) plays with full visibility and zero clipping!
        m_collapseDelayTimer->start(280);
    } else {
        // Expanding: Cancel any pending collapse timer and resize native window immediately
        // so the expanding QML pill can stretch freely!
        m_collapseDelayTimer->stop();
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
    if (GetCursorPos(&pt)) {
        POINT ptClient = pt;
        ScreenToClient(m_hwnd, &ptClient);
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        bool inside = PtInRect(&rc, ptClient);

        // Outside desktop click auto-collapse (when expanded and not pinned)
        if (!inside && AppState::instance().islandState() == "expanded" && !AppState::instance().isPinned()) {
            if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
                AppState::instance().collapseToIdle();
            }
        }

        if (inside != m_isHovered) {
            m_isHovered = inside;
            emit hoverChanged();
        }
    }
}

} // namespace Aura
