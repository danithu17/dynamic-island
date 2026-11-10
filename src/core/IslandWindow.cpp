#include "IslandWindow.h"
#include "WindowTweaks.h"
#include "AppState.h"
#include <QGuiApplication>
#include <QResizeEvent>
#include <QDebug>
#include <windowsx.h>

namespace Aura {

IslandWindow::IslandWindow(QWindow *parent)
    : QQuickWindow(parent), m_proximityTimer(new QTimer(this)) {
    // Window configuration: frameless, topmost, toolwindow, non-focus-stealing
    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setColor(QColor(Qt::transparent));
    setVisible(true);

    // Initial canvas size: 540x360 centered at top
    repositionToTopCenter(540, 360);

    // Connect to AppState window resize requests
    connect(&AppState::instance(), &AppState::requestWindowResize,
            this, &IslandWindow::onWindowResizeRequested);

    // Proximity timer at 25ms to monitor mouse hover / outside clicks
    connect(m_proximityTimer, &QTimer::timeout, this, &IslandWindow::checkCursorProximity);
    m_proximityTimer->start(25);

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

void IslandWindow::updatePillBounds(int x, int y, int width, int height) {
    m_pillBounds = QRect(x, y, width, height);
}

void IslandWindow::requestWindowFocus() {
    if (m_hwnd) {
        SetForegroundWindow(m_hwnd);
        SetActiveWindow(m_hwnd);
    }
}

void IslandWindow::releaseWindowFocus() {
    if (m_hwnd) {
        // Clear focus if needed
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
    int winWidth = (targetWidth > 0) ? targetWidth : 540;
    int winHeight = (targetHeight > 0) ? targetHeight : 360;

    int x = screenGeo.x() + (screenGeo.width() - winWidth) / 2;
    int y = screenGeo.y(); // Top aligned

    setGeometry(x, y, winWidth, winHeight);

    if (m_hwnd) {
        WindowTweaks::setTopmostNoActivate(m_hwnd);
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    }
}

void IslandWindow::onWindowResizeRequested(int width, int height, int radius) {
    Q_UNUSED(width);
    Q_UNUSED(height);
    m_currentRadius = radius;
    // Canvas remains 540x360 centered; QML smoothly animates width/height inside!
    repositionToTopCenter(540, 360);
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
    repositionToTopCenter(540, 360);
}

bool IslandWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    MSG *msg = static_cast<MSG *>(message);
    if (!msg) return QQuickWindow::nativeEvent(eventType, message, result);

    if (msg->message == WM_NCHITTEST && m_hwnd) {
        POINT pt = { GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam) };
        ScreenToClient(m_hwnd, &pt);

        qreal dpr = devicePixelRatio();
        QRect physPill(
            static_cast<int>(m_pillBounds.x() * dpr) - 6,
            static_cast<int>(m_pillBounds.y() * dpr) - 6,
            static_cast<int>(m_pillBounds.width() * dpr) + 12,
            static_cast<int>(m_pillBounds.height() * dpr) + 12
        );

        if (!physPill.contains(pt.x, pt.y)) {
            if (result) *result = HTTRANSPARENT;
            return true;
        }
    }

    return QQuickWindow::nativeEvent(eventType, message, result);
}

void IslandWindow::checkCursorProximity() {
    if (!m_hwnd || m_isFullscreenSuppressed) return;

    POINT pt;
    if (GetCursorPos(&pt)) {
        POINT ptClient = pt;
        ScreenToClient(m_hwnd, &ptClient);

        qreal dpr = devicePixelRatio();
        QRect physPill(
            static_cast<int>(m_pillBounds.x() * dpr) - 6,
            static_cast<int>(m_pillBounds.y() * dpr) - 6,
            static_cast<int>(m_pillBounds.width() * dpr) + 12,
            static_cast<int>(m_pillBounds.height() * dpr) + 12
        );

        bool inside = physPill.contains(ptClient.x, ptClient.y);

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
