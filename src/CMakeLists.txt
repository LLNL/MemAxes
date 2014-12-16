cmake_minimum_required(VERSION 2.8.3)
project(MemAxes)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#########################################
# Set QT and VTK Build Directories Here #
#########################################
set(QT_DIR "/usr/local/opt/qt5/")
set(VTK_DIR "/Users/chai/src/VTK/Build")
#########################################

# Necessary Qt cmake variables
set(CMAKE_PREFIX_PATH ${QT_DIR})
set(QT_UIC_EXECUTABLE "${QT_DIR}/bin/uic")

# Qt5 + Modules
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON) # tested, not working

# OpenGL 
find_package(OpenGL)

# VTK
find_package(VTK REQUIRED)
set(VTK_USE_QVTK ON)

# Includes, Definitions, Flags
include_directories(${Qt5Widgets_INCLUDE_DIRS} ${VTK_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

# ui files
qt5_wrap_ui(ui_form.h form.ui)

# Sources and UI Files
set(MemAxes_SOURCES 
        codeeditor.cpp 
        codeviz.cpp 
        console.cpp 
        dataobject.cpp 
        hardwaretopology.cpp 
        main.cpp 
        mainwindow.cpp 
        memtopoviz.cpp 
        parallelcoordinatesviz.cpp 
        parseUtil.cpp 
        util.cpp 
        varviz.cpp 
        vizwidget.cpp 
        volumevizwidget.cpp)

set(MemAxes_UIC 
        ui_form.h)

# Build Target
add_executable(MemAxes MACOSX_BUNDLE ${MemAxes_SOURCES} ${MemAxes_UIC})
# add_executable(MemAxes ${MemAxes_SOURCES} ${MemAxes_UIC})
qt5_use_modules(MemAxes Widgets OpenGL)

target_link_libraries(MemAxes Qt5::Widgets Qt5::OpenGL ${OPENGL_LIBRARIES} ${VTK_LIBRARIES})


