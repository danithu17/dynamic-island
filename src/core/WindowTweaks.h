#pragma once

#include <windows.h>
#include <dwmapi.h>
#include <QRect>

namespace Aura {

class WindowTweaks {
public:
    static void applyIslandWindowStyles(HWND hwnd);
    static void applyGlassEffect(HWND hwnd);
    static void setClickThrough(HWND hwnd, bool clickThrough);
    static void updateWindowShape(HWND hwnd, int width, int height, int cornerRadius, bool isNotch = false);
    static void setTopmostNoActivate(HWND hwnd);
    static bool isPointInsideIsland(HWND hwnd, const QRect &islandGeometry, POINT cursorPos);
};

} // namespace Aura
