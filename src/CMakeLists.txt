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
  codeeditor.cpp
  codevizwidget.cpp
  console.cpp
  dataobject.cpp
  hwtopo.cpp
  main.cpp
  mainwindow.cpp
  hwtopovizwidget.cpp
  pcvizwidget.cpp
  parseUtil.cpp
  util.cpp
  varvizwidget.cpp
  vizwidget.cpp)

set(HEADERS
  codeeditor.h
  codevizwidget.h
  console.h
  dataobject.h
  hwtopo.h
  mainwindow.h
  hwtopovizwidget.h
  pcvizwidget.h
  parseUtil.h
  util.h
  varvizwidget.h
  vizwidget.h)

set(UIC
  ui_form.h)

# Build Target
add_executable(MemAxes MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${UIC})

qt5_use_modules(MemAxes Widgets OpenGL)

target_link_libraries(MemAxes Qt5::Widgets Qt5::OpenGL ${OPENGL_LIBRARIES})# ${VTK_LIBRARIES})

install(TARGETS MemAxes DESTINATION bin)
