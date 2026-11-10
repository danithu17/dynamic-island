#include "SingleInstance.h"
#include "Logger.h"
#include <QCoreApplication>
#include <iostream>

namespace Aura {

static const wchar_t *MUTEX_NAME = L"Local\\AuraIsland_SingleInstance_Mutex";
static const wchar_t *PIPE_NAME = L"\\\\.\\pipe\\AuraIsland_Ipc_Pipe";

SingleInstance &SingleInstance::instance() {
    static SingleInstance s_instance;
    return s_instance;
}

SingleInstance::SingleInstance(QObject *parent)
    : QObject(parent) {
}

SingleInstance::~SingleInstance() {
    release();
}

bool SingleInstance::checkAndAcquire(const QStringList &args) {
    Logger::logStep("Checking single-instance mutex and IPC pipe");

    // Formulate command to send if secondary instance
    std::string cmdToSend = "WAKE\n";
    if (args.contains("--show")) {
        cmdToSend = "SHOW\n";
    } else if (args.contains("--first-run")) {
        cmdToSend = "SETUP\n";
    }

    // Try creating or opening mutex
    m_hMutex = CreateMutexW(NULL, FALSE, MUTEX_NAME);
    DWORD lastErr = GetLastError();

    if (lastErr == ERROR_ALREADY_EXISTS) {
        // Mutex exists. Check if an owning instance is actually alive via named pipe.
        bool contactedExisting = false;

        // Try connecting to existing pipe
        for (int attempt = 0; attempt < 3; ++attempt) {
            HANDLE hPipe = CreateFileW(
                PIPE_NAME,
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );

            if (hPipe != INVALID_HANDLE_VALUE) {
                DWORD written = 0;
                WriteFile(hPipe, cmdToSend.c_str(), static_cast<DWORD>(cmdToSend.size()), &written, NULL);
                FlushFileBuffers(hPipe);
                CloseHandle(hPipe);
                contactedExisting = true;
                break;
            }

            if (GetLastError() == ERROR_PIPE_BUSY) {
                WaitNamedPipeW(PIPE_NAME, 200);
            } else {
                Sleep(50);
            }
        }

        if (contactedExisting) {
            Logger::log(QtInfoMsg, "Another running instance of Aura Island was notified. This instance will now exit.");
            if (m_hMutex) {
                CloseHandle(m_hMutex);
                m_hMutex = nullptr;
            }
            return false; // Exit this instance
        }

        // Pipe could not be reached -> Stale lock from crashed or killed instance!
        Logger::log(QtWarningMsg, "Detected stale single-instance lock from crashed process. Recovering lock...");
        DWORD waitRes = WaitForSingleObject(m_hMutex, 500);
        if (waitRes == WAIT_ABANDONED || waitRes == WAIT_OBJECT_0) {
            Logger::log(QtInfoMsg, "Successfully recovered ownership from abandoned lock.");
        }
    }

    // This process is the primary instance
    Logger::log(QtInfoMsg, "Acquired single-instance ownership.");
    startServer();
    return true;
}

void SingleInstance::startServer() {
    if (m_isServerRunning) return;

    m_hStopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    m_isServerRunning = true;
    m_hServerThread = CreateThread(NULL, 0, serverThreadProc, this, 0, NULL);
}

void SingleInstance::release() {
    if (m_isServerRunning) {
        m_isServerRunning = false;
        if (m_hStopEvent) {
            SetEvent(m_hStopEvent);
        }

        // Connect dummy client to unblock ConnectNamedPipe
        HANDLE hDummy = CreateFileW(PIPE_NAME, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hDummy != INVALID_HANDLE_VALUE) {
            CloseHandle(hDummy);
        }

        if (m_hServerThread) {
            WaitForSingleObject(m_hServerThread, 1000);
            CloseHandle(m_hServerThread);
            m_hServerThread = nullptr;
        }

        if (m_hStopEvent) {
            CloseHandle(m_hStopEvent);
            m_hStopEvent = nullptr;
        }
    }

    if (m_hMutex) {
        ReleaseMutex(m_hMutex);
        CloseHandle(m_hMutex);
        m_hMutex = nullptr;
    }
}

DWORD WINAPI SingleInstance::serverThreadProc(LPVOID param) {
    auto *self = reinterpret_cast<SingleInstance*>(param);

    while (self->m_isServerRunning) {
        HANDLE hPipe = CreateNamedPipeW(
            PIPE_NAME,
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512,
            512,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(200);
            continue;
        }

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected && self->m_isServerRunning) {
            char buffer[128] = { 0 };
            DWORD bytesRead = 0;
            if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                std::string cmd(buffer, bytesRead);
                // Dispatch to Qt main thread
                QMetaObject::invokeMethod(self, [self, cmd]() {
                    if (cmd.find("SHOW") != std::string::npos) {
                        emit self->showRequested();
                    } else if (cmd.find("SETUP") != std::string::npos) {
                        emit self->setupRequested();
                    } else {
                        emit self->wakeRequested("Aura Island is running");
                    }
                }, Qt::QueuedConnection);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    return 0;
}

} // namespace Aura
