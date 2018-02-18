cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(Tau1)

#sensible project options:
##-------------------------------------------------------------------------
option(BUILD_UI "Compile UI that depends on Qt5." ON)
option(MEMADDR_SANITIZER "Turn on memory sanitizer" ON)

## Where to look for Qt5:
set(QT5_SEARCH_PATH $ENV{HOME}/Qt/5.10.0 CACHE PATH "Qt5 search path")
set(QT_QMAKE_EXECUTABLE ${QT5_SEARCH_PATH}/gcc_64/bin/qmake CACHE PATH "Qt5 qmake path for GCC")
##-------------------------------------------------------------------------

set(DEPENDS_ROOT ${CMAKE_SOURCE_DIR}/build_deps CACHE PATH "Directory where we put dependencies libraries.")


find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions("-std=c++1z")


add_subdirectory(src)
add_subdirectory(src_gui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

