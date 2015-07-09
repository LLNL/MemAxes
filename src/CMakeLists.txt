set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON) # tested, not working

# Includes, Definitions, Flags
include_directories(${Qt5Widgets_INCLUDE_DIRS})# ${VTK_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

# ui files
qt5_wrap_ui(ui_form.h form.ui)

# Sources and UI Files
set(SOURCES
  axisvizwidget.cpp
  clustermetrics.cpp
  clustertreevizwidget.cpp
  codeeditor.cpp
  codevizwidget.cpp
  console.cpp
  dataobject.cpp
  datacluster.cpp
  hwtopo.cpp
  hwtopodrawable.cpp
  hwtopovizwidget.cpp
  main.cpp
  mainwindow.cpp
  pcvizwidget.cpp
  parseUtil.cpp
  util.cpp
  varvizwidget.cpp
  vizwidget.cpp)

set(HEADERS
  axisvizwidget.h
  clustermetrics.h
  clustertreevizwidget.h
  codeeditor.h
  codevizwidget.h
  console.h
  dataobject.h
  datacluster.h
  hwtopo.h
  hwtopodrawable.h
  hwtopovizwidget.h
  mainwindow.h
  pcvizwidget.h
  parseUtil.h
  typedefs.h
  util.h
  varvizwidget.h
  vizwidget.h)
  #volumevizwidget.h)

set(UIC
  ui_form.h)

# Build Target
add_executable(MemAxes MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${UIC})
# add_executable(MemAxes ${SOURCES} ${HEADERS} ${UIC})

# C++11 support
target_compile_features(MemAxes PRIVATE cxx_range_for)

qt5_use_modules(MemAxes Widgets OpenGL)

target_link_libraries(MemAxes Qt5::Widgets Qt5::OpenGL ${OPENGL_LIBRARIES})# ${VTK_LIBRARIES})

install(TARGETS MemAxes DESTINATION bin)
