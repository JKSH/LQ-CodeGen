#include "method.h"
#include "param.h"

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
	if (isConstructor())
		return _className + '*';

	return _data["retType"].toString();
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
		list.prepend(Param{_className+'*', _className.toLower()});

	return list;
}

QList<Param>
Method::paramList_dll() const
{
	QList<Param> list = paramList_bridge();

	for (Param& param : list)
		param.type = TypeConv::dllType(param.type);


	QString retType = returnType_dll();
	if (retType != "void")
	{
		// HACK. TODO: Find a more robust solution.
		if (retType != "LStrHandle" && retType != "LVBoolean*")
			retType += '*';
		list.prepend(Param{retType, "retVal"});
	}

	return list;
}

QString
Method::prototypeParams(const QList<Param>& params, bool full)
{
	QString str;
	if (full)
	{
		for (const Param& p : params)
			str += p.type + ' ' + p.name + ", ";
	}
	else
	{
		for (const Param& p : params)
			str += p.name + ", ";
	}

	// Remove the trailing ", "
	str.chop(2);
	return str;
}
