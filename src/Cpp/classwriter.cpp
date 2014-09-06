#include "classwriter.h"
#include "typeconv.h"
#include "method.h"
#include "param.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

ClassWriter::ClassWriter() :
	dllH("labqt.h"),
	dllC("labqt.cpp"),
	bridgeH("bridge.h")
{}

bool
ClassWriter::startWriting()
{
	bool ok = dllH.open(QFile::WriteOnly|QFile::Text)
			&& dllC.open(QFile::WriteOnly|QFile::Text)
			&& bridgeH.open(QFile::WriteOnly|QFile::Text);

	if (!ok)
	{
		qWarning() << "Couldn't open something!";
		return false;
	}

	// TODO: Warn if templates are malformed

	// TODO: Refactor
	QFile template_dllH("../../templates/Cpp/labqt.h");
	if (template_dllH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllH.readAll()).split("//[TEMPLATE]");
		dllH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllH = bits[1];

		template_dllH.close();
	}
	else
		qWarning() << "Could not find labqt.h template";

	QFile template_dllC("../../templates/Cpp/labqt.cpp");
	if (template_dllC.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllC.readAll()).split("//[TEMPLATE]");
		dllC.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllC = bits[1];

		template_dllC.close();
	}
	else
		qWarning() << "Could not find labqt.cpp template";

	QFile template_bridgeH("../../templates/Cpp/bridge.h");
	if (template_bridgeH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_bridgeH.readAll()).split("//[TEMPLATE]");
		bridgeH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_bridgeH = bits[1];

		template_bridgeH.close();
	}
	else
		qWarning() << "Could not find bridge.h template";

	return true;
}

void
ClassWriter::stopWriting()
{
	dllH.write(_footer_dllH.toUtf8());
	dllC.write(_footer_dllC.toUtf8());
	bridgeH.write(_footer_bridgeH.toUtf8());

	dllH.close();
	dllC.close();
	bridgeH.close();
}

void
ClassWriter::writeClass(const QJsonObject& classObj)
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
