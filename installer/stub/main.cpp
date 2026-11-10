#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ole32.lib")

using namespace Gdiplus;

#define IDR_AURA_ENGINE 101

enum SetupState {
    STATE_LAUNCH_DROP,
    STATE_LAUNCH_HELLO,
    STATE_WELCOME,
    STATE_OPTIONS,
    STATE_INSTALLING,
    STATE_DONE,
    STATE_SHRINKING,
    STATE_ERROR
};

struct Spring {
    float current;
    float target;
    float velocity;
    float stiffness = 250.0f;
    float damping = 24.0f;
    float mass = 1.0f;

    void update(float dt) {
        float force = -stiffness * (current - target) - damping * velocity;
        velocity += (force / mass) * dt;
        current += velocity * dt;
    }
};

class AuraSetupApp {
public:
    HWND hwnd = nullptr;
    SetupState state = STATE_LAUNCH_DROP;
    bool isUpdate = false;
    bool silentMode = false;
    std::wstring customDir;
    std::wstring installDir = L"C:\\Program Files\\Aura Island";
    bool optStartWithWindows = true;
    bool optDesktopShortcut = true;
    std::wstring errorMessage;
    std::wstring logFilePath;

    Spring springWidth;
    Spring springHeight;
    Spring springRadius;
    float contentAlpha = 0.0f;
    float helloAlpha = 0.0f;
    float welcomeAlpha = 0.0f;
    float progress = 0.0f;
    float targetProgress = 0.0f;
    std::atomic<bool> installCompleted{false};
    std::atomic<int> installExitCode{0};
    std::atomic<bool> installCancelled{false};

    float mascotBlink = 1.0f; // 1.0 = open, 0.0 = closed
    float mascotPupilX = 0.0f;
    float mascotPupilY = 0.0f;
    int tickCount = 0;

    // Hover and press tracking
    bool btnInstallHovered = false;
    bool btnInstallPressed = false;
    bool linkOptionsHovered = false;
    bool chkAutostartHovered = false;
    bool chkDesktopHovered = false;
    bool btnBackHovered = false;
    bool btnTryAgainHovered = false;
    bool btnShowLogHovered = false;

    AuraSetupApp() {
        springWidth.current = 148.0f;
        springWidth.target = 148.0f;
        springWidth.velocity = 0.0f;

        springHeight.current = 34.0f;
        springHeight.target = 34.0f;
        springHeight.velocity = 0.0f;

        springRadius.current = 17.0f;
        springRadius.target = 17.0f;
        springRadius.velocity = 0.0f;
    }

    void checkExistingInstall() {
        // Check if C:\Program Files\Aura Island\AuraIsland.exe exists
        DWORD attr = GetFileAttributesW(L"C:\\Program Files\\Aura Island\\AuraIsland.exe");
        if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            isUpdate = true;
            return;
        }

        // Check registry uninstall key
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{C7B38F64-280E-4C2E-8E5A-7FE156A2B3C9}_is1", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            isUpdate = true;
            RegCloseKey(hKey);
        }
    }

    void setState(SetupState newState) {
        state = newState;
        switch (state) {
        case STATE_LAUNCH_DROP:
            springWidth.target = 148.0f;
            springHeight.target = 34.0f;
            springRadius.target = 17.0f;
            break;
        case STATE_LAUNCH_HELLO:
            springWidth.target = 220.0f;
            springHeight.target = 54.0f;
            springRadius.target = 24.0f;
            break;
        case STATE_WELCOME:
            springWidth.target = 420.0f;
            springHeight.target = 180.0f;
            springRadius.target = 32.0f;
            break;
        case STATE_OPTIONS:
            springWidth.target = 440.0f;
            springHeight.target = 220.0f;
            springRadius.target = 32.0f;
            break;
        case STATE_INSTALLING:
            springWidth.target = 360.0f;
            springHeight.target = 100.0f;
            springRadius.target = 28.0f;
            break;
        case STATE_DONE:
            springWidth.target = 280.0f;
            springHeight.target = 60.0f;
            springRadius.target = 24.0f;
            break;
        case STATE_SHRINKING:
            springWidth.target = 148.0f;
            springHeight.target = 34.0f;
            springRadius.target = 17.0f;
            break;
        case STATE_ERROR:
            springWidth.target = 380.0f;
            springHeight.target = 140.0f;
            springRadius.target = 28.0f;
            break;
        }
    }

    bool extractEngine(std::wstring &extractedPath) {
        HMODULE hMod = GetModuleHandleW(NULL);
        HRSRC hRes = FindResourceW(hMod, MAKEINTRESOURCEW(IDR_AURA_ENGINE), RT_RCDATA);
        if (!hRes) return false;

        HGLOBAL hMem = LoadResource(hMod, hRes);
        if (!hMem) return false;

        DWORD size = SizeofResource(hMod, hRes);
        void *pData = LockResource(hMem);
        if (!pData || size == 0) return false;

        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);

        std::wstring setupDir = std::wstring(tempPath) + L"AuraSetup-" + std::to_wstring(GetTickCount());
        CreateDirectoryW(setupDir.c_str(), NULL);

        extractedPath = setupDir + L"\\AuraEngine.exe";
        HANDLE hFile = CreateFileW(extractedPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) return false;

        DWORD written = 0;
        WriteFile(hFile, pData, size, &written, NULL);
        CloseHandle(hFile);
        return (written == size);
    }

    void startInstallation() {
        setState(STATE_INSTALLING);
        progress = 0.05f;
        targetProgress = 0.10f;
        installCompleted = false;
        installCancelled = false;
        installExitCode = 0;

        wchar_t tempPath[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPath);
        logFilePath = std::wstring(tempPath) + L"AuraSetup.log";

        std::thread([this]() {
            std::wstring engineExe;
            if (!extractEngine(engineExe)) {
                errorMessage = L"Failed to unpack setup engine.";
                installExitCode = 1;
                installCompleted = true;
                return;
            }

            std::wstring tasks;
            if (optDesktopShortcut) tasks += L"desktopicon,";
            if (optStartWithWindows) tasks += L"autostart,";
            if (!tasks.empty() && tasks.back() == L',') tasks.pop_back();

            std::wstring params = L"/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /CLOSEAPPLICATIONS";
            params += L" /DIR=\"" + installDir + L"\"";
            if (!tasks.empty()) {
                params += L" /TASKS=\"" + tasks + L"\"";
            }
            params += L" /LOG=\"" + logFilePath + L"\"";

            SHELLEXECUTEINFOW sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
            sei.lpVerb = L"runas"; // Section B.3: admin prompt appears here only
            sei.lpFile = engineExe.c_str();
            sei.lpParameters = params.c_str();
            sei.nShow = SW_HIDE;

            targetProgress = 0.25f;
            if (!ShellExecuteExW(&sei)) {
                DWORD err = GetLastError();
                if (err == ERROR_CANCELLED) {
                    installCancelled = true;
                    errorMessage = L"Setup was cancelled.";
                } else {
                    errorMessage = L"Failed to start installer with admin privileges.";
                }
                installExitCode = err;
                installCompleted = true;
                return;
            }

            targetProgress = 0.50f;
            while (sei.hProcess) {
                DWORD wait = WaitForSingleObject(sei.hProcess, 100);
                if (wait == WAIT_OBJECT_0) {
                    DWORD code = 0;
                    GetExitCodeProcess(sei.hProcess, &code);
                    CloseHandle(sei.hProcess);
                    installExitCode = code;
                    if (code != 0) {
                        errorMessage = L"Installer failed with exit code " + std::to_wstring(code);
                    }
                    installCompleted = true;
                    break;
                }
                if (targetProgress < 0.90f) {
                    targetProgress += 0.05f;
                }
            }
        }).detach();
    }

    void launchInstalledAppAndExit() {
        // Section B.4: start installed app WITHOUT elevation from non-elevated stub
        std::wstring exePath = installDir + L"\\AuraIsland.exe";

        // Use ShellExecuteW with NULL verb to start as standard non-admin user
        ShellExecuteW(NULL, L"open", exePath.c_str(), L"--first-run", NULL, SW_SHOWNORMAL);

        // Delay 400ms then destroy window so app takes over smoothly
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            PostMessageW(hwnd, WM_CLOSE, 0, 0);
        }).detach();
    }
};

static AuraSetupApp g_app;

static void DrawNotchPath(GraphicsPath &path, float x, float y, float w, float h, float r) {
    // Solid notch shape: top flat flush with screen, smooth flare out at top corners, rounded bottom
    float flare = 10.0f;
    // Top-left flare
    path.AddBezier(x - flare, y, x - flare / 2.0f, y + 2.0f, x, y + 6.0f, x, y + 10.0f);
    // Left edge down
    path.AddLine(x, y + 10.0f, x, y + h - r);
    // Bottom-left arc
    path.AddArc(x, y + h - r * 2.0f, r * 2.0f, r * 2.0f, 180.0f, -90.0f);
    // Bottom edge
    path.AddLine(x + r, y + h, x + w - r, y + h);
    // Bottom-right arc
    path.AddArc(x + w - r * 2.0f, y + h - r * 2.0f, r * 2.0f, r * 2.0f, 90.0f, -90.0f);
    // Right edge up
    path.AddLine(x + w, y + h - r, x + w, y + 10.0f);
    // Top-right flare
    path.AddBezier(x + w, y + 10.0f, x + w, y + 6.0f, x + w + flare / 2.0f, y + 2.0f, x + w + flare, y);
    path.CloseFigure();
}

static void RenderStubUI(HDC hdcMem, int width, int height) {
    Graphics g(hdcMem);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    // Clear transparent
    g.Clear(Color(0, 0, 0, 0));

    float winW = g_app.springWidth.current;
    float winH = g_app.springHeight.current;
    float winR = g_app.springRadius.current;

    float x = (width - winW) / 2.0f;
    float y = 0.0f;

    // 1. Draw solid black notch pill
    GraphicsPath notchPath;
    DrawNotchPath(notchPath, x, y, winW, winH, winR);
    SolidBrush blackBrush(Color(255, 0, 0, 0));
    g.FillPath(&blackBrush, &notchPath);

    // Subtle dark hairline border (1px)
    Pen borderPen(Color(40, 255, 255, 255), 1.0f);
    g.DrawPath(&borderPen, &notchPath);

    // Fonts: Segoe UI Variable fallback to Segoe UI
    const wchar_t *fontName = L"Segoe UI Variable Display";
    {
        FontFamily test(fontName);
        if (test.GetLastStatus() != Ok) {
            fontName = L"Segoe UI";
        }
    }
    FontFamily fontFamily(fontName);

    Font fontTitle(&fontFamily, 16.0f, FontStyleBold, UnitPixel);
    Font fontSub(&fontFamily, 12.0f, FontStyleRegular, UnitPixel);
    Font fontBtn(&fontFamily, 13.0f, FontStyleBold, UnitPixel);
    Font fontSmall(&fontFamily, 11.0f, FontStyleRegular, UnitPixel);

    StringFormat centerFormat;
    centerFormat.SetAlignment(StringAlignmentCenter);
    centerFormat.SetLineAlignment(StringAlignmentCenter);

    StringFormat leftFormat;
    leftFormat.SetAlignment(StringAlignmentNear);
    leftFormat.SetLineAlignment(StringAlignmentCenter);

    // 2. Mascot Eyes at Top Center
    if (g_app.state != STATE_DONE && g_app.state != STATE_SHRINKING) {
        float centerX = width / 2.0f;
        float eyeY = 12.0f;
        float eyeSpacing = 7.0f;
        float eyeW = 4.5f;
        float eyeH = 6.0f * g_app.mascotBlink;

        SolidBrush eyeWhite(Color(255, 255, 255, 255));
        if (eyeH > 0.8f) {
            // Left eye
            g.FillEllipse(&eyeWhite, centerX - eyeSpacing - eyeW / 2.0f + g_app.mascotPupilX,
                          eyeY - eyeH / 2.0f + g_app.mascotPupilY, eyeW, eyeH);
            // Right eye
            g.FillEllipse(&eyeWhite, centerX + eyeSpacing - eyeW / 2.0f + g_app.mascotPupilX,
                          eyeY - eyeH / 2.0f + g_app.mascotPupilY, eyeW, eyeH);
        }
    }

    // 3. Screen content depending on state
    if (g_app.state == STATE_LAUNCH_HELLO) {
        SolidBrush textBrush(Color((BYTE)(g_app.helloAlpha * 255), 255, 255, 255));
        RectF r(x, y + 16, winW, winH - 16);
        g.DrawString(L"Hello", -1, &fontTitle, r, &centerFormat, &textBrush);
    }
    else if (g_app.state == STATE_WELCOME) {
        BYTE alpha = (BYTE)(g_app.welcomeAlpha * 255);
        SolidBrush titleBrush(Color(alpha, 255, 255, 255));
        SolidBrush subBrush(Color(alpha, 160, 160, 160));

        RectF titleRect(x, y + 26, winW, 28);
        g.DrawString(L"Welcome to Aura Island", -1, &fontTitle, titleRect, &centerFormat, &titleBrush);

        RectF subRect(x + 20, y + 56, winW - 40, 36);
        std::wstring subtitle = g_app.isUpdate
            ? L"A new version is ready to be installed."
            : L"Your dynamic desktop productivity notch.";
        g.DrawString(subtitle.c_str(), -1, &fontSub, subRect, &centerFormat, &subBrush);

        // Primary Button: "Install" or "Update"
        float btnW = 160.0f;
        float btnH = 36.0f;
        float btnX = (width - btnW) / 2.0f;
        float btnY = y + 104.0f;

        GraphicsPath btnPath;
        btnPath.AddArc(btnX, btnY, btnH, btnH, 90.0f, 180.0f);
        btnPath.AddArc(btnX + btnW - btnH, btnY, btnH, btnH, 270.0f, 180.0f);
        btnPath.CloseFigure();

        Color btnColor = g_app.btnInstallPressed
            ? Color(alpha, 0, 100, 210)
            : (g_app.btnInstallHovered ? Color(alpha, 20, 140, 255) : Color(alpha, 10, 132, 255));
        SolidBrush btnBrush(btnColor);
        g.FillPath(&btnBrush, &btnPath);

        SolidBrush btnTextBrush(Color(alpha, 255, 255, 255));
        RectF btnRect(btnX, btnY, btnW, btnH);
        g.DrawString(g_app.isUpdate ? L"Update" : L"Install", -1, &fontBtn, btnRect, &centerFormat, &btnTextBrush);

        // Options Link
        RectF linkRect(x, y + 148, winW, 20);
        SolidBrush linkBrush(g_app.linkOptionsHovered ? Color(alpha, 255, 255, 255) : Color(alpha, 140, 140, 140));
        g.DrawString(L"Options", -1, &fontSmall, linkRect, &centerFormat, &linkBrush);
    }
    else if (g_app.state == STATE_OPTIONS) {
        SolidBrush titleBrush(Color(255, 255, 255, 255));
        SolidBrush textBrush(Color(255, 220, 220, 220));
        SolidBrush mutedBrush(Color(255, 140, 140, 140));

        RectF titleRect(x + 24, y + 22, winW - 48, 24);
        g.DrawString(L"Installation Options", -1, &fontTitle, titleRect, &leftFormat, &titleBrush);

        // Path label
        RectF dirLabelRect(x + 24, y + 54, winW - 48, 18);
        g.DrawString(L"Install location:", -1, &fontSmall, dirLabelRect, &leftFormat, &mutedBrush);

        // Path box
        RectF dirBox(x + 24, y + 74, winW - 48, 28);
        SolidBrush boxBg(Color(255, 30, 30, 30));
        Pen boxBorder(Color(255, 60, 60, 60), 1.0f);
        g.FillRectangle(&boxBg, dirBox);
        g.DrawRectangle(&boxBorder, dirBox.X, dirBox.Y, dirBox.Width, dirBox.Height);

        RectF dirTextRect(x + 32, y + 74, winW - 64, 28);
        g.DrawString(g_app.installDir.c_str(), -1, &fontSmall, dirTextRect, &leftFormat, &textBrush);

        // Checkbox: Start with Windows
        float chkY1 = y + 114.0f;
        RectF chk1Box(x + 24, chkY1, 16, 16);
        SolidBrush chk1Bg(g_app.optStartWithWindows ? Color(255, 10, 132, 255) : Color(255, 40, 40, 40));
        g.FillRectangle(&chk1Bg, chk1Box);
        if (g_app.optStartWithWindows) {
            SolidBrush white(Color(255, 255, 255, 255));
            g.DrawString(L"v", -1, &fontSmall, chk1Box, &centerFormat, &white);
        }
        RectF chk1Text(x + 48, chkY1, winW - 72, 16);
        g.DrawString(L"Start Aura Island when Windows starts", -1, &fontSub, chk1Text, &leftFormat, &textBrush);

        // Checkbox: Desktop shortcut
        float chkY2 = y + 138.0f;
        RectF chk2Box(x + 24, chkY2, 16, 16);
        SolidBrush chk2Bg(g_app.optDesktopShortcut ? Color(255, 10, 132, 255) : Color(255, 40, 40, 40));
        g.FillRectangle(&chk2Bg, chk2Box);
        if (g_app.optDesktopShortcut) {
            SolidBrush white(Color(255, 255, 255, 255));
            g.DrawString(L"v", -1, &fontSmall, chk2Box, &centerFormat, &white);
        }
        RectF chk2Text(x + 48, chkY2, winW - 72, 16);
        g.DrawString(L"Create desktop shortcut", -1, &fontSub, chk2Text, &leftFormat, &textBrush);

        // Buttons: Back (left), Install (right)
        RectF backRect(x + 24, y + 172, 80, 32);
        SolidBrush backBrush(g_app.btnBackHovered ? Color(255, 255, 255, 255) : Color(255, 160, 160, 160));
        g.DrawString(L"Back", -1, &fontBtn, backRect, &leftFormat, &backBrush);

        float btnW = 140.0f;
        float btnH = 34.0f;
        float btnX = x + winW - btnW - 24.0f;
        float btnY = y + 170.0f;

        GraphicsPath btnPath;
        btnPath.AddArc(btnX, btnY, btnH, btnH, 90.0f, 180.0f);
        btnPath.AddArc(btnX + btnW - btnH, btnY, btnH, btnH, 270.0f, 180.0f);
        btnPath.CloseFigure();

        SolidBrush btnBrush(Color(255, 10, 132, 255));
        g.FillPath(&btnBrush, &btnPath);
        SolidBrush white(Color(255, 255, 255, 255));
        RectF bR(btnX, btnY, btnW, btnH);
        g.DrawString(g_app.isUpdate ? L"Update" : L"Install", -1, &fontBtn, bR, &centerFormat, &white);
    }
    else if (g_app.state == STATE_INSTALLING) {
        SolidBrush titleBrush(Color(255, 255, 255, 255));
        RectF titleRect(x, y + 26, winW, 24);
        g.DrawString(L"Setting things up...", -1, &fontTitle, titleRect, &centerFormat, &titleBrush);

        // Progress bar
        float barW = winW - 60.0f;
        float barH = 6.0f;
        float barX = (width - barW) / 2.0f;
        float barY = y + 64.0f;

        SolidBrush barBg(Color(255, 40, 40, 40));
        g.FillRectangle(&barBg, barX, barY, barW, barH);

        SolidBrush barFill(Color(255, 10, 132, 255));
        float fillW = barW * g_app.progress;
        if (fillW < 4.0f) fillW = 4.0f;
        g.FillRectangle(&barFill, barX, barY, fillW, barH);
    }
    else if (g_app.state == STATE_DONE || g_app.state == STATE_SHRINKING) {
        SolidBrush titleBrush(Color(255, 255, 255, 255));
        // Check mark and "All set"
        RectF doneRect(x, y + 16, winW, 28);
        g.DrawString(L"All set", -1, &fontTitle, doneRect, &centerFormat, &titleBrush);
    }
    else if (g_app.state == STATE_ERROR) {
        SolidBrush errBrush(Color(255, 255, 80, 80));
        SolidBrush textBrush(Color(255, 200, 200, 200));

        RectF titleRect(x, y + 20, winW, 24);
        g.DrawString(g_app.installCancelled ? L"Setup was cancelled" : L"Something went wrong",
                     -1, &fontTitle, titleRect, &centerFormat, &errBrush);

        RectF msgRect(x + 20, y + 48, winW - 40, 32);
        g.DrawString(g_app.errorMessage.c_str(), -1, &fontSmall, msgRect, &centerFormat, &textBrush);

        // "Try again" button
        float btnW = 120.0f;
        float btnH = 30.0f;
        float btnX = width / 2.0f - btnW - 10.0f;
        float btnY = y + 90.0f;

        GraphicsPath btnPath;
        btnPath.AddArc(btnX, btnY, btnH, btnH, 90.0f, 180.0f);
        btnPath.AddArc(btnX + btnW - btnH, btnY, btnH, btnH, 270.0f, 180.0f);
        btnPath.CloseFigure();

        SolidBrush btnBrush(Color(255, 10, 132, 255));
        g.FillPath(&btnBrush, &btnPath);
        SolidBrush white(Color(255, 255, 255, 255));
        RectF bR(btnX, btnY, btnW, btnH);
        g.DrawString(L"Try again", -1, &fontBtn, bR, &centerFormat, &white);

        // "Show log" button
        float logX = width / 2.0f + 10.0f;
        GraphicsPath logPath;
        logPath.AddArc(logX, btnY, btnH, btnH, 90.0f, 180.0f);
        logPath.AddArc(logX + btnW - btnH, btnY, btnH, btnH, 270.0f, 180.0f);
        logPath.CloseFigure();

        SolidBrush logBrush(Color(255, 50, 50, 50));
        g.FillPath(&logBrush, &logPath);
        RectF lR(logX, btnY, btnW, btnH);
        g.DrawString(L"Show log", -1, &fontBtn, lR, &centerFormat, &white);
    }
}

static void UpdateLayeredPillWindow(HWND hwnd) {
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoW(hMon, &mi);

    int screenW = mi.rcMonitor.right - mi.rcMonitor.left;
    int maxW = 500;
    int maxH = 260;

    int winX = mi.rcMonitor.left + (screenW - maxW) / 2;
    int winY = mi.rcMonitor.top;

    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = maxW;
    bmi.bmiHeader.biHeight = -maxH; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *pBits = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

    RenderStubUI(hdcMem, maxW, maxH);

    POINT ptSrc = { 0, 0 };
    SIZE sizeWnd = { maxW, maxH };
    POINT ptDst = { winX, winY };
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, 1, 16, NULL); // ~60 FPS animation loop
        return 0;

    case WM_TIMER: {
        float dt = 0.016f;
        g_app.springWidth.update(dt);
        g_app.springHeight.update(dt);
        g_app.springRadius.update(dt);

        g_app.tickCount++;

        // Mascot eye blinking animation: blink every ~200 ticks
        if ((g_app.tickCount % 220) < 10) {
            g_app.mascotBlink = 0.0f; // closed
        } else {
            g_app.mascotBlink = 1.0f; // open
        }

        // State progression on start
        if (g_app.state == STATE_LAUNCH_DROP) {
            if (g_app.tickCount > 25) {
                g_app.setState(STATE_LAUNCH_HELLO);
            }
        } else if (g_app.state == STATE_LAUNCH_HELLO) {
            if (g_app.helloAlpha < 1.0f) g_app.helloAlpha += 0.06f;
            if (g_app.tickCount > 75) {
                g_app.setState(STATE_WELCOME);
            }
        } else if (g_app.state == STATE_WELCOME) {
            if (g_app.welcomeAlpha < 1.0f) g_app.welcomeAlpha += 0.06f;
        } else if (g_app.state == STATE_INSTALLING) {
            // Smooth progress interpolation
            g_app.progress += (g_app.targetProgress - g_app.progress) * 0.12f;

            if (g_app.installCompleted) {
                if (g_app.installExitCode == 0 && !g_app.installCancelled) {
                    g_app.progress = 1.0f;
                    g_app.setState(STATE_DONE);
                } else {
                    g_app.setState(STATE_ERROR);
                }
            }
        } else if (g_app.state == STATE_DONE) {
            if (g_app.tickCount % 60 == 0) { // After ~1 second on "All set"
                g_app.setState(STATE_SHRINKING);
                g_app.launchInstalledAppAndExit();
            }
        }

        UpdateLayeredPillWindow(hwnd);
        return 0;
    }

    case WM_MOUSEMOVE: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        // Update pupil tracking relative to center
        float cx = 250.0f;
        float cy = 12.0f;
        float dx = (mouseX - cx) / 100.0f;
        float dy = (mouseY - cy) / 100.0f;
        g_app.mascotPupilX = (dx < -1.5f) ? -1.5f : ((dx > 1.5f) ? 1.5f : dx);
        g_app.mascotPupilY = (dy < -1.0f) ? -1.0f : ((dy > 1.0f) ? 1.0f : dy);

        // Check hover zones
        if (g_app.state == STATE_WELCOME) {
            float btnX = (500 - 160) / 2.0f;
            float btnY = 104.0f;
            g_app.btnInstallHovered = (mouseX >= btnX && mouseX <= btnX + 160 && mouseY >= btnY && mouseY <= btnY + 36);
            g_app.linkOptionsHovered = (mouseX >= 180 && mouseX <= 320 && mouseY >= 145 && mouseY <= 170);
        } else if (g_app.state == STATE_OPTIONS) {
            g_app.btnBackHovered = (mouseX >= 50 && mouseX <= 130 && mouseY >= 165 && mouseY <= 205);
            g_app.chkAutostartHovered = (mouseX >= 50 && mouseX <= 400 && mouseY >= 110 && mouseY <= 130);
            g_app.chkDesktopHovered = (mouseX >= 50 && mouseX <= 400 && mouseY >= 135 && mouseY <= 155);
        } else if (g_app.state == STATE_ERROR) {
            g_app.btnTryAgainHovered = (mouseX >= 120 && mouseX <= 240 && mouseY >= 85 && mouseY <= 125);
            g_app.btnShowLogHovered = (mouseX >= 260 && mouseX <= 380 && mouseY >= 85 && mouseY <= 125);
        }
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (g_app.state == STATE_WELCOME) {
            float btnX = (500 - 160) / 2.0f;
            float btnY = 104.0f;
            if (mouseX >= btnX && mouseX <= btnX + 160 && mouseY >= btnY && mouseY <= btnY + 36) {
                g_app.btnInstallPressed = true;
            }
        }
        return 0;
    }

    case WM_LBUTTONUP: {
        int mouseX = LOWORD(lParam);
        int mouseY = HIWORD(lParam);

        if (g_app.state == STATE_WELCOME) {
            float btnX = (500 - 160) / 2.0f;
            float btnY = 104.0f;
            if (g_app.btnInstallPressed && mouseX >= btnX && mouseX <= btnX + 160 && mouseY >= btnY && mouseY <= btnY + 36) {
                g_app.btnInstallPressed = false;
                g_app.startInstallation();
                return 0;
            }
            g_app.btnInstallPressed = false;

            if (mouseX >= 180 && mouseX <= 320 && mouseY >= 145 && mouseY <= 170) {
                g_app.setState(STATE_OPTIONS);
                return 0;
            }
        }
        else if (g_app.state == STATE_OPTIONS) {
            // Back button
            if (mouseX >= 50 && mouseX <= 130 && mouseY >= 165 && mouseY <= 205) {
                g_app.setState(STATE_WELCOME);
                return 0;
            }
            // Checkbox 1
            if (mouseX >= 50 && mouseX <= 400 && mouseY >= 110 && mouseY <= 130) {
                g_app.optStartWithWindows = !g_app.optStartWithWindows;
                return 0;
            }
            // Checkbox 2
            if (mouseX >= 50 && mouseX <= 400 && mouseY >= 135 && mouseY <= 155) {
                g_app.optDesktopShortcut = !g_app.optDesktopShortcut;
                return 0;
            }
            // Install button from options
            if (mouseX >= 310 && mouseX <= 450 && mouseY >= 165 && mouseY <= 205) {
                g_app.startInstallation();
                return 0;
            }
        }
        else if (g_app.state == STATE_ERROR) {
            // Try again
            if (mouseX >= 120 && mouseX <= 240 && mouseY >= 85 && mouseY <= 125) {
                g_app.startInstallation();
                return 0;
            }
            // Show log
            if (mouseX >= 260 && mouseX <= 380 && mouseY >= 85 && mouseY <= 125) {
                if (!g_app.logFilePath.empty()) {
                    ShellExecuteW(NULL, L"open", g_app.logFilePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
                }
                return 0;
            }
        }
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int) {
    // 1. Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // 2. Parse command line for /silent and /dir
    std::wstring cmd(lpCmdLine);
    if (cmd.find(L"/silent") != std::wstring::npos || cmd.find(L"/SILENT") != std::wstring::npos ||
        cmd.find(L"/verysilent") != std::wstring::npos || cmd.find(L"/VERYSILENT") != std::wstring::npos) {
        g_app.silentMode = true;
    }

    size_t dirPos = cmd.find(L"/dir=");
    if (dirPos == std::wstring::npos) dirPos = cmd.find(L"/DIR=");
    if (dirPos != std::wstring::npos) {
        size_t start = dirPos + 5;
        if (start < cmd.size() && cmd[start] == L'\"') {
            start++;
            size_t end = cmd.find(L'\"', start);
            if (end != std::wstring::npos) {
                g_app.installDir = cmd.substr(start, end - start);
            }
        } else {
            size_t end = cmd.find(L' ', start);
            g_app.installDir = (end == std::wstring::npos) ? cmd.substr(start) : cmd.substr(start, end - start);
        }
    }

    // If /silent mode requested: execute Inno engine directly and exit
    if (g_app.silentMode) {
        std::wstring engineExe;
        if (g_app.extractEngine(engineExe)) {
            std::wstring params = L"/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /CLOSEAPPLICATIONS /DIR=\"" + g_app.installDir + L"\"";
            SHELLEXECUTEINFOW sei = { 0 };
            sei.cbSize = sizeof(sei);
            sei.fMask = SEE_MASK_NOCLOSEPROCESS;
            sei.lpVerb = L"runas";
            sei.lpFile = engineExe.c_str();
            sei.lpParameters = params.c_str();
            sei.nShow = SW_HIDE;
            if (ShellExecuteExW(&sei)) {
                WaitForSingleObject(sei.hProcess, INFINITE);
                DWORD code = 0;
                GetExitCodeProcess(sei.hProcess, &code);
                CloseHandle(sei.hProcess);
                GdiplusShutdown(gdiplusToken);
                return static_cast<int>(code);
            }
        }
        GdiplusShutdown(gdiplusToken);
        return 1;
    }

    g_app.checkExistingInstall();

    // 3. Register Window Class
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"AuraSetupStubPillWindow";
    RegisterClassExW(&wc);

    // 4. Create Layered, Topmost, Toolwindow
    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"Aura Island Setup",
        WS_POPUP,
        0, 0, 500, 260,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) {
        GdiplusShutdown(gdiplusToken);
        return 1;
    }

    g_app.hwnd = hwnd;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // 5. Message Loop
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}
