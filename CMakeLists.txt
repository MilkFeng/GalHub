cmake_minimum_required(VERSION 3.20)
project(GalHub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug mode detection ==========================================
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
    message(STATUS "Debug mode")
else ()
    message(STATUS "Release mode")
endif ()

# architecture detection ========================================
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
    message(STATUS "64-bit architecture detected, will configure for Hook-x64.dll and GalHub.exe")
else ()
    set(X64 FALSE)
    message(STATUS "32-bit architecture detected, will configure for Hook-x86.dll and Helper.exe")
endif ()

add_subdirectory(3rdparty/minhook)

# configure MinHook dll ========================================
add_library(Hook SHARED src/hook/main.cpp
        src/common/json_extend.hpp
        src/common/env.cpp
        src/common/env.h)

# MinHook library
target_include_directories(Hook PUBLIC "3rdparty/minhook/include")
target_link_libraries(Hook PRIVATE minhook)

# Jsonxx library
target_include_directories(Hook PUBLIC "3rdparty/configor/include/configor")

# configure Helper exe ========================================
if (NOT X64)
    add_executable(Helper src/helper/main.cpp
            src/common/json_extend.hpp
            src/common/env.cpp
            src/common/env.h
            src/common/util.h
            src/common/util.cpp)

    # Jsonxx library
    target_include_directories(Helper PUBLIC "3rdparty/configor/include/configor")
endif ()

# configure GalHub exe ========================================
if (X64)
    # because Qt6 may not support Debug mode, we need to save the original build type and set it to Release
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        set(_CMAKE_BUILD_TYPE Debug)
    else ()
        set(_CMAKE_BUILD_TYPE Release)
    endif ()

    set(CMAKE_BUILD_TYPE Release)


    set(CMAKE_PREFIX_PATH $ENV{QT6_PATH})
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
    qt_standard_project_setup()

    qt_add_executable(GalHub src/hub/main.cpp

            src/hub/gui/main_window.h
            src/hub/gui/main_window.cpp
            src/hub/gui/game_table.h
            src/hub/gui/game_table.cpp
            src/hub/gui/game_config_rules_widget.h
            src/hub/gui/game_config_rules_widget.cpp
            src/hub/gui/game_config_dialog.h
            src/hub/gui/game_config_dialog.cpp

            src/common/json_extend.hpp
            src/common/env.cpp
            src/common/env.h
            src/common/util.h
            src/common/util.cpp
            src/hub/env_manager.cpp
            src/hub/env_manager.h
            src/hub/runner.cpp
            src/hub/runner.h)

    target_link_libraries(GalHub PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

    # if the original build type is Debug, show console window
    if (_CMAKE_BUILD_TYPE STREQUAL Release)
        set_property(TARGET GalHub PROPERTY WIN32_EXECUTABLE TRUE)
    endif ()

    # Jsonxx library
    target_include_directories(GalHub PUBLIC "3rdparty/configor/include/configor")
endif ()