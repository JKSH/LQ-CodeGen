#ifndef CLASSWRITER_H
#define CLASSWRITER_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QStringList>

#include "method.h"
#include "typeconv.h"

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
