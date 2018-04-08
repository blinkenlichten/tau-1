cmake_minimum_required(VERSION 3.0)


link_directories(${DEPENDS_ROOT}/lib ${DEPENDS_ROOT}/bin)
include_directories(${DEPENDS_ROOT}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(../include)
include_directories(./display_gui)

set(AUTOMOC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${QT5_SEARCH_PATH}/include)

add_definitions("-std=c++1z")

file(GLOB_RECURSE src_display_gui ${CMAKE_CURRENT_SOURCE_DIR}/display_gui/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/display_gui/*.h
${CMAKE_CURRENT_SOURCE_DIR}/display_gui/*.hpp)
QT5_WRAP_CPP(moc_out ${src_display_gui})
message(WARNING "Qt/MOC files = ${moc_out}")
add_executable(display_gui ${moc_out} ${src_display_gui})
if(DO_MEMADDR_SANITIZE)
  add_definitions("-fsanitize=address")
  target_link_libraries(display_gui -lasan)
endif()

target_compile_features(display_gui PUBLIC cxx_constexpr)
target_link_libraries(display_gui Qt5::Widgets Qt5::Gui Qt5::Charts)


