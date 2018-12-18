/*\
 * Copyright (c) 2016 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#ifndef METHOD_H
#define METHOD_H

#include "typeconv.h"
#include <QJsonObject>

struct Param;

class Method
{
public:
	Method(const QString& className, const QJsonObject& methodObj);

	// Method info
	bool isValid() const;
	bool isConst() const;
	bool isConstructor() const;
	bool isStaticMember() const;
	QString className() const {return _className;}
	QString name() const;
	QString qualifiedName(const QString& separator) const;

	// Return type names
	QString returnType_qt() const {return _data["retType"].toString();}
	QString returnType_bridge() const;
	QString returnType_dll() const;

	// Param lists
	QList<Param> paramList_raw() const;
	QList<Param> paramList_dll() const;

	// Param list codification
	static QString paramsToCode_prototype(const QList<Param>& params);

private:
	QString _className;
	QJsonObject _data;
};

#endif // METHOD_H
