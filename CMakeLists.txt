project(MemAxes C CXX)

# target_compile_features (for C++11) requires 3.1.0
cmake_minimum_required(VERSION 3.1.0)

# Qt5 + Modules
find_package(Qt5 REQUIRED Core Widgets OpenGL Xml)

# OpenGL
find_package(OpenGL)

# Top-level build just includes subdirectories.
add_subdirectory(src)
add_subdirectory(example_data)
