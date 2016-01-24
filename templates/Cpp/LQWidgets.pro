CONFIG += C++11
QT     += core_private # For QMetaObjectBuilder only. Do not use other private API.
QT     += widgets svg winextras

TARGET = LQWidgets
TEMPLATE = lib

SOURCES += \
    lqwidgets.cpp \
    utils.cpp \
    errors.cpp \
    lqapplication.cpp

HEADERS += \
    lqwidgets.h \
    bridge.h \
    utils.h \
    errors.h \
    lqapplication.h

INCLUDEPATH += "C:/Program Files (x86)/National Instruments/LabVIEW 2013/cintools"
LIBS        += "C:/Program Files (x86)/National Instruments/LabVIEW 2013/cintools/labviewv.lib"

include(C:/Qwt-6.1.2/features/qwt.prf)
