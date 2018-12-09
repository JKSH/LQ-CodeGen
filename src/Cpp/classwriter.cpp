/*\
 * Copyright (c) 2016 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

#include "classwriter.h"
#include "typeconv.h"
#include "method.h"
#include "param.h"

#include <QJsonObject>
#include <QJsonArray>

#include <QDebug>

ClassWriter::ClassWriter() :
	dllH("lqlibinterface.h"),
	dllC("lqlibinterface.cpp"),
	bridgeH("lqbridge.h")
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
	QFile template_dllH("../../templates/Cpp/lqlibinterface.h");
	if (template_dllH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllH.readAll()).split("//[TEMPLATE]");
		dllH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllH = bits[1];

		template_dllH.close();
	}
	else
		qWarning() << "Could not find lqlibinterface.h template";

	QFile template_dllC("../../templates/Cpp/lqlibinterface.cpp");
	if (template_dllC.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllC.readAll()).split("//[TEMPLATE]");
		dllC.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllC = bits[1];

		template_dllC.close();
	}
	else
		qWarning() << "Could not find lqlibinterface.cpp template";

	QFile template_bridgeH("../../templates/Cpp/lqbridge.h");
	if (template_bridgeH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_bridgeH.readAll()).split("//[TEMPLATE]");
		bridgeH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_bridgeH = bits[1];

		template_bridgeH.close();
	}
	else
		qWarning() << "Could not find lqbridge.h template";

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
	QString currentClass = classObj["name"].toString();
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
		dllC.write("qint32\n");

		QByteArray funcName = method.qualifiedName("_").toUtf8();
		bridgeH.write(funcName + '(');
		dllH.write(funcName + '(');
		dllC.write(funcName + '(');

		QString paramStr_dll = Method::paramsToCode_prototype(method.paramList_dll());
		QString paramStr_brg = Method::paramsToCode_prototype(method.paramList_bridge());

		bridgeH.write(paramStr_brg.toUtf8() + ") {");
		dllH.write(paramStr_dll.toUtf8() + ");\n");
		dllC.write(paramStr_dll.toUtf8() + ")\n{\n");

		QString body_brg = funcCallBody_inBridge(method);
		QString body_dll = funcCallBody_inDll(method);

		bridgeH.write(body_brg.toUtf8() + "}\n");
		dllC.write(body_dll.toUtf8() + "}\n\n");

		// TODO: Bypass Bridge if method is a slot
	}
}

QString
ClassWriter::funcCallBody_inDll(const Method &method)
{
	QString body_dll;
	body_dll =
			"\t"      "if (!bridge)"    "\n"
			"\t\t"        "return LQ::EngineNotRunningError;"           "\n"
			""        "%CHECK_INSTANCE_NULLITY%"                        "\n"
			""        "%RETURN_VAR_INTERMEDIATE%"
			"\t"      "QMetaObject::invokeMethod(bridge,"               "\n"
			"\t\t\t"          "\"" + method.qualifiedName("_") + "\","  "\n"
			"\t\t\t"          "Qt::BlockingQueuedConnection,"           "\n"
			""                "%RETURN_LINE_INVOKE%"
			""                "%CLASSNAME_LINE_INVOKE%"
			""                "%INSTANCE_LINE_INVOKE%"
			""                "%INPUT_LINES_INVOKE%";

	QString thisClass = method.className();
	if ( method.isConstructor() && TypeConv::category(thisClass) == TypeConv::QObject )
		body_dll.replace("%CLASSNAME_LINE_INVOKE%", "\t\t\tQ_ARG(const char*, _className),\n");
	else
		body_dll.replace("%CLASSNAME_LINE_INVOKE%", "");

	if (!method.isConstructor() && !method.isStaticMember())
	{
		QString conversion = TypeConv::convCode_dll2Bridge(thisClass);
		conversion.replace("_qtType_", TypeConv::instanceType_bridge(thisClass));
		conversion.replace("_dllValue_", "_instance");

		switch (TypeConv::category(method.className()))
		{
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			body_dll.replace("%CHECK_INSTANCE_NULLITY%", "\tif (!_instance)\n\t\treturn LQ::NullPointerUseError;\n");
		default:
			body_dll.replace("%CHECK_INSTANCE_NULLITY%", "");
		}
		body_dll.replace("%INSTANCE_LINE_INVOKE%", "\t\t\tQ_ARG(" + TypeConv::instanceType_bridge(thisClass) + ", "	+ conversion + "),\n");
	}
	else
	{
		body_dll.replace("%CHECK_INSTANCE_NULLITY%", "");
		body_dll.replace("%INSTANCE_LINE_INVOKE%", "");
	}

	QString retType_brg = method.returnType_bridge();
	bool hasReturn = (retType_brg != "void");
	if (hasReturn)
	{
		body_dll.replace("%RETURN_VAR_INTERMEDIATE%", "\t%RETURN_TYPE_BRIDGE% retVal_brg;\n");
		body_dll.replace("%RETURN_LINE_INVOKE%", "\t\t\tQ_RETURN_ARG(%RETURN_TYPE_BRIDGE%, retVal_brg)," "\n");
		body_dll.replace("%RETURN_TYPE_BRIDGE%", retType_brg);
	}
	else
	{
		body_dll.replace("%RETURN_VAR_INTERMEDIATE%", "");
		body_dll.replace("%RETURN_LINE_INVOKE%", "");
	}

	QString inputLines;
	for (const Param& p : method.paramList_raw())
	{
		QByteArray normType = QMetaObject::normalizedType(p.type.toUtf8());
		QString conversion = TypeConv::convCode_dll2Bridge(normType);
		conversion.replace("_qtType_", normType);
		conversion.replace("_dllValue_", p.name);

		inputLines
				+= "\t\t\tQ_ARG(" + TypeConv::bridgeType(p.type) + ", "
				+ conversion + "),\n";
	}
	body_dll.replace("%INPUT_LINES_INVOKE%", inputLines);

	body_dll.chop(2);
	body_dll += ");\n";

	if (hasReturn)
	{
		QString conversion = TypeConv::convCode_bridge2Dll(retType_brg);
		conversion.replace("_dllType_", method.returnType_dll());
		conversion.replace("_dllValue_", "_retVal");
		conversion.replace("_bridgeValue_", "retVal_brg");

		body_dll
				+= "\t%RETURN_KEY%" + conversion + ";\n";

		switch (TypeConv::category(retType_brg))
		{
		case TypeConv::Boolean:
		case TypeConv::Numeric:
		case TypeConv::Enum:
		case TypeConv::SimpleStruct:
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			body_dll.replace("%RETURN_KEY%", "*_retVal = ");
			break;
		case TypeConv::SimpleContainer:
		case TypeConv::FullArray:
		case TypeConv::OpaqueStruct:
			body_dll.replace("%RETURN_KEY%", "");
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inDll(): Unsupported return type:" << retType_brg;
		}
	}

	body_dll += "\n\treturn LQ::NoError;\n";
	return body_dll;
}

QString
ClassWriter::funcCallBody_inBridge(const Method &method)
{
	auto classCategory = TypeConv::category(method.className());
	bool hasReturn = (method.returnType_bridge() != "void");

	// TODO: Optimize passing data across the Bridge
	// e.g. To return a LabVIEW string, write the data in the GUI thread and make the Bridge return void
	QString wrapper;
	if (method.isConstructor())
	{
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:
			wrapper = "return serialize(%METHOD_CALL%);";
			break;
		case TypeConv::SimpleIdentity:
			wrapper = "return new %METHOD_CALL%;";
			break;
		case TypeConv::QObject:
			wrapper = "return newLQObject<%CLASS%>(%PARAMS%);";
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inBridge(): This type cannot have constructors:" << method.className();
			return "";
		}
	}
	else if (method.isStaticMember())
	{
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			wrapper = "%RETURN_KEY_END%%CLASS%::%METHOD_CALL%;";
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inBridge(): This type cannot have methods:" << method.className();
			return "";
		}

		if (hasReturn)
			wrapper.replace("%RETURN_KEY_END%", "return ");
		else
			wrapper.replace("%RETURN_KEY_END%", "");
	}
	else
	{
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:

			wrapper = "\n"
					"\t\t"  "%CLASS% thisInstance = deserialize<%CLASS%>(_instance);"  "\n"
					"\t\t"  "%CALL_STMT_MAIN%;"                                                       "\n"
							"%SERIAlIZE_LINE%"
					"\t\t"  "%RETURN_STMT_END%"                                                       "\n"
					"\t";
			break;

		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			wrapper = "%RETURN_KEY_END%%FINAL_CALL_CONVERTED%;";
			wrapper.replace("%FINAL_CALL_CONVERTED%", TypeConv::convCode_qt2Bridge(method.returnType_qt()));
			wrapper.replace("_qtValue_", "_instance->%METHOD_CALL%");
			break;

		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inBridge(): This type cannot have methods:" << method.className();
			return "";
		}

		if (method.isConst())
			wrapper.replace("%SERIAlIZE_LINE%", "");
		else
			wrapper.replace("%SERIAlIZE_LINE%", "\t\tcopyIntoLStr(_instance, serialize(thisInstance));\n");

		if (hasReturn)
		{
			wrapper.replace("%CALL_STMT_MAIN%", "%RETURN_TYPE% retVal = " + TypeConv::convCode_qt2Bridge(method.returnType_qt()));
			wrapper.replace("_qtValue_", "thisInstance.%METHOD_CALL%");

			wrapper.replace("%RETURN_KEY_END%", "return ");
			wrapper.replace("%RETURN_STMT_END%", "return retVal;");
			wrapper.replace("%RETURN_TYPE%", method.returnType_bridge());
		}
		else
		{
			wrapper.replace("%CALL_STMT_MAIN%", "thisInstance.%METHOD_CALL%");

			wrapper.replace("%RETURN_KEY_END%", "");
			wrapper.replace("%RETURN_STMT_END%", "");
		}
	}

	QString methodCall = method.name() + "(%PARAMS%)";
	QString params;
	if (method.isConstructor() && classCategory == TypeConv::QObject)
		params += "_className, ";
	for (const Param& param : method.paramList_raw())
	{
		QByteArray normType = QMetaObject::normalizedType(param.type.toUtf8());
		QString conversion = TypeConv::convCode_bridge2Qt(normType);
		conversion.replace("_qtType_", normType);
		conversion.replace("_bridgeValue_", param.name);

		params += conversion + ", ";
	}
	params.chop(2);

	wrapper.replace("%CLASS%", method.className());
	wrapper.replace("%METHOD_CALL%", methodCall);
	wrapper.replace("%PARAMS%", params);

	return wrapper;
}
