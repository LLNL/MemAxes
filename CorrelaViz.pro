#-------------------------------------------------
#
# Project created by QtCreator 2013-09-11T17:55:55
#
#-------------------------------------------------

QT       += opengl core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CorrelaViz
TEMPLATE = app

SOURCES  += main.cpp\
            mainwindow.cpp \
            correlationmatrixviz.cpp \
            dataobject.cpp \
            util.cpp \
            vizwidget.cpp \
            parallelcoordinatesviz.cpp \
    volumevizwidget.cpp \
    selectionviz.cpp

HEADERS  += mainwindow.h \
            correlationmatrixviz.h \
            dataobject.h \
            util.h \
            vizwidget.h \
            parallelcoordinatesviz.h \
            qtlogo.h \
    volumevizwidget.h \
    selectionviz.h

FORMS    += mainwindow.ui

# VTK
INCLUDEPATH += /Users/chai/vtk-5.10/include

LIBS    += -L/Users/chai/vtk-5.10/lib \
           -lvtkCommon \
           -lvtksys \
           -lQVTK \
           -lvtkViews \
           -lvtkWidgets \
           -lvtkInfovis \
           -lvtkRendering \
           -lvtkGraphics \
           -lvtkImaging \
           -lvtkIO \
           -lvtkFiltering \
           -lvtkVolumeRendering \
           -lvtklibxml2 \
           -lvtkDICOMParser \
           -lvtkpng \
           -lvtkpng \
           -lvtktiff \
           -lvtkzlib \
           -lvtkjpeg \
           -lvtkalglib \
           -lvtkexpat \
           -lvtkverdict \
           -lvtkmetaio \
           -lvtkNetCDF \
           -lvtksqlite \
           -lvtkexoIIc \
           -lvtkftgl \
           -lvtkfreetype \
           -lvtkHybrid

# mac specific for VTK
macx:LIBS += -framework CoreFoundation \
             -framework IOKit \
             -framework Cocoa \
             -framework OpenGL
