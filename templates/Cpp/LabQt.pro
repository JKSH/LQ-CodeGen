CONFIG += C++11
QT     += widgets

TARGET = LabQt
TEMPLATE = lib

SOURCES += \
    labqt.cpp \
    utils.cpp

HEADERS += \
    labqt.h \
    bridge.h \
    utils.h

INCLUDEPATH += "C:/Program Files (x86)/National Instruments/LabVIEW 2013/cintools"
LIBS        += "C:/Program Files (x86)/National Instruments/LabVIEW 2013/cintools/labviewv.lib"

