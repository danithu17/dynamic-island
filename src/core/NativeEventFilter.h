#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <windows.h>

namespace Aura {

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    static constexpr int HOTKEY_ID_TOGGLE = 0xAA01;

    explicit NativeEventFilter(QObject *parent = nullptr);
    ~NativeEventFilter() override;

    void registerGlobalHotkey(HWND hwnd);
    void unregisterGlobalHotkey(HWND hwnd);

    void registerClipboardListener(HWND hwnd);
    void unregisterClipboardListener(HWND hwnd);

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    void hotkeyTriggered();
    void clipboardChanged();
    void displayConfigurationChanged();

private:
    HWND m_hwnd = nullptr;
    bool m_hotkeyRegistered = false;
    bool m_clipboardListenerRegistered = false;
};

} // namespace Aura
