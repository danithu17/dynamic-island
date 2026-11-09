#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include "LocalShareServer.h"

namespace Aura {

class DropManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(LocalShareServer* shareServer READ shareServer CONSTANT)
    Q_PROPERTY(bool isQrActive READ isQrActive NOTIFY qrActiveChanged)
    Q_PROPERTY(QString lastActionMessage READ lastActionMessage NOTIFY lastActionMessageChanged)

public:
    static DropManager &instance();

    LocalShareServer *shareServer() { return &m_shareServer; }
    bool isQrActive() const { return m_shareServer.isRunning(); }
    QString lastActionMessage() const { return m_lastActionMessage; }

    Q_INVOKABLE void handleDropMail(const QString &data, const QString &type, const QString &name);
    Q_INVOKABLE void handleDropSave(const QString &data, const QString &type, const QString &name);
    Q_INVOKABLE void handleDropPhone(const QString &data, const QString &type, const QString &name);
    Q_INVOKABLE void dismissQr();

signals:
    void qrActiveChanged();
    void lastActionMessageChanged();
    void dropActionCompleted(const QString &actionName, const QString &toastMsg);

private:
    explicit DropManager(QObject *parent = nullptr);

    LocalShareServer m_shareServer;
    QString m_lastActionMessage;
};

} // namespace Aura
