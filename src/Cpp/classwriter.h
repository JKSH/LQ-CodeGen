#ifndef CLASSWRITER_H
#define CLASSWRITER_H

#include "jsonparser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QStringList>

class TypeConv
{
	static QJsonObject _qt2dll;

public:
	static void init(const QJsonArray& conversions)
	{
		// TODO: More robust checks
		_qt2dll = JsonParser::arrayToObject(conversions, "qtType");
	}

	/// This is a lossy (irreversible) conversion.
	static QString dllType(const QString& qtType)
	{
		// Pointer
		// TODO: See if quintptr is any good
		if (qtType.contains('*'))
			return "quint32";

		// TODO: Study normalization
		QString tmp = QMetaObject::normalizedType(qtType.toUtf8());

		// Unchanged types
		if (tmp == "void")
			return tmp;

		if (_qt2dll.contains(tmp))
			return _qt2dll[tmp].toObject()["dllType"].toString();

		qWarning() << "WARNING: Unsupported type:" << qtType;
		return tmp;
	}

	static QString conversion_toDll(const QString& qtType)
	{
		// TODO: Return default if non-existent
		return _qt2dll[qtType].toObject()["qt2dll"].toString();
	}
	static QString conversion_fromDll(const QString& qtType)
	{
		// TODO: Return default if non-existent
		return _qt2dll[qtType].toObject()["dll2qt"].toString();
	}
};

struct Param
{
	QString type;
	QString name;
};

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

class ClassWriter
{
public:
	ClassWriter();

	bool startWriting();
	void stopWriting();

	void writeClass(const QJsonObject& classObj)
	{
		currentClass = classObj["name"].toString();
		if (currentClass.isEmpty())
		{
			qWarning() << "Not a class object:" << classObj;
			return;
		}

		for (const QJsonValue& mVal : classObj["methods"].toArray())
		{
			auto methodObj = mVal.toObject();
			Method method(currentClass, methodObj);
			if (!method.isValid())
			{
				qWarning() << "Invalid method found in class" << currentClass << ":";
				qWarning() << methodObj;
				continue;
			}

			QString retType_brg = method.returnType_bridge();
			bridgeH.write('\t' + retType_brg.toUtf8() + ' ');
			dllH.write("extern qint32 Q_DECL_EXPORT ");
			dllC.write(       "qint32 Q_DECL_EXPORT\n");

			QByteArray funcName = method.qualifiedName("_").toUtf8();
			bridgeH.write(funcName + '(');
			dllH.write(funcName + '(');
			dllC.write(funcName + '(');

			// TODO: Don't let the Method know about DLL vs Bridge
			auto paramList = method.paramList_raw();
			auto paramList_dll = method.paramList_dll();
			auto paramList_brg = method.paramList_bridge();

			QString paramStr_dll = Method::prototypeParams(paramList_dll);
			QString paramStr_brg = Method::prototypeParams(paramList_brg);


			bridgeH.write(paramStr_brg.toUtf8() + ") {");
			dllH.write(paramStr_dll.toUtf8() + ");\n");
			dllC.write(paramStr_dll.toUtf8() + ")\n{\n");

			QString body_brg;
			QString body_dll;

			body_dll =
					"\t"    "if (!bridge)"    "\n"
					"\t\t"      "return -1;"  "\n\n";

			bool hasReturn = (retType_brg != "void");
			if (method.isConstructor())
			{
				body_brg += "return new " + currentClass;
			}
			else
			{
				if (hasReturn)
					body_brg = "return ";

				body_brg += currentClass.toLower() + "->" + method.name();
			}
			body_brg += '(' + Method::prototypeParams(paramList, false) + ");";

			if (hasReturn)
				body_dll += "\t" + retType_brg + " retVal_brg;\n";

			body_dll +=
					"\t"        "QMetaObject::invokeMethod(bridge,"         "\n"
					"\t\t\t"            "\"" + funcName + "\","             "\n"
					"\t\t\t"            "Qt::BlockingQueuedConnection,"     "\n";

			if (hasReturn)
			{
				body_dll +=
						"\t\t\tQ_RETURN_ARG(" + retType_brg + ", retVal_brg)," "\n";
			}

			for (const Param& p : paramList_brg)
			{
				QByteArray normType = QMetaObject::normalizedType(p.type.toUtf8());
				QString conversion = TypeConv::conversion_fromDll(normType);
				conversion.replace("_dllValue_", p.name);

				// TODO: Default to static_cast
				if (conversion.isEmpty())
				{
					if (TypeConv::dllType(normType) == normType)
						conversion = normType;
					else
						conversion = '(' + normType + ')' + p.name;
				}
				body_dll
						+= "\t\t\tQ_ARG(" + QMetaObject::normalizedType(p.type.toUtf8()) + ", "
						+ conversion + "),\n";
			}
			body_dll.chop(2);
			body_dll += ");\n";

			if (hasReturn)
			{
				QByteArray retType_brg = QMetaObject::normalizedType(method.returnType_bridge().toUtf8());

				QString retType_dll = method.returnType_dll();

				QString conversion = TypeConv::conversion_toDll(retType_brg);
				conversion.replace("_dllValue_", "retVal");
				conversion.replace("_qtValue_", "retVal_brg");

				// TODO: Default to static_cast
				if (conversion.isEmpty())
				{
					if (retType_dll == retType_brg)
						conversion = "retVal_brg";
					else
						conversion = '(' + retType_dll + ")retVal_brg";
				}

				body_dll
						+= "\t*retVal = " + conversion + ";\n";
			}

			body_dll += "\n\treturn 0;\n";

			bridgeH.write(body_brg.toUtf8() + "}\n");
			dllC.write(body_dll.toUtf8() + "}\n\n");

			// TODO: Bypass Bridge if method is a slot
		}
	}

private:
	QFile dllH;
	QFile dllC;
	QFile bridgeH;

	QString _footer_dllH;
	QString _footer_dllC;
	QString _footer_bridgeH;

	QString currentClass;
};

#endif // CLASSWRITER_H
