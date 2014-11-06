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
Method::isConstructor() const
{
	return _data["name"].toString() == _className;
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

QString
Method::returnType_bridge() const
{
	QString retType_qt = _data["retType"].toString();
	if (isConstructor())
	{
		switch (TypeConv::category(_className))
		{
		case TypeConv::Identity:
			return _className + '*';
		default:
			qWarning() << "WARNING: Method::returnType_bridge(): Explicit constructor not supported for type:" << _className;
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
		case TypeConv::SimpleStruct:
		case TypeConv::Identity:
		case TypeConv::Container:
			retType_bridge = TypeConv::bridgeType(retType_qt);
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
	for (const QJsonValue& pVal : _data["params"].toArray())
	{
		auto paramObj = pVal.toObject();
		list << Param{
				paramObj["type"].toString(),
				paramObj["name"].toString()};
	}
	return list;
}

QList<Param>
Method::paramList_bridge() const
{
	QList<Param> list = paramList_raw();
	if (!isConstructor())
	{
		switch (   TypeConv::category(  QMetaObject::normalizedType( _className.toUtf8() )  )   )
		{
		case TypeConv::Boolean:
		case TypeConv::Numeric:
		case TypeConv::SimpleStruct:
		case TypeConv::Identity:
		case TypeConv::Container:
			list.prepend(Param{_className+'*', _className.toLower()});
			break;
		default:
			break;
		}
	}

	return list;
}

QList<Param>
Method::paramList_dll() const
{
	QList<Param> list = paramList_bridge();

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
		case TypeConv::Identity:
		case TypeConv::Container: // TODO: Use Handle Pointers for containers, for efficiency
			break;
		default:
			qWarning() << "WARNING: Method::paramList_dll(): Unsupported input arg type:" << qtType;
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
	case TypeConv::SimpleStruct:
	case TypeConv::Identity:
		retType += '*';
	case TypeConv::Container:
		list.prepend(Param{retType, "retVal"});
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
	QString str;
	for (const Param& p : params)
		str += p.type + ' ' + p.name + ", ";

	// Remove the trailing ", "
	str.chop(2);
	return str;
}