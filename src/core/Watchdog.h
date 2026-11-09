#pragma once

#include <QStringList>

namespace Aura {

class Watchdog {
public:
    static bool isWorkerProcess(int argc, char *argv[]);
    static int runSupervisor(int argc, char *argv[]);
};

} // namespace Aura
