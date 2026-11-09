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
    // Register Ctrl + Alt + Space
    if (RegisterHotKey(hwnd, HOTKEY_ID_TOGGLE, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, VK_SPACE)) {
        m_hotkeyRegistered = true;
    } else {
        qWarning() << "Failed to register global hotkey Ctrl+Alt+Space";
    }
}

void NativeEventFilter::unregisterGlobalHotkey(HWND hwnd) {
    if (m_hotkeyRegistered && hwnd) {
        UnregisterHotKey(hwnd, HOTKEY_ID_TOGGLE);
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
    Q_UNUSED(result);

    MSG *msg = static_cast<MSG *>(message);
    if (!msg) return false;

    if (msg->message == WM_HOTKEY) {
        if (msg->wParam == HOTKEY_ID_TOGGLE) {
            emit hotkeyTriggered();
            return true;
        }
    } else if (msg->message == WM_CLIPBOARDUPDATE) {
        emit clipboardChanged();
        return false;
    } else if (msg->message == WM_DISPLAYCHANGE || msg->message == WM_DPICHANGED) {
        emit displayConfigurationChanged();
        return false;
    }

    return false;
}

} // namespace Aura
