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
		EngineAlreadyRunningError = -8001
	};
}

extern "C" {

extern void Q_DECL_EXPORT errorStringFromCode(LStrHandle retVal, int32 errorCode);

}

#endif // ERRORS_H
