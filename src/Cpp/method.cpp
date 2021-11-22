/*\
 * Copyright (c) 2018 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#include "method.h"

#include <QJsonArray>

#include <QDebug>

Method::Method(const QString& className, const QJsonObject& methodObj) :
	_className(className),
	_data(methodObj)
{
	// TODO: Validate object, including params

	/*
		Error if:
		- Constructor has a retType
		- Non-constructor has no retType
	*/

	const QJsonArray pArray = _data["params"].toArray();
	for (const QJsonValue& pVal : pArray)
	{
		auto paramObj = pVal.toObject();
		_paramList << Param
		{
			paramObj["type"].toString(),
			paramObj["name"].toString()
		};
	}
}

bool
Method::isValid() const
{
	// TODO: Validate object
	return true;
}

bool
Method::isConstructor() const
{
	return _data["name"].toString() == _className;
}

QString
Method::qualifiedName(const QString& separator) const
{
	return _className + separator + name();
}

QString
Method::returnType_dll() const
{
	QString retType_qt = returnType_qt();
	if (isConstructor())
	{
		// Treat constructors as methods that return a class instance
		switch (TypeConv::category(_className))
		{
		case TypeConv::OpaqueStruct:
			retType_qt = _className;
			break;
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			retType_qt = _className + '*';
			break;
		default:
			qWarning() << "WARNING: Method::returnType_dll(): This type cannot have constructors:" << _className;
		}
	}

	QString retType_dll = TypeConv::dllType(retType_qt);
	switch ( TypeConv::category(retType_qt) )
	{
	case TypeConv::Void:
	case TypeConv::OpaqueStruct:
	case TypeConv::SimpleContainer:
	case TypeConv::FullArray:
		return retType_dll;
	case TypeConv::Boolean:
	case TypeConv::Numeric:
	case TypeConv::Enum:
	case TypeConv::SimpleStruct:
	case TypeConv::SimpleIdentity:
	case TypeConv::QObject:
	case TypeConv::LQCustomCast:
		return retType_dll + '*'; // Parameter is returned to LabVIEW by pointer.
	default:
		qWarning() << "WARNING: Method::returnType_dll(): Unsupported return type:" << retType_qt;
		return "";
	}
}

QList<Param>
Method::paramList_dll() const
{
	QList<Param> list = paramList_qt();

	for (Param& param : list)
	{
		QString qtType = param.type;
		param.type = TypeConv::dllType(qtType);

		// TODO: Intelligent pointerification
		switch (   TypeConv::category(  QMetaObject::normalizedType( qtType.toUtf8() )  )   )
		{
		case TypeConv::Boolean:
		case TypeConv::SimpleStruct:
			param.type += '*';
		case TypeConv::Numeric:
		case TypeConv::Enum:
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
		case TypeConv::SimpleContainer: // TODO: Use Handle Pointers for containers, for efficiency
		case TypeConv::FullArray:
		case TypeConv::LQCustomCast:
			break;
		case TypeConv::OpaqueStruct:
			param.type = "LStrHandle";
			break;
		default:
			qWarning() << "WARNING: Method::paramList_dll(): Unsupported input arg type:" << qtType;
		}
	}

	// Prepend list with Instance (if applicable)
	auto classCategory = TypeConv::category(_className);
	if (!isConstructor() && !isStaticMember())
	{
		switch (classCategory)
		{
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
		case TypeConv::OpaqueStruct:
			list.prepend(Param{TypeConv::instanceType_dll(_className), "_instance"});
			break;
		default:
			qWarning() << "WARNING: Method::paramList_dll(): This type cannot have methods:" << _className;
			break;
		}
	}

	// Prepend list with Class Name (if QObject constructor)
	if (isConstructor() && classCategory == TypeConv::QObject)
		list.prepend(Param{"const char*", "_className"});

	// Prepend list with the return value (if any)
	auto returnCategory = TypeConv::category(returnType_qt());
	if (isConstructor() || returnCategory != TypeConv::Void)
		list.prepend(Param{returnType_dll(), "_retVal"});
	return list;
}

QString
Method::paramsToCode_prototype(const QList<Param>& params)
{
	QStringList paramStrings;
	for (const Param& p : params)
		paramStrings << p.type + ' ' + p.name;

	return paramStrings.join(", ");
}
