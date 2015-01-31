project(MemAxes C CXX)

# Qt5 requires CMake 2.8.9 or higher.
cmake_minimum_required(VERSION 2.8.9)

# Qt5 + Modules
find_package(Qt5 REQUIRED Core Widgets OpenGL Xml)

# OpenGL
find_package(OpenGL)

# VTK
find_package(VTK REQUIRED)
set(VTK_USE_QVTK ON)

# Top-level build just includes subdirectories.
add_subdirectory(src)
add_subdirectory(example_data)
