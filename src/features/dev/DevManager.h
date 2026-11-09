#pragma once

#include <QObject>
#include "GitWatcher.h"
#include "PomodoroTimer.h"
#include "AgentHookServer.h"

namespace Aura {

class DevManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(GitWatcher* git READ git CONSTANT)
    Q_PROPERTY(PomodoroTimer* pomodoro READ pomodoro CONSTANT)
    Q_PROPERTY(AgentHookServer* agentHook READ agentHook CONSTANT)

public:
    static DevManager &instance();

    GitWatcher *git() { return &m_git; }
    PomodoroTimer *pomodoro() { return &m_pomodoro; }
    AgentHookServer *agentHook() { return &m_agentHook; }

    Q_INVOKABLE void selectRepoPath(const QString &path);

private:
    explicit DevManager(QObject *parent = nullptr);

    GitWatcher m_git;
    PomodoroTimer m_pomodoro;
    AgentHookServer m_agentHook;
};

} // namespace Aura
