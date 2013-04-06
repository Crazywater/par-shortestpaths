QT       -= gui

TARGET = par-shortestpaths
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11 -fopenmp -O3 -g -msse
QMAKE_LFLAGS += -fopenmp -O3 -g

SOURCES += main.cpp \
    graph.cpp \
    dsthread.cpp \
    shortestpath.cpp \
    pardeltastepping.cpp

HEADERS += \
    graph.h \
    dsthread.h \
    shortestpath.h \
    reqentry.h \
    barrier.h \
    pardeltastepping.h
