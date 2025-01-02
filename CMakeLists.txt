set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

cmake_minimum_required(VERSION 3.10)
project(epsilang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# List all source files
set(SOURCES
        src/*.cpp
        src/core/*.cpp
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)


# Get all source files
file(GLOB_RECURSE SOURCES
        "src/*.cpp"
        "src/core/*.cpp"
)



# Add executable
add_executable(epsilang ${SOURCES})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -DCMAKE_EXPORT_COMPILE_COMMANDS=1")