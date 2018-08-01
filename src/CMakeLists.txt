cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

link_directories(${DEPENDS_ROOT}/lib ${DEPENDS_ROOT}/bin)
include_directories(${DEPENDS_ROOT}/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions("-std=c++1z")

file(GLOB_RECURSE t1src t1net/*.cpp ../include/t1net/*.h*
    t1_anything_goes/*.cpp ./include/t1_anything_goes/*.h*)

add_library(t1net STATIC ${t1src})
target_compile_features(t1net PUBLIC cxx_constexpr)

if(DO_MEMADDR_SANITIZE)
        add_definitions("-fsanitize=address")
        target_link_libraries(t1net -lasan)
endif()

target_link_libraries(t1net -pthread -lssl -lcrypto -lzmq -lczmq -lsodium -lmlm)

