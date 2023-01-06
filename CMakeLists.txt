# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.5)
add_compile_options(-Wall -Wextra -pedantic)

# Name of the project
project(BEM)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 17)

# Add the "common" directory as a subdirectory
add_subdirectory(common)

# Add the "coordinator" directory as a subdirectory
add_subdirectory(coordinator)

# Add the "node" directory as a subdirectory
add_subdirectory(node)