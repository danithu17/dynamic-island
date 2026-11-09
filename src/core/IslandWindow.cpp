#include "IslandWindow.h"
#include "WindowTweaks.h"
#include "AppState.h"
#include <QGuiApplication>
#include <QResizeEvent>
#include <QDebug>

namespace Aura {

IslandWindow::IslandWindow(QWindow *parent)
    : QQuickWindow(parent), m_proximityTimer(new QTimer(this)) {
    // Window configuration: frameless, topmost, toolwindow, non-focus-stealing
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setColor(QColor(Qt::transparent));

    // Connect to AppState window resize requests
    connect(&AppState::instance(), &AppState::requestWindowResize,
            this, &IslandWindow::onWindowResizeRequested);

    // Initial size: Idle pill (120x32)
    repositionToTopCenter(120, 32);

    // Proximity timer at 30ms to monitor mouse hover / click-through
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
    m_hwnd = reinterpret_cast<HWND>(winId());
    if (m_hwnd) {
        WindowTweaks::applyIslandWindowStyles(m_hwnd);
        WindowTweaks::applyGlassEffect(m_hwnd);
        WindowTweaks::updateWindowShape(m_hwnd, width(), height(), m_currentRadius);
        emit nativeWindowReady(m_hwnd);
    }
}

void IslandWindow::repositionToTopCenter(int targetWidth, int targetHeight) {
    QScreen *targetScreen = screen();
    if (!targetScreen) targetScreen = QGuiApplication::primaryScreen();
    if (!targetScreen) return;

    QRect screenGeo = targetScreen->geometry();
    // 8px transparent padding buffer so the pill edges never touch native window boundary
    const int pad = 8;
    int winWidth = targetWidth + (pad * 2);
    int winHeight = targetHeight + (pad * 2);

    int x = screenGeo.x() + (screenGeo.width() - winWidth) / 2;
    int y = screenGeo.y() + 8 - pad; // Align top of pill at 8px from screen edge

    setGeometry(x, y, winWidth, winHeight);

    if (m_hwnd) {
        WindowTweaks::setTopmostNoActivate(m_hwnd);
    }
}

void IslandWindow::onWindowResizeRequested(int width, int height, int radius) {
    m_currentRadius = radius;
    repositionToTopCenter(width, height);
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
    repositionToTopCenter(width(), height());
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

        if (inside != m_isHovered) {
            m_isHovered = inside;
            // If mouse is inside the island, clear WS_EX_TRANSPARENT so clicks are received
            // If outside, clicks pass directly through to windows underneath
            WindowTweaks::setClickThrough(m_hwnd, !m_isHovered);
            emit hoverChanged();
        }
    }
}

} // namespace Aura
