cmake_minimum_required(VERSION 3.22)
project(AuraIsland LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    Widgets
    Network
    NetworkAuth
    Sql
)

if(MSVC)
    add_compile_options(
        /permissive-
        /utf-8
        /EHsc
        /W4
        /wd4100 # unused formal parameter
    )
endif()

set(SOURCES
    src/main.cpp
    src/core/Logger.h
    src/core/Logger.cpp
    src/core/DatabaseManager.h
    src/core/DatabaseManager.cpp
    src/core/Settings.h
    src/core/Settings.cpp
    src/core/WindowTweaks.h
    src/core/WindowTweaks.cpp
    src/core/NativeEventFilter.h
    src/core/NativeEventFilter.cpp
    src/core/FullscreenDetector.h
    src/core/FullscreenDetector.cpp
    src/core/Watchdog.h
    src/core/Watchdog.cpp
    src/core/AppState.h
    src/core/AppState.cpp
    src/core/AlertManager.h
    src/core/AlertManager.cpp
    src/core/IslandWindow.h
    src/core/IslandWindow.cpp
    src/features/shelf/ShelfModel.h
    src/features/shelf/ShelfModel.cpp
    src/features/shelf/ShelfManager.h
    src/features/shelf/ShelfManager.cpp
    src/features/clipboard/ClipboardModel.h
    src/features/clipboard/ClipboardModel.cpp
    src/features/clipboard/ClipboardManager.h
    src/features/clipboard/ClipboardManager.cpp
    src/features/notes/NotesModel.h
    src/features/notes/NotesModel.cpp
    src/features/notes/NotesManager.h
    src/features/notes/NotesManager.cpp
    src/features/gmail/CredentialStore.h
    src/features/gmail/CredentialStore.cpp
    src/features/gmail/GmailModel.h
    src/features/gmail/GmailModel.cpp
    src/features/gmail/GmailService.h
    src/features/gmail/GmailService.cpp
    src/features/dev/GitWatcher.h
    src/features/dev/GitWatcher.cpp
    src/features/dev/PomodoroTimer.h
    src/features/dev/PomodoroTimer.cpp
    src/features/dev/AgentHookServer.h
    src/features/dev/AgentHookServer.cpp
    src/features/dev/DevManager.h
    src/features/dev/DevManager.cpp
    src/features/media/WinRtMediaBridge.h
    src/features/media/WinRtMediaBridge.cpp
    src/features/media/MediaManager.h
    src/features/media/MediaManager.cpp
    src/features/drop/QrCode.hpp
    src/features/drop/QrCode.cpp
    src/features/drop/LocalShareServer.h
    src/features/drop/LocalShareServer.cpp
    src/features/drop/DropManager.h
    src/features/drop/DropManager.cpp
    src/features/system/SystemControlManager.h
    src/features/system/SystemControlManager.cpp
    resources.qrc
    resources/version.rc
)

add_executable(AuraIsland WIN32 ${SOURCES})

target_include_directories(AuraIsland PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/features
)

target_link_libraries(AuraIsland PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Widgets
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Sql
    dwmapi
    user32
    shell32
    advapi32
    credui
    crypt32
    windowsapp
    powrprof
    dxva2
)

# Set target properties
set_target_properties(AuraIsland PROPERTIES
    OUTPUT_NAME "AuraIsland"
    WIN32_EXECUTABLE TRUE
)
