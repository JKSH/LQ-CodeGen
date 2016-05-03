/*\
 * Copyright (c) 2016 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#ifndef TYPECONV_H
#define TYPECONV_H

#include <QString>
class QJsonArray;

namespace TypeConv
{
	enum Category {
		Invalid,
		Void,
		Boolean,
		Numeric,
		Enum,
		SimpleStruct,
		OpaqueStruct,
		SimpleIdentity,
		QObject,
		SimpleContainer,
		FullArray
	};

	void init(const QJsonArray& conversions, Category category);

	Category category(const QString& qtType);
	QString typeBase(const QString& qtType);
	QString innerType(const QString& qtType);
	QString bridgeType(const QString& qtType);
	QString dllType(const QString& qtType);

	QString instanceType_dll(const QString& qtType);
	QString instanceType_bridge(const QString& qtType);

	QString convCode_bridge2Dll(const QString& qtType);
	QString convCode_dll2Bridge(const QString& qtType);
	QString convCode_qt2Bridge(const QString& qtType);
	QString convCode_bridge2Qt(const QString& qtType);
}

#endif // TYPECONV_H
