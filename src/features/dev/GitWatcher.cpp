#include "GitWatcher.h"
#include "../../core/AppState.h"
#include <QDir>
#include <QDebug>

namespace Aura {

GitWatcher::GitWatcher(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this)) {
    connect(m_timer, &QTimer::timeout, this, &GitWatcher::pollStatus);
    m_timer->setInterval(5000); // Check every 5s if active
}

void GitWatcher::setRepoPath(const QString &path) {
    if (m_repoPath != path) {
        m_repoPath = path;
        emit repoPathChanged();
        if (!m_repoPath.isEmpty()) {
            refresh();
            m_timer->start();
        } else {
            m_timer->stop();
            m_branch = "-";
            m_uncommittedCount = 0;
            emit statusChanged();
        }
    }
}

void GitWatcher::refresh() {
    pollStatus();
}

void GitWatcher::pollStatus() {
    if (m_repoPath.isEmpty() || m_isRunning) return;
    QDir d(m_repoPath);
    if (!d.exists() || !d.exists(".git")) {
        m_branch = "Not a git repo";
        m_uncommittedCount = 0;
        emit statusChanged();
        return;
    }

    m_isRunning = true;

    // Run branch check
    QProcess *pBranch = new QProcess(this);
    pBranch->setWorkingDirectory(m_repoPath);
    connect(pBranch, &QProcess::finished, this, [this, pBranch](int exitCode) {
        pBranch->deleteLater();
        if (exitCode == 0) {
            m_branch = QString::fromLocal8Bit(pBranch->readAllStandardOutput()).trimmed();
        } else {
            m_branch = "git error";
        }

        // Run status check
        QProcess *pStatus = new QProcess(this);
        pStatus->setWorkingDirectory(m_repoPath);
        connect(pStatus, &QProcess::finished, this, [this, pStatus](int sExit) {
            pStatus->deleteLater();
            m_isRunning = false;
            if (sExit == 0) {
                QString out = QString::fromLocal8Bit(pStatus->readAllStandardOutput());
                QStringList lines = out.split('\n', Qt::SkipEmptyParts);
                m_uncommittedCount = lines.size();
            } else {
                m_uncommittedCount = 0;
            }

            emit statusChanged();
            AppState::instance().setGitBranch(m_branch);
            AppState::instance().setGitUncommittedCount(m_uncommittedCount);
        });
        pStatus->start("git", QStringList() << "status" << "--porcelain");
    });
    pBranch->start("git", QStringList() << "rev-parse" << "--abbrev-ref" << "HEAD");
}

} // namespace Aura
