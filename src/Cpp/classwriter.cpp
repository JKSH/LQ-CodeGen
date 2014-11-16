#include "classwriter.h"
#include "typeconv.h"
#include "method.h"
#include "param.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

ClassWriter::ClassWriter() :
	dllH("lqwidgets.h"),
	dllC("lqwidgets.cpp"),
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
	QFile template_dllH("../../templates/Cpp/lqwidgets.h");
	if (template_dllH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllH.readAll()).split("//[TEMPLATE]");
		dllH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllH = bits[1];

		template_dllH.close();
	}
	else
		qWarning() << "Could not find lqwidgets.h template";

	QFile template_dllC("../../templates/Cpp/lqwidgets.cpp");
	if (template_dllC.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllC.readAll()).split("//[TEMPLATE]");
		dllC.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllC = bits[1];

		template_dllC.close();
	}
	else
		qWarning() << "Could not find lqwidgets.cpp template";

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
		auto paramList_dll = method.paramList_dll();
		auto paramList_brg = method.paramList_bridge();

		QString paramStr_dll = Method::paramsToCode_prototype(paramList_dll);
		QString paramStr_brg = Method::paramsToCode_prototype(paramList_brg);


		bridgeH.write(paramStr_brg.toUtf8() + ") {");
		dllH.write(paramStr_dll.toUtf8() + ");\n");
		dllC.write(paramStr_dll.toUtf8() + ")\n{\n");

		QString body_brg = funcCallBody_inBridge(method);
		QString body_dll;
		body_dll =
				"\t"    "if (!bridge)"    "\n"
				"\t\t"      "return -1;"  "\n\n";

		bool hasReturn = (retType_brg != "void");
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
			QString conversion = TypeConv::convCode_dll2Bridge(normType);
			conversion.replace("_qtType_", normType);
			conversion.replace("_dllValue_", p.name);

			body_dll
					+= "\t\t\tQ_ARG(" + QMetaObject::normalizedType(p.type.toUtf8()) + ", "
					+ conversion + "),\n";
		}
		body_dll.chop(2);
		body_dll += ");\n";

		if (hasReturn)
		{
			QByteArray retType_brg = QMetaObject::normalizedType(method.returnType_bridge().toUtf8());

			QString conversion = TypeConv::convCode_bridge2Dll(retType_brg);
			conversion.replace("_dllType_", method.returnType_dll());
			conversion.replace("_dllValue_", "retVal");
			conversion.replace("_bridgeValue_", "retVal_brg");

			body_dll
					+= "\t*retVal = " + conversion + ";\n";
		}

		body_dll += "\n\treturn 0;\n";

		bridgeH.write(body_brg.toUtf8() + "}\n");
		dllC.write(body_dll.toUtf8() + "}\n\n");

		// TODO: Bypass Bridge if method is a slot
	}
}

QString
ClassWriter::funcCallBody_inBridge(const Method &method)
{
	auto classCategory = TypeConv::category(method.className());

	// TODO: Optimize passing data across the Bridge
	// e.g. To return a LabVIEW string, write the data in the GUI thread and make the Bridge return void
	QString wrapper;
	if (method.isConstructor())
	{
		switch (classCategory)
		{
		case TypeConv::Identity:
			wrapper = "return new %METHOD_CALL%;";
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inBridge(): Explicit constructor not supported for type:" << method.className();
			return "";
		}
	}
	else
	{
		switch (classCategory)
		{
		case TypeConv::Identity:
			wrapper = "%RETURN_KEY_END%%FINAL_CALL_CONVERTED%;";
			wrapper.replace("%FINAL_CALL_CONVERTED%", TypeConv::convCode_qt2Bridge(method.returnType_qt()));
			wrapper.replace("_qtValue_", "%INSTANCE%->%METHOD_CALL%");
			break;

		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inBridge(): Method calls not supported for type:" << method.className();
			return "";
		}

		bool hasReturn = (method.returnType_bridge() != "void");
		if (hasReturn)
			wrapper.replace("%RETURN_KEY_END%", "return ");
		else
			wrapper.replace("%RETURN_KEY_END%", "");
	}

	QString methodCall = method.name() + "(%PARAMS%)";
	QString params;
	for (const Param& param : method.paramList_raw())
	{
		QByteArray normType = QMetaObject::normalizedType(param.type.toUtf8());
		QString conversion = TypeConv::convCode_bridge2Qt(normType);
		conversion.replace("_qtType_", normType);
		conversion.replace("_bridgeValue_", param.name);

		params += conversion + ", ";
	}
	params.chop(2);

	methodCall.replace("%PARAMS%", params);
	wrapper.replace("%CLASS%", method.className());
	wrapper.replace("%INSTANCE%", method.className().toLower());
	wrapper.replace("%METHOD_CALL%", methodCall);

	return wrapper;
}
