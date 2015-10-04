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
extern qint32 Q_DECL_EXPORT connect_bySignature_qtMethod(QMetaObject::Connection* _retVal, quintptr sender, const char* encodedSignal, quintptr receiver, const char* encodedMethod);
extern qint32 Q_DECL_EXPORT activate_void(quintptr _instance, qint32 signalIndex);
extern qint32 Q_DECL_EXPORT activate_bool(quintptr _instance, qint32 signalIndex, bool* data);
extern qint32 Q_DECL_EXPORT activate_i32(quintptr _instance, qint32 signalIndex, qint32* data);
extern qint32 Q_DECL_EXPORT activate_dbl(quintptr _instance, qint32 signalIndex, double* data);
extern qint32 Q_DECL_EXPORT activate_string(quintptr _instance, qint32 signalIndex, LStrHandle data);
extern qint32 Q_DECL_EXPORT emit_void(quintptr _instance, const char* normalizedSignal);
extern qint32 Q_DECL_EXPORT emit_bool(quintptr _instance, const char* normalizedSignal, bool* data);
extern qint32 Q_DECL_EXPORT emit_i32(quintptr _instance, const char* normalizedSignal, qint32* data);
extern qint32 Q_DECL_EXPORT emit_dbl(quintptr _instance, const char* normalizedSignal, double* data);
extern qint32 Q_DECL_EXPORT emit_string(quintptr _instance, const char* normalizedSignal, LStrHandle data);
extern qint32 Q_DECL_EXPORT registerLQObject(quintptr _instance, LVArray<LStrHandle>** signalList, LStrHandle superClassName);
extern qint32 Q_DECL_EXPORT findSignalIndex(qint64* _retVal, quintptr _instance, const char* encodedSignal);
extern qint32 Q_DECL_EXPORT findSignalInfo(qint32* _retVal_signalIndex, bool* _retVal_isDynamic, quintptr _instance, const char* normalizedSignal);

// TODO: Add QObject::setProperty() and QObject::property()

//[TEMPLATE]

}

#endif
