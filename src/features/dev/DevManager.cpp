#include "DevManager.h"
#include "../../core/Settings.h"

namespace Aura {

DevManager &DevManager::instance() {
    static DevManager s_instance;
    return s_instance;
}

DevManager::DevManager(QObject *parent)
    : QObject(parent) {
    // Start local agent hook server with configured port and token
    auto &settings = Settings::instance();
    m_agentHook.startServer(settings.agentServerPort(), settings.agentServerToken());
}

void DevManager::selectRepoPath(const QString &path) {
    m_git.setRepoPath(path);
}

} // namespace Aura
