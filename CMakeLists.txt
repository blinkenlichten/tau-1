cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(Tau1 CXX)

#sensible project options:
##-------------------------------------------------------------------------
option(MEMADDR_SANITIZER "Turn on memory sanitizer" ON)

set(DEPENDS_ROOT ${CMAKE_SOURCE_DIR}/external CACHE PATH "Directory where we put dependencies libraries.")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions("-std=c++1z")


add_subdirectory(src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/external/include)
INCLUDE_DIRECTORIES(${DEPENDS_ROOT}/include)

