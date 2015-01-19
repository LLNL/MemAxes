project(MemAxes C CXX)

# Qt5 requires CMake 2.8.9 or higher.
cmake_minimum_required(VERSION 2.8.9)

# Top-level build just includes subdirectories.
add_subdirectory(src)
add_subdirectory(example_data)
