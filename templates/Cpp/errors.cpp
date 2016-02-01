#include "errors.h"
#include "utils.h"
#include <QMap>

static const QMap<int, QByteArray> errorMap
{
	{LQ::NoError, "No error."},
	{LQ::EngineNotRunningError, "The widget engine is not running."},
	{LQ::EngineAlreadyRunningError, "The widget engine is already running."},
	{LQ::NotAnLQObjectError, "This QObject was not implemented and registered in LabVIEW."},
	{LQ::InvalidSignalError, "This signal does not exist."},
	{LQ::IncompatibleArgumentsError, "The signal argument(s) are incompatible with its target."},
	{LQ::ConnectionFailedError, "The connection failed."},
	{LQ::NullPointerUseError, "Invalid LQ reference."}
};

void
errorStringFromCode(LStrHandle _retVal, int32 errorCode)
{
	copyIntoLStr(_retVal, errorMap.value(errorCode, "Unknown error."));
}
