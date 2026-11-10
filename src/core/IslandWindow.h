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
    void repositionToTopCenter(int targetWidth = 148, int targetHeight = 34);

    Q_INVOKABLE void requestWindowFocus();
    Q_INVOKABLE void releaseWindowFocus();
    Q_INVOKABLE void handleEscKey();

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
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void checkCursorProximity();
    void onHoverExpandTriggered();
    void onCollapseTimerTriggered();

private:
    HWND m_hwnd = nullptr;
    HWND m_prevForegroundHwnd = nullptr;
    int m_currentRadius = 17;
    bool m_isExposedFirstTime = false;
    bool m_isFullscreenSuppressed = false;
    QTimer *m_proximityTimer = nullptr;
    QTimer *m_hoverExpandTimer = nullptr;
    QTimer *m_collapseDelayTimer = nullptr;
    bool m_isHovered = false;
    int m_targetWidth = 148;
    int m_targetHeight = 34;
};

} // namespace Aura
