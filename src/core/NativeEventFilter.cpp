#include "NativeEventFilter.h"
#include <QDebug>

namespace Aura {

NativeEventFilter::NativeEventFilter(QObject *parent)
    : QObject(parent) {
}

NativeEventFilter::~NativeEventFilter() {
    if (m_hwnd) {
        unregisterGlobalHotkey(m_hwnd);
        unregisterClipboardListener(m_hwnd);
    }
}

void NativeEventFilter::registerGlobalHotkey(HWND hwnd) {
    if (!hwnd) return;
    m_hwnd = hwnd;
    // Register Ctrl + Alt + Space (Toggle Island)
    RegisterHotKey(hwnd, HOTKEY_ID_TOGGLE, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_SPACE);
    // Register Ctrl + Alt + N (Quick Note capture)
    RegisterHotKey(hwnd, HOTKEY_ID_QUICK_NOTE, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'N');
    // Register Ctrl + Alt + V (Paste Stack pop)
    RegisterHotKey(hwnd, HOTKEY_ID_PASTE_STACK, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'V');
    m_hotkeyRegistered = true;
}

void NativeEventFilter::unregisterGlobalHotkey(HWND hwnd) {
    if (m_hotkeyRegistered && hwnd) {
        UnregisterHotKey(hwnd, HOTKEY_ID_TOGGLE);
        UnregisterHotKey(hwnd, HOTKEY_ID_QUICK_NOTE);
        UnregisterHotKey(hwnd, HOTKEY_ID_PASTE_STACK);
        m_hotkeyRegistered = false;
    }
}

void NativeEventFilter::registerClipboardListener(HWND hwnd) {
    if (!hwnd) return;
    m_hwnd = hwnd;
    if (AddClipboardFormatListener(hwnd)) {
        m_clipboardListenerRegistered = true;
    } else {
        qWarning() << "Failed to add clipboard format listener";
    }
}

void NativeEventFilter::unregisterClipboardListener(HWND hwnd) {
    if (m_clipboardListenerRegistered && hwnd) {
        RemoveClipboardFormatListener(hwnd);
        m_clipboardListenerRegistered = false;
    }
}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
    Q_UNUSED(eventType);

    MSG *msg = static_cast<MSG *>(message);
    if (!msg) return false;

    // Eliminate non-client frame lines / top shadow ears on our window
    if (msg->hwnd == m_hwnd && msg->message == WM_NCCALCSIZE) {
        if (msg->wParam) {
            if (result) *result = 0;
            return true;
        }
    }

    if (msg->message == WM_HOTKEY) {
        if (msg->wParam == HOTKEY_ID_TOGGLE) {
            emit hotkeyTriggered();
            return true;
        } else if (msg->wParam == HOTKEY_ID_QUICK_NOTE) {
            emit quickNoteTriggered();
            return true;
        } else if (msg->wParam == HOTKEY_ID_PASTE_STACK) {
            emit pasteStackTriggered();
            return true;
        }
    } else if (msg->message == WM_CLIPBOARDUPDATE) {
        emit clipboardChanged();
        return false;
    } else if (msg->message == WM_DISPLAYCHANGE || msg->message == WM_DPICHANGED) {
        emit displayConfigurationChanged();
        return false;
    }

    static const UINT s_wmShowIsland = RegisterWindowMessageW(L"WM_AURA_ISLAND_SHOW");
    if (s_wmShowIsland != 0 && msg->message == s_wmShowIsland) {
        emit hotkeyTriggered();
        return true;
    }

    return false;
}

} // namespace Aura
