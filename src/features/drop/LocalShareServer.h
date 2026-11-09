#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QString>
#include <QByteArray>

namespace Aura {

class LocalShareServer : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(QString shareUrl READ shareUrl NOTIFY shareUrlChanged)
    Q_PROPERTY(QString qrSvgPath READ qrSvgPath NOTIFY qrSvgPathChanged)
    Q_PROPERTY(QString sharedName READ sharedName NOTIFY sharedNameChanged)
    Q_PROPERTY(int remainingSeconds READ remainingSeconds NOTIFY remainingSecondsChanged)

public:
    explicit LocalShareServer(QObject *parent = nullptr);
    ~LocalShareServer() override;

    bool isRunning() const { return m_isRunning; }
    QString shareUrl() const { return m_shareUrl; }
    QString qrSvgPath() const { return m_qrSvgPath; }
    QString sharedName() const { return m_sharedName; }
    int remainingSeconds() const { return m_remainingSeconds; }

    bool startShare(const QString &filePath, const QString &textData, const QString &displayName);
    Q_INVOKABLE void stopShare();

signals:
    void runningChanged();
    void shareUrlChanged();
    void qrSvgPathChanged();
    void sharedNameChanged();
    void remainingSecondsChanged();
    void downloadCompleted();
    void expired();

private slots:
    void onNewConnection();
    void onTick();

private:
    QString getLanIpAddress() const;
    void cleanup();

    QTcpServer *m_server = nullptr;
    QTimer *m_countdownTimer = nullptr;
    bool m_isRunning = false;
    QString m_token;
    QString m_shareUrl;
    QString m_qrSvgPath;
    QString m_sharedName;
    QString m_sourceFilePath;
    QByteArray m_textPayload;
    int m_remainingSeconds = 300;
};

} // namespace Aura
