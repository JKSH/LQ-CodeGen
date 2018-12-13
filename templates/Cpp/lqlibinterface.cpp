/*\
 * Copyright (c) 2016 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#include "lqlibinterface.h"
#include "lqbridge.h"
#include "lqerrors.h"

static qint32
lqInvoke(std::function<void()> func)
{
	if (!bridge)
		return LQ::EngineNotRunningError;

	QMetaObject::invokeMethod(qApp, func, Qt::BlockingQueuedConnection);
	return LQ::NoError;
}

static qint32
lqInvoke(quintptr _instance, std::function<void()> func)
{
	if (!_instance)
		return LQ::NullPointerUseError;
	return lqInvoke(func);
}

// NOTE: This file contains auto-generated code. Do not modify by hand.

//[TEMPLATE]
