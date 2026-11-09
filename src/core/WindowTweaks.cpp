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
    exStyle |= WS_EX_LAYERED;

    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

    // Remove borders / titlebar in standard style
    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
    style &= ~WS_CAPTION;
    style &= ~WS_THICKFRAME;
    style &= ~WS_MINIMIZEBOX;
    style &= ~WS_MAXIMIZEBOX;
    style |= WS_POPUP;
    SetWindowLongPtr(hwnd, GWL_STYLE, style);

    // Set topmost without activating
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void WindowTweaks::applyGlassEffect(HWND hwnd) {
    if (!hwnd) return;

    // Try Windows 11 Backdrop (Acrylic / TransientWindow)
    int backdrop = DWMSBT_TRANSIENTWINDOW;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

    // Try Windows 11 Rounded Corners preference
    int corner = DWMWCP_ROUND;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

    // If DWM backdrop failed (e.g. running on Windows 10), attempt Windows 10 composition acrylic fallback
    if (FAILED(hr)) {
        HMODULE hUser = GetModuleHandleW(L"user32.dll");
        if (hUser) {
            auto setWindowComposition = reinterpret_cast<pfnSetWindowCompositionAttribute>(
                GetProcAddress(hUser, "SetWindowCompositionAttribute"));
            if (setWindowComposition) {
                ACCENT_POLICY policy = {4 /* ACCENT_ENABLE_ACRYLICBLURBEHIND */, 2, static_cast<int>(0x99181818), 0};
                WINDOWCOMPOSITIONATTRIBDATA data = {19 /* WCA_ACCENT_POLICY */, &policy, sizeof(policy)};
                setWindowComposition(hwnd, &data);
            }
        }
    }

    // Extend frame into client area for glass composition
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
    if (!hwnd || width <= 0 || height <= 0) return;

    // SetWindowRgn creates an exact hardware clip region matching the rounded pill shape.
    // Mouse clicks outside this region pass directly to apps below with 0 latency and 0 CPU.
    int diameter = cornerRadius * 2;
    HRGN hrgn = CreateRoundRectRgn(0, 0, width + 1, height + 1, diameter, diameter);
    if (hrgn) {
        SetWindowRgn(hwnd, hrgn, TRUE);
    }
}

void WindowTweaks::setTopmostNoActivate(HWND hwnd, int x, int y, int width, int height) {
    if (!hwnd) return;
    SetWindowPos(hwnd, HWND_TOPMOST, x, y, width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

bool WindowTweaks::isPointInsideIsland(HWND hwnd, const QRect &islandGeometry, POINT cursorPos) {
    Q_UNUSED(hwnd);
    return islandGeometry.contains(cursorPos.x, cursorPos.y);
}

} // namespace Aura
