#ifndef ERRORS_H
#define ERRORS_H

#include <QtGlobal>
#include "extcode.h"

namespace LQ
{
	enum Error
	{
		/*
			See http://zone.ni.com/reference/en-XX/help/371361L-01/lvhowto/def_cust_errors/
			for all allowed ranges.
			  - Have errors < -8000
			  - Have warnings > 8000
		*/
		NoError = 0,
		EngineNotRunningError = -8000,
		EngineAlreadyRunningError = -8001,
		NotAnLQObjectError = -8002,
		InvalidSignalError = -8003,
		IncompatibleArgumentsError = -8004,
		ConnectionFailedError = -8005
	};
}

extern "C" {

extern void Q_DECL_EXPORT errorStringFromCode(LStrHandle _retVal, int32 errorCode);

}

#endif // ERRORS_H
