cmake_minimum_required(VERSION 3.0)

# add_subdirectory(display_gui)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

link_directories(${DEPENDS_ROOT}/lib ${DEPENDS_ROOT}/bin)
include_directories(${DEPENDS_ROOT}/include)



file(GLOB_RECURSE src_display_gui ${CMAKE_CURRENT_SOURCE_DIR}/display_gui/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/display_gui/*.h*)

add_executable(display_gui ${src_display_gui})
if(DO_MEMADDR_SANITIZE)
  add_definitions("-fsanitize=address")
  target_link_libraries(display_gui -lasan)
endif()

target_compile_features(display_gui PUBLIC cxx_constexpr)
target_link_libraries(display_gui Qt5::Widgets Qt5::Gui ) #Qt5::Charts


