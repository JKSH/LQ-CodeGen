#ifndef LQWIDGETS_H
#define LQWIDGETS_H

#include <QtCore>
#include <QtGlobal>
#include "extcode.h"
#include "utils.h"

extern "C" {

extern qint32 Q_DECL_EXPORT startWidgetEngine(quintptr* _retVal, LStrHandle pluginDir);
extern qint32 Q_DECL_EXPORT stopWidgetEngine();

extern qint32 Q_DECL_EXPORT registerEventRefs(LVUserEventRef* voidRef, LVUserEventRef* boolRef, LVUserEventRef* i32Ref, LVUserEventRef* dblRef, LVUserEventRef* stringRef);
extern qint32 Q_DECL_EXPORT connect_void(quintptr _instance, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_bool(quintptr _instance, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_i32(quintptr _instance, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_dbl(quintptr _instance, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT connect_string(quintptr _instance, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT emit_void(quintptr _instance, const char* normalizedSignal);
extern qint32 Q_DECL_EXPORT emit_bool(quintptr _instance, const char* normalizedSignal, bool* data);
extern qint32 Q_DECL_EXPORT emit_i32(quintptr _instance, const char* normalizedSignal, qint32* data);
extern qint32 Q_DECL_EXPORT emit_dbl(quintptr _instance, const char* normalizedSignal, double* data);
extern qint32 Q_DECL_EXPORT emit_string(quintptr _instance, const char* normalizedSignal, LStrHandle data);
extern qint32 Q_DECL_EXPORT registerLQObject(quintptr _instance, LVArray<LStrHandle>** signalList, LStrHandle superClassName);
extern qint32 Q_DECL_EXPORT findSignalIndex(qint64* _retVal, quintptr _instance, const char* encodedSignal);

// TODO: Add QObject::setProperty() and QObject::property()

//[TEMPLATE]

}

#endif
