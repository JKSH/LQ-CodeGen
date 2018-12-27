QT     += core_private # For QMetaObjectBuilder only. Do not use other private API.
QT     += widgets svg winextras

TARGET = LQWidgets
TEMPLATE = lib

SOURCES += \
    lqmain.cpp \
    lqlibinterface.cpp \
    lqtypes.cpp \
    lqerrors.cpp \
    lqapplication.cpp

HEADERS += \
    lqmain.h \
    lqlibinterface.h \
    lqtypes.h \
    lqerrors.h \
    lqapplication.h

INCLUDEPATH += "C:/Program Files (x86)/National Instruments/LabVIEW 2014/cintools"
LIBS        += "C:/Program Files (x86)/National Instruments/LabVIEW 2014/cintools/labviewv.lib"

include(C:/Qwt-6.1.2/features/qwt.prf)
