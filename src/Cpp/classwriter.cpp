/*\
 * Copyright (c) 2018 Sze Howe Koh
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
	dllC("lqlibinterface.cpp")
{}

bool
ClassWriter::startWriting()
{
	bool ok = dllH.open(QFile::WriteOnly|QFile::Text)
			&& dllC.open(QFile::WriteOnly|QFile::Text);

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

	return true;
}

void
ClassWriter::stopWriting()
{
	dllH.write(_footer_dllH.toUtf8());
	dllC.write(_footer_dllC.toUtf8());

	dllH.close();
	dllC.close();
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

	const QJsonArray mArray = classObj["methods"].toArray();
	for (const QJsonValue& mVal : mArray)
	{
		auto methodObj = mVal.toObject();
		Method method(currentClass, methodObj);
		if (!method.isValid())
		{
			qWarning() << "Invalid method found in class" << currentClass << ":";
			qWarning() << methodObj;
			continue;
		}

		dllH.write("extern qint32 Q_DECL_EXPORT ");
		dllC.write("qint32\n");

		QByteArray funcName = method.qualifiedName("_").toUtf8();
		dllH.write(funcName + '(');
		dllC.write(funcName + '(');

		QString paramStr_dll = Method::paramsToCode_prototype(method.paramList_dll());
		dllH.write(paramStr_dll.toUtf8() + ");\n");
		dllC.write(paramStr_dll.toUtf8() + ")\n{\n");

		QString body_dll = funcCallBody_inDll(method);
		dllC.write(body_dll.toUtf8() + "}\n\n");
	}
}

QString
ClassWriter::funcCallBody_inDll(const Method &method)
{
	QString body_dll =
			"\t"      "return lqInvoke(%PASS_INSTANCE%[=]"              "\n"
			"\t"      "{"                                               "\n"
			""            "%BODY%"
			"\t"      "});"                                             "\n";

	auto classCategory = TypeConv::category(method.className());
	bool isInstanceMethod =
			(classCategory == TypeConv::SimpleIdentity || classCategory == TypeConv::QObject)
			&& !method.isConstructor()
			&& !method.isStaticMember();

	if (isInstanceMethod)
		body_dll.replace("%PASS_INSTANCE%", "_instance, ");
	else
		body_dll.replace("%PASS_INSTANCE%", "");

	body_dll.replace("%BODY%", funcCallBody_inLambda(method));
	return body_dll;
}

QString
ClassWriter::funcCallBody_inLambda(const Method& method)
{
	QString body =
			""      "%UNPACK_INSTANCE%"
			"\t\t"  "%RETURN_ASSIGNMENT_1%"  "%CALL_EXPR%;"  "\n"
			""      "%RETURN_ASSIGNMENT_2%"
			""      "%REPACK_INSTANCE%";

	bool hasReturn = (method.returnType_dll() != "void");
	auto classCategory = TypeConv::category(method.className());
	if (method.isConstructor() || method.isStaticMember())
		body.replace("%UNPACK_INSTANCE%", "");
	else
	{
		body.replace("%UNPACK_INSTANCE%", "\t\tauto instance = %UNPACK_INSTANCE_ACTION%<%CLASS%%PTR%>(_instance);\n");
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:
			body.replace("%UNPACK_INSTANCE_ACTION%", "deserialize");
			body.replace("%PTR%", "");
			break;
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			body.replace("%UNPACK_INSTANCE_ACTION%", "reinterpret_cast");
			body.replace("%PTR%", "*");
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inLambda(): This type cannot be instantiated for method calls:" << method.className();
			return "";
		}
	}

	// Call the function
	if (method.isConstructor())
	{
		QString methodCallWrapper;
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:
			methodCallWrapper = "%METHOD_CALL%";
			break;
		case TypeConv::SimpleIdentity:
			methodCallWrapper = "new %METHOD_CALL%";
			break;
		case TypeConv::QObject:
			methodCallWrapper = "newLQObject<%CLASS%>(%PARAMS%)";
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inLambda(): This type cannot have constructors:" << method.className();
			return "";
		}
		body.replace("%CALL_EXPR%", methodCallWrapper);
	}
	else if (method.isStaticMember())
	{
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			body.replace("%CALL_EXPR%", "%CLASS%::%METHOD_CALL%");
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inLambda(): This type cannot have methods:" << method.className();
			return "";
		}
	}
	else
	{
		body.replace("%CALL_EXPR%", "instance%CALL_OPERATOR%%METHOD_CALL%");
		switch (classCategory)
		{
		case TypeConv::OpaqueStruct:
			body.replace("%CALL_OPERATOR%", ".");
			break;

		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			body.replace("%CALL_OPERATOR%", "->");
			break;

		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inLambda(): This type cannot have methods:" << method.className();
			return "";
		}
	}

	// Write actual method call
	QString methodCall = method.name() + "(%PARAMS%)";
	QStringList params;
	if (method.isConstructor() && classCategory == TypeConv::QObject)
		params << "_className";
	for (const Param& param : method.paramList_raw())
	{
		QByteArray normType = QMetaObject::normalizedType(param.type.toUtf8());
		QString conversion = TypeConv::convCode_dll2Qt(normType);
		conversion.replace("_qtType_", normType);
		conversion.replace("_dllValue_", param.name);

		params << conversion;
	}

	body.replace("%CLASS%", method.className());
	body.replace("%METHOD_CALL%", methodCall);
	body.replace("%PARAMS%", params.join(", "));

	// Repack instance
	if (classCategory == TypeConv::OpaqueStruct
			&& !method.isConstructor()
			&& !method.isConst()
			&& !method.isStaticMember())
	{
		body.replace("%REPACK_INSTANCE%", "\t\t_instance << instance;\n");
	}
	else
		body.replace("%REPACK_INSTANCE%", "");

	// Finalize return value
	if (hasReturn)
	{
		QString retType = method.returnType_qt();
		if (method.isConstructor())
			retType = method.className(); // HACK: This should often be a POINTER to the class type

		QString storeReturn;
		QString retOp;
		QString retConv;
		switch (TypeConv::category(retType))
		{
		case TypeConv::Boolean:
		case TypeConv::Numeric:
		case TypeConv::Enum:
		case TypeConv::SimpleStruct:
			storeReturn = '*';
			retOp       = '=';
			retConv     = "retVal";
			break;
		case TypeConv::SimpleIdentity:
		case TypeConv::QObject:
			storeReturn = '*';
			retOp       = '=';
			retConv     = "reinterpret_cast<quintptr>(retVal)";
			break;
		case TypeConv::SimpleContainer:
		case TypeConv::FullArray:
		case TypeConv::OpaqueStruct:
			retOp       = "<<";
			retConv     = "retVal";
			break;
		default:
			qWarning() << "WARNING: ClassWriter::funcCallBody_inDll(): This method cannot return:" << method.name();
			break;
		}

		storeReturn += "_retVal " + retOp + ' ';
		if (retConv == "retVal")
		{
			// If the expression is a single variable...
			body.replace("%RETURN_ASSIGNMENT_1%", storeReturn);
			body.replace("%RETURN_ASSIGNMENT_2%", "");
		}
		else
		{
			// If the expression is longer...
			body.replace("%RETURN_ASSIGNMENT_1%", "auto retVal = ");
			body.replace("%RETURN_ASSIGNMENT_2%", "\t\t" + storeReturn + retConv + ";\n");
		}
	}
	else
	{
		body.replace("%RETURN_ASSIGNMENT_1%", "");
		body.replace("%RETURN_ASSIGNMENT_2%", "");
	}

	return body;
}
