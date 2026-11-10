#pragma once

#include <QQuickWindow>
#include <QTimer>
#include <QScreen>
#include <QRect>
#include <windows.h>

namespace Aura {

class IslandWindow : public QQuickWindow {
    Q_OBJECT
    Q_PROPERTY(bool isHovered READ isHovered NOTIFY hoverChanged)

public:
    explicit IslandWindow(QWindow *parent = nullptr);
    ~IslandWindow() override;

    HWND hwnd() const { return m_hwnd; }
    bool isHovered() const { return m_isHovered; }

    void initializeNativeWindow();
    void repositionToTopCenter(int targetWidth = 540, int targetHeight = 360);

    Q_INVOKABLE void updatePillBounds(int x, int y, int width, int height);
    Q_INVOKABLE void requestWindowFocus();
    Q_INVOKABLE void releaseWindowFocus();

public slots:
    void onWindowResizeRequested(int width, int height, int radius);
    void onFullscreenChanged(bool isFullscreen);
    void onDisplayConfigurationChanged();

signals:
    void nativeWindowReady(HWND hwnd);
    void hoverChanged();

protected:
    void exposeEvent(QExposeEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void checkCursorProximity();

private:
    HWND m_hwnd = nullptr;
    int m_currentRadius = 16;
    bool m_isExposedFirstTime = false;
    bool m_isFullscreenSuppressed = false;
    QTimer *m_proximityTimer = nullptr;
    bool m_isHovered = false;
    QRect m_pillBounds{210, 8, 120, 32};
};

} // namespace Aura
