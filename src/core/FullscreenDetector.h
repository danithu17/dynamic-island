#pragma once

#include <QObject>
#include <QTimer>

namespace Aura {

class FullscreenDetector : public QObject {
    Q_OBJECT
public:
    explicit FullscreenDetector(QObject *parent = nullptr);
    ~FullscreenDetector() override;

    bool isFullscreenAppActive() const { return m_isFullscreen; }
    void start(int intervalMs = 1500);
    void stop();

signals:
    void fullscreenStateChanged(bool isFullscreen);

private slots:
    void checkState();

private:
    QTimer *m_timer = nullptr;
    bool m_isFullscreen = false;
};

} // namespace Aura
