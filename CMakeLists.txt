cmake_minimum_required(VERSION 3.20)
project(GalHub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug mode detection ==========================================

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
    set(DEBUG TRUE)
    message(STATUS "Debug mode")
else ()
    set(DEBUG FALSE)
    message(STATUS "Release mode")
endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 TRUE)
else ()
    set(X64 FALSE)
endif ()

if (X64 AND GENERATE_QT)
    message(STATUS "64-bit build with Qt. candidate targets: GalHub")
elseif (X64 AND NOT GENERATE_QT)
    message(STATUS "64-bit build without Qt. candidate targets: Hook(x64)")
elseif (NOT X64 AND NOT GENERATE_QT)
    message(STATUS "32-bit build. candidate targets: Hook(x86), Helper")
else ()
    message(FATAL_ERROR "Invalid build configuration")
endif ()

# configure Hook dll ===========================================
if (NOT GENERATE_QT)
    add_subdirectory(3rdparty/minhook EXCLUDE_FROM_ALL)

    add_library(Hook SHARED src/hook/main.cpp

            src/hook/path_util.h
            src/hook/path_util.cpp

            src/hook/nt.hpp
            src/hook/nt_set_information_file.hpp
            src/hook/nt_create_file.hpp
            src/hook/nt_open_file.hpp
            src/hook/nt_query_attributes_file.hpp

            src/common/json_extend.hpp
            src/common/env.cpp
            src/common/env.h
            src/common/util.h
            src/common/util.cpp)

    # postfix for 32-bit and 64-bit
    if (X64)
        set_target_properties(Hook PROPERTIES DEBUG_POSTFIX "-x64")
        set_target_properties(Hook PROPERTIES RELEASE_POSTFIX "-x64")
        set_target_properties(Hook PROPERTIES RELWITHDEBINFO_POSTFIX "-x64")
        set_target_properties(Hook PROPERTIES MINSIZEREL_POSTFIX "-x64")
    else ()
        set_target_properties(Hook PROPERTIES DEBUG_POSTFIX "-x86")
        set_target_properties(Hook PROPERTIES RELEASE_POSTFIX "-x86")
        set_target_properties(Hook PROPERTIES RELWITHDEBINFO_POSTFIX "-x86")
        set_target_properties(Hook PROPERTIES MINSIZEREL_POSTFIX "-x86")
    endif ()

    # MinHook library
    target_include_directories(Hook PUBLIC "3rdparty/minhook/include")
    target_link_libraries(Hook PRIVATE minhook)

    # Jsonxx library
    target_include_directories(Hook PUBLIC "3rdparty/configor/include/configor")
endif ()

# configure Helper exe ========================================
if (NOT X64 AND NOT GENERATE_QT)
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
if (X64 AND GENERATE_QT)
    set(CMAKE_PREFIX_PATH $ENV{QT6_PATH})
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
    qt_standard_project_setup()

    set(SOURCES src/hub/main.cpp

            src/common/json_extend.hpp
            src/common/env.cpp
            src/common/env.h
            src/common/util.h
            src/common/util.cpp

            src/hub/env_manager.cpp
            src/hub/env_manager.h
            src/hub/runner.cpp
            src/hub/runner.h

            src/hub/gui/main_window.h
            src/hub/gui/main_window.cpp
            src/hub/gui/game_table.h
            src/hub/gui/game_table.cpp
            src/hub/gui/path_edit_group.h
            src/hub/gui/path_edit_group.cpp
            src/hub/gui/auto_deselect_event_filter.h
            src/hub/gui/auto_deselect_event_filter.cpp
            src/hub/gui/rule_diag.h
            src/hub/gui/rule_diag.cpp
            src/hub/gui/game_config_rules_widget.h
            src/hub/gui/game_config_rules_widget.cpp
            src/hub/gui/game_config_dialog.h
            src/hub/gui/game_config_dialog.cpp)

    qt_add_executable(GalHub ${SOURCES})

    target_link_libraries(GalHub PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

    # if the build type is Debug or environment variable QT6_ENABLE_CONSOLE is set to 1, enable console
    if (DEBUG OR QT6_ENABLE_CONSOLE)
        set_property(TARGET GalHub PROPERTY WIN32_EXECUTABLE FALSE)
    else ()
        set_property(TARGET GalHub PROPERTY WIN32_EXECUTABLE TRUE)
    endif ()

    # Jsonxx library
    target_include_directories(GalHub PUBLIC "3rdparty/configor/include/configor")
endif ()


# install ======================================================
if (TARGET Hook)
    install(TARGETS Hook DESTINATION "${CMAKE_SOURCE_DIR}/dist/Bin")
endif ()

if (TARGET Helper)
    install(TARGETS Helper DESTINATION "${CMAKE_SOURCE_DIR}/dist/Bin")
endif ()

if (TARGET GalHub)
    install(TARGETS GalHub DESTINATION "${CMAKE_SOURCE_DIR}/dist")
endif ()
