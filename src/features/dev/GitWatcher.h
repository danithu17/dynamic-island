#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QProcess>

namespace Aura {

class GitWatcher : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString repoPath READ repoPath WRITE setRepoPath NOTIFY repoPathChanged)
    Q_PROPERTY(QString branch READ branch NOTIFY statusChanged)
    Q_PROPERTY(int uncommittedCount READ uncommittedCount NOTIFY statusChanged)

public:
    explicit GitWatcher(QObject *parent = nullptr);

    QString repoPath() const { return m_repoPath; }
    void setRepoPath(const QString &path);

    QString branch() const { return m_branch; }
    int uncommittedCount() const { return m_uncommittedCount; }

    Q_INVOKABLE void refresh();

signals:
    void repoPathChanged();
    void statusChanged();

private slots:
    void pollStatus();

private:
    QString m_repoPath;
    QString m_branch = "-";
    int m_uncommittedCount = 0;
    QTimer *m_timer = nullptr;
    bool m_isRunning = false;
};

} // namespace Aura
