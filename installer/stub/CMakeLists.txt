cmake_minimum_required(VERSION 3.22)
project(AuraSetupStub LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED APP_VERSION)
    set(APP_VERSION "1.0.0")
endif()

if(NOT DEFINED AURA_ENGINE_PATH)
    set(AURA_ENGINE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../dist-engine/AuraEngine.exe")
endif()

# Static MSVC runtime (/MT) for zero external dependencies
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(STUB_SOURCES
    main.cpp
    stub.rc
    manifest.xml
)

add_executable(AuraSetupStub WIN32 ${STUB_SOURCES})

# Pass AURA_ENGINE_FILE definition to resource compiler if file exists
if(EXISTS "${AURA_ENGINE_PATH}")
    get_filename_component(AURA_ENGINE_ABS "${AURA_ENGINE_PATH}" ABSOLUTE)
    # CMake rc definition
    target_compile_definitions(AuraSetupStub PRIVATE AURA_ENGINE_FILE="${AURA_ENGINE_ABS}")
endif()

target_compile_definitions(AuraSetupStub PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

target_link_libraries(AuraSetupStub PRIVATE
    gdiplus
    user32
    gdi32
    shell32
    advapi32
    ole32
)

# Output named AuraIsland-Setup-<version>.exe
set_target_properties(AuraSetupStub PROPERTIES
    OUTPUT_NAME "AuraIsland-Setup-${APP_VERSION}"
    WIN32_EXECUTABLE TRUE
)
