project(MemAxes C CXX)

# Qt5 requires CMake 2.8.9 or higher.
cmake_minimum_required(VERSION 2.8.9)

# Qt5 + Modules
find_package(Qt5 REQUIRED Core Widgets OpenGL Xml)

# OpenGL
find_package(OpenGL)

# Top-level build just includes subdirectories.
add_subdirectory(src)
add_subdirectory(example_data)
