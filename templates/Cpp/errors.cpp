#include "errors.h"
#include "utils.h"
#include <QMap>

static const QMap<int, QByteArray> errorMap
{
	{LQ::NoError, "No error."},
	{LQ::EngineNotRunningError, "The widget engine is not running."},
	{LQ::EngineAlreadyRunningError, "The widget engine is already running."}
};

void
errorStringFromCode(LStrHandle _retVal, int32 errorCode)
{
	copyIntoLStr(_retVal, errorMap.value(errorCode, "Unknown error."));
}
