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
    static void updateWindowShape(HWND hwnd, int width, int height, int cornerRadius);
    static void setTopmostNoActivate(HWND hwnd, int x, int y, int width, int height);
    static bool isPointInsideIsland(HWND hwnd, const QRect &islandGeometry, POINT cursorPos);
};

} // namespace Aura
