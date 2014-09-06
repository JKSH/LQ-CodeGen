#ifndef METHOD_H
#define METHOD_H

#include "param.h"
#include "typeconv.h"
#include <QJsonObject>

class Method
{
public:
	Method(const QString& className, const QJsonObject& methodObj) :
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
	bool isValid() const
	{
		// TODO: Validate object
		return true;
	}

	QString name() const {return _data["name"].toString();}
	QString qualifiedName(const QString& separator) const { return _className + separator + name();}
	bool isConstructor() const { return _data["name"].toString() == _className; }

	QString returnType_bridge() const
	{
		if (isConstructor())
			return _className + '*';

		return _data["retType"].toString();
	}

	QString returnType_dll() const
	{
		return TypeConv::dllType(returnType_bridge());
	}

	QList<Param> paramList_raw() const
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

	QList<Param> paramList_bridge() const
	{
		QList<Param> list = paramList_raw();
		if (!isConstructor())
			list.prepend(Param{_className+'*', _className.toLower()});

		return list;
	}
	QList<Param> paramList_dll() const
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

	static QString prototypeParams(const QList<Param>& params, bool full = true)
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

private:
	QString _className;
	QJsonObject _data;
};

#endif // METHOD_H
