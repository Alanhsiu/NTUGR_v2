# src/flute/CMakeLists.txt
add_compile_options(-w)

cmake_minimum_required(VERSION 3.10)
project(flute)

# Add source files
file(GLOB FLUTE_SOURCES "*.c")

add_library(flute ${FLUTE_SOURCES})
message("Compiling flute sources: ${FLUTE_SOURCES}")

target_include_directories(flute PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})