cmake_minimum_required(VERSION 3.20)
project(GalHub)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2022_64")
#
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

#find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_library(Hook SHARED src/hook/main.cpp
        src/common/env.cpp
        src/common/env.h)

add_executable(GalHub src/hub/main.cpp
        src/common/env.cpp
        src/common/env.h
        src/common/inject.h
        src/common/inject.cpp
        src/hub/env_manager.cpp
        src/hub/env_manager.h
        src/hub/runner.cpp
        src/hub/runner.h)

add_executable(Helper src/helper/main.cpp
        src/common/inject.h
        src/common/inject.cpp)

# Debug mode
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
endif ()

# MinHook library
target_include_directories(Hook PUBLIC "3rdparty/minhook/include")

# select MinHook library based on target architecture
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(Hook "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/minhook/lib/libMinHook-x64-v141-mt.lib")
else ()
    target_link_libraries(Hook "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/minhook/lib/libMinHook-x86-v141-mt.lib")
endif ()

# Jsonxx library
target_include_directories(Hook PUBLIC "3rdparty/configor")
target_include_directories(GalHub PUBLIC "3rdparty/configor")

## Qt library
#target_link_libraries(GalHub Qt6::Core)
#
## Copy Qt dlls to GalHub output directory
#if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#    set(DEBUG_SUFFIX)
#    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
#        set(DEBUG_SUFFIX "d")
#    endif ()
#    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
#    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
#        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
#        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
#            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
#        endif ()
#    endif ()
#    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E make_directory
#                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E copy
#                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
#                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
#    endif ()
#    foreach (QT_LIB Core Gui Widgets)
#        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#                COMMAND ${CMAKE_COMMAND} -E copy
#                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
#                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
#    endforeach (QT_LIB)
#endif ()
