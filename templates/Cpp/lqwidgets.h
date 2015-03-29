#ifndef LQWIDGETS_H
#define LQWIDGETS_H

#include <QtCore>
#include <QtGlobal>
#include "extcode.h"
#include "utils.h"

extern "C" {

extern qint32 Q_DECL_EXPORT startWidgetEngine(LStrHandle pluginDir);
extern qint32 Q_DECL_EXPORT stopWidgetEngine();

extern qint32 Q_DECL_EXPORT registerEventRefs(LVUserEventRef* voidRef, LVUserEventRef* boolRef, LVUserEventRef* i32Ref, LVUserEventRef* stringRef);
extern qint32 Q_DECL_EXPORT connect_void(quint32 qobject, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_bool(quint32 qobject, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_i32(quint32 qobject, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_string(quint32 qobject, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT findSignalIndex(qint64* retVal, quint32 qobject, const char* encodedSignal);

// TODO: Add QObject::setProperty() and QObject::property()

//[TEMPLATE]

}

#endif
