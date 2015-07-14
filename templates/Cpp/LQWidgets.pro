CONFIG += C++11
QT     += widgets svg

TARGET = LQWidgets
TEMPLATE = lib

SOURCES += \
    lqwidgets.cpp \
    utils.cpp \
    errors.cpp

HEADERS += \
    lqwidgets.h \
    bridge.h \
    utils.h \
    errors.h

INCLUDEPATH += "C:/Program Files (x86)/National Instruments/LabVIEW 2013/cintools"
LIBS        += "C:/Program Files (x86)/National Instruments/LabVIEW 2013/cintools/labviewv.lib"

include(C:/Qwt-6.1.2/features/qwt.prf)
