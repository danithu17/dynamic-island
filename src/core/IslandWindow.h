#pragma once

#include <QQuickWindow>
#include <QTimer>
#include <QScreen>
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
    void repositionToTopCenter(int targetWidth = 134, int targetHeight = 36);

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

private slots:
    void checkCursorProximity();

private:
    HWND m_hwnd = nullptr;
    int m_currentRadius = 16;
    bool m_isExposedFirstTime = false;
    bool m_isFullscreenSuppressed = false;
    QTimer *m_proximityTimer = nullptr;
    QTimer *m_collapseDelayTimer = nullptr;
    bool m_isHovered = false;
    int m_targetWidth = 134;
    int m_targetHeight = 36;
};

} // namespace Aura
