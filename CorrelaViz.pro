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
    scatterplotviz.cpp \
    vizwidget.cpp \
    parallelcoordinatesviz.cpp

HEADERS  += mainwindow.h \
            correlationmatrixviz.h \
    dataobject.h \
    util.h \
    scatterplotviz.h \
    vizwidget.h \
    parallelcoordinatesviz.h

FORMS    += mainwindow.ui
