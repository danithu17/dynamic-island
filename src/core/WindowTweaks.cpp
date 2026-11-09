#include "WindowTweaks.h"
#include <QDebug>

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWCP_ROUND
#define DWMWCP_ROUND 2
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMSBT_TRANSIENTWINDOW
#define DWMSBT_TRANSIENTWINDOW 3
#endif

// Undocumented Windows 10 Acrylic structs
struct ACCENT_POLICY {
    int AccentState;
    int AccentFlags;
    int GradientColor;
    int AnimationId;
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    int Attrib;
    void *pvData;
    int cbData;
};

typedef BOOL(WINAPI *pfnSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA *);

namespace Aura {

void WindowTweaks::applyIslandWindowStyles(HWND hwnd) {
    if (!hwnd) return;

    // Retrieve current extended styles
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

    // Apply WS_EX_TOOLWINDOW (no taskbar, no Alt-Tab) and WS_EX_NOACTIVATE (never steals focus)
    exStyle |= WS_EX_TOOLWINDOW;
    exStyle |= WS_EX_NOACTIVATE;

    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    // Remove borders / titlebar in standard style
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~WS_CAPTION;
    style &= ~WS_THICKFRAME;
    style &= ~WS_MINIMIZEBOX;
    style &= ~WS_MAXIMIZEBOX;
    style |= WS_POPUP;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    // Disable DWM non-client rendering to prevent top window shadow / frame ears
    DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));

    // Set topmost without activating
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void WindowTweaks::applyGlassEffect(HWND hwnd) {
    if (!hwnd) return;

    // Extend DWM frame into client area with -1 margins.
    // This turns the native window background into a true transparent alpha-compositing canvas.
    // Qt Quick with setColor(Qt::transparent) will render directly on the desktop with clean,
    // antialiased Liquid Glass borders and shadows — without any ugly opaque Windows 11 gray boxes!
    MARGINS margins = {-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);
}

void WindowTweaks::setClickThrough(HWND hwnd, bool clickThrough) {
    if (!hwnd) return;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (clickThrough) {
        if (!(exStyle & WS_EX_TRANSPARENT)) {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
        }
    } else {
        if (exStyle & WS_EX_TRANSPARENT) {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
        }
    }
}

void WindowTweaks::updateWindowShape(HWND hwnd, int width, int height, int cornerRadius) {
    Q_UNUSED(hwnd);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(cornerRadius);
}

void WindowTweaks::setTopmostNoActivate(HWND hwnd) {
    if (!hwnd) return;
    // SWP_NOMOVE | SWP_NOSIZE ensures we do NOT overwrite Qt's DPI-scaled position or dimensions.
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

bool WindowTweaks::isPointInsideIsland(HWND hwnd, const QRect &islandGeometry, POINT cursorPos) {
    Q_UNUSED(hwnd);
    return islandGeometry.contains(cursorPos.x, cursorPos.y);
}

} // namespace Aura
