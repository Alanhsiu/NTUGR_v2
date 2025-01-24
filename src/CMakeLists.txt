# CMakeLists.txt for src

# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project's name
project(NTUGR)

# Set the default C++ standard
set(CMAKE_CXX_STANDARD 17) # set C++11 for EDA39 compatibility
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE) # Default to Release
endif()

# Print project information
message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Add main source file
set(MAIN_SOURCES main.cpp)

# Add subdirectories for modular organization
add_subdirectory(basic)
add_subdirectory(gr)
add_subdirectory(flute)

# Create the executable
add_executable(route ${MAIN_SOURCES})

# Specify include directories
target_include_directories(route PRIVATE basic gr flute)

# Link libraries to the main executable
target_link_libraries(route PRIVATE basic gr flute)

# Add OpenMP support if available
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(route PUBLIC OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP Found: Enabled parallelization")
else()
    message(WARNING "OpenMP Not Found: Compilation without parallelization")
endif()

# Build options for Debug and Release configurations
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(route PRIVATE -g3 -O0)
    message(STATUS "Debug mode enabled: Adding debug symbols")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(route PRIVATE -O3 -march=native -flto -DNDEBUG)
    message(STATUS "Release mode enabled: Adding optimization flags")
endif()

# Enable testing
enable_testing()

# Add memory check with Valgrind (optional)
find_program(VALGRIND_EXECUTABLE valgrind)
if(VALGRIND_EXECUTABLE)
    add_test(NAME ValgrindMemoryCheck
             COMMAND ${VALGRIND_EXECUTABLE} --tool=memcheck --leak-check=full --show-reachable=yes --error-exitcode=1 $<TARGET_FILE:route>)
    set_tests_properties(ValgrindMemoryCheck
                         PROPERTIES PASS_REGULAR_EXPRESSION "no leaks are possible")
    message(STATUS "Valgrind found: Memory checking enabled")
else()
    message(WARNING "Valgrind not found: Memory checking skipped")
endif()
