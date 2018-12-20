/*\
 * Copyright (c) 2018 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#include "method.h"
#include "param.h"

#include <QList>
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
}

bool
Method::isValid() const
{
	// TODO: Validate object
	return true;
}

bool
Method::isConst() const
{
	return _data["isConst"].toBool();
}

bool
Method::isConstructor() const
{
	return _data["name"].toString() == _className;
}

bool
Method::isStaticMember() const
{
	return _data["isStaticMember"].toBool();
}

QString
Method::name() const
{
	return _data["name"].toString();
}

QString
Method::qualifiedName(const QString& separator) const
{
	return _className + separator + name();
}

// TODO: Remove the remnant references to "bridge".
//       Functor-based invokes have obsoleted the Bridge class.
QString
Method::returnType_bridge() const
{
	QString retType_qt = _data["retType"].toString();
	if (isConstructor())
	{
		switch (TypeConv::category(_className))
		{
		case TypeConv::OpaqueStruct:
			return "QByteArray";
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			return _className + '*';
		default:
			qWarning() << "WARNING: Method::returnType_bridge(): This type cannot have constructors:" << _className;
			return "";
		}
	}
	else
	{
		QString retType_bridge;
		switch (TypeConv::category(retType_qt))
		{
		case TypeConv::Void:
		case TypeConv::Boolean:
		case TypeConv::Numeric:
		case TypeConv::Enum:
		case TypeConv::SimpleStruct:
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
		case TypeConv::SimpleContainer:
		case TypeConv::FullArray:
			retType_bridge = TypeConv::bridgeType(retType_qt);
			break;
		case TypeConv::OpaqueStruct:
			retType_bridge = "QByteArray";
			break;
		default:
			qWarning() << "WARNING: Method::returnType_bridge(): Unsupported return type:" << retType_qt;
			return "";
		}
		return retType_bridge;
	}
}

QString
Method::returnType_dll() const
{
	return TypeConv::dllType(returnType_bridge());
}

QList<Param>
Method::paramList_raw() const
{
	QList<Param> list;
	const QJsonArray pArray = _data["params"].toArray();
	for (const QJsonValue& pVal : pArray)
	{
		auto paramObj = pVal.toObject();
		list << Param{
				paramObj["type"].toString(),
				paramObj["name"].toString()};
	}
	return list;
}

QList<Param>
Method::paramList_dll() const
{
	QList<Param> list = paramList_raw();

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
	if (isConstructor())
	{
		switch (classCategory)
		{
		case TypeConv::QObject:
			list.prepend(Param{"const char*", "_className"});
			break;
		default:
			break;
		}
	}

	// Prepend list with the return value (if any)
	QString retType = returnType_dll();
	switch (  TypeConv::category( returnType_bridge() )  )
	{
	// NOTE: The actions in the switch() case flow through.
	//       There is only 1 "break", before "default".
	case TypeConv::Boolean:
	case TypeConv::Numeric:
	case TypeConv::Enum:
	case TypeConv::SimpleStruct:
	case TypeConv::SimpleIdentity:
	case TypeConv::QObject:
		retType += '*';
	case TypeConv::SimpleContainer:
	case TypeConv::FullArray:
		list.prepend(Param{retType, "_retVal"});
	case TypeConv::Void:
		break;
	default:
		qWarning() << "WARNING: Method::paramList_dll(): Unsupported return type:" << returnType_bridge();
	}
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
