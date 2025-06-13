

cmake_minimum_required(VERSION 3.10)
project(bcc C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.c")

# Define executable
add_executable(bcc ${SOURCES})
