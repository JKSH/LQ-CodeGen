#include "typeconv.h"

#include <QMetaObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

#include <QDebug>

QJsonObject _bridge2dll;
QMap<QString, TypeConv::Category> _categories;

void
TypeConv::init(const QJsonArray& conversions, Category category)
{
	// TODO: More robust checks
	// TODO: Warn if the type is already registered
	for (const QJsonValue& obj : conversions)
	{
		QString objName = obj.toObject()["name"].toString();

		if (category == FullArray)
		{
			// Store base names of FullArrays, e.g. "QVector"
			int templateStartIdx = objName.indexOf('<');
			if (templateStartIdx != -1)
				objName = objName.left(templateStartIdx);
			else
				qWarning() << "WARNING: TypeConv::init(): Not a FullArray:" << objName;
		}

		_bridge2dll[objName] = obj;
		_categories[objName] = category;

		// TODO: Protect against non-pointer parameters?
		// Need to support both base (for constructors) and pointer names (for params)
		if (category == SimpleIdentity || category == QObject)
		{
			_bridge2dll[objName+'*'] = obj;
			_categories[objName+'*'] = category;
		}
	}
}

/*!
	Reduces \a qtType to its minimum, normalized form. This discards minor
	variations in type names, so that they can be compared for equality.

	For templated container classes, this function also removes information
	about the type held inside the container.

	e.g. "const QVector<qreal>&" becomes simply "QVector".
*/
QString
TypeConv::typeBase(const QString& qtType)
{
	QByteArray tmp = QMetaObject::normalizedType(qtType.toUtf8());

	// TODO: Handle pointers too

	int templateStartIdx = tmp.indexOf('<');
	if (templateStartIdx != -1)
		return tmp.left(templateStartIdx);
	return tmp;
}

QString
TypeConv::innerType(const QString& qtType)
{
	int templateStartIdx = qtType.indexOf('<');
	int templateEndIdx = qtType.lastIndexOf('>');
	if (templateStartIdx != -1 && templateEndIdx != -1)
		return qtType.left(templateEndIdx).mid(templateStartIdx+1);

	return "";
}

TypeConv::Category
TypeConv::category(const QString& qtType)
{
	return _categories.value(typeBase(qtType), Invalid);
}

QString
TypeConv::bridgeType(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Void:
	case Boolean:
	case Numeric:
	case Enum:
	case SimpleStruct:
	case SimpleContainer:
	case FullArray:
	case SimpleIdentity:
	case QObject:
		return tmp;
	case OpaqueStruct:
		return "LStrHandle";
	default:
		qWarning() << "WARNING: TypeConv::bridgeType(): Unsupported type:" << qtType;
		return tmp;
		// TODO: Decide on a good default to return
	}
}

/// This is a lossy (irreversible) conversion.
QString
TypeConv::dllType(const QString& qtType)
{
	QString tmp = typeBase(qtType);
	switch (category(tmp))
	{
	case Void:
	case Boolean:
	case SimpleStruct:
		return tmp;
	case Numeric:
	case SimpleContainer: return _bridge2dll[tmp].toObject()["dllType"].toString();
	case Enum:
		return "int32"; // ASSUMPTION: All enums fit in 32-bit integers
	case FullArray:
	{
		int start = qtType.indexOf('<') + 1;
		int end = qtType.lastIndexOf('>');

		QString inner = qtType.mid(start, end-start);
		QString type = _bridge2dll[tmp].toObject()["dllType"].toString();
		return type.replace("%DLL_TYPE_INNER%", dllType(inner));
	}
	case SimpleIdentity:
	case QObject:
		return "quintptr";
	case OpaqueStruct:
		return "LStrHandle";
	default:
		qWarning() << "WARNING: TypeConv::dllType(): Unsupported type:" << qtType;
		return tmp;
		// TODO: Decide on a good default to return
	}
}

// TODO: Use instanceType() in more generation code
QString
TypeConv::instanceType_bridge(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case SimpleIdentity:
	case QObject:
		// TODO: Clean this up
		if (tmp.contains('*'))
			return tmp;
		return tmp+'*';

	case OpaqueStruct:
		return "LStrHandle";

	default:
		qWarning() << "WARNING: instanceType_bridge(): This type cannot have methods:" << qtType;
		return tmp;
	}
}

QString
TypeConv::instanceType_dll(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case SimpleIdentity:
	case QObject:
		return "quintptr";
	case OpaqueStruct:
		return "LStrHandle";
	default:
		qWarning() << "WARNING: instanceType_dll(): This type cannot have methods:" << qtType;
		return tmp;
	}
}

QString
TypeConv::convCode_bridge2Dll(const QString& qtType)
{
	QString tmp = typeBase(qtType);
	switch (category(tmp))
	{
	case Boolean:
	case Numeric:
	case SimpleStruct:
	case OpaqueStruct:
		return "_bridgeValue_";
	case SimpleContainer: return _bridge2dll[qtType].toObject()["bridge2dll"].toString();
	case FullArray:
	{
		int start = qtType.indexOf('<') + 1;
		int end = qtType.lastIndexOf('>');

		QString inner = qtType.mid(start, end-start);
		QString type = _bridge2dll[tmp].toObject()["bridge2dll"].toString();

		return type.replace("%DLL_TYPE_INNER%", dllType(inner));
	}
	case Enum:
	case SimpleIdentity:
	case QObject:
		return "(_dllType_)_bridgeValue_";
	default:
		qWarning() << "WARNING: TypeConv::convCode_bridge2Dll(): Don't know how to convert" << qtType;
		return QString();
	}
}

QString
TypeConv::convCode_dll2Bridge(const QString& qtType)
{
	QString tmp = typeBase(qtType);
	switch (category(tmp))
	{
	case Boolean:
	case SimpleStruct:
		return "*_dllValue_";
	case Numeric:
	case OpaqueStruct:
		return "_dllValue_";
	case SimpleContainer:
	case FullArray:
		return _bridge2dll[tmp].toObject()["dll2bridge"].toString().replace("%QT_TYPE_INNER%", innerType(qtType));
	case Enum:
	case SimpleIdentity:
	case QObject:
		return "(_qtType_)_dllValue_";
	default:
		qWarning() << "WARNING: TypeConv::convCode_dll2Bridge(): Don't know how to convert" << qtType;
		return QString();
	}
}

QString
TypeConv::convCode_qt2Bridge(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Void:
	case Boolean:
	case Numeric:
	case Enum:
	case SimpleStruct:
	case SimpleIdentity:
	case QObject:
	case SimpleContainer:
	case FullArray:
		return "_qtValue_";
	case OpaqueStruct:
		return "serialize(_qtValue_)";
	default:
		qWarning() << "WARNING: TypeConv::convCode_qt2Bridge(): Don't know how to convert" << qtType;
		return "";
	}
}

QString
TypeConv::convCode_bridge2Qt(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Boolean:
	case Numeric:
	case Enum:
	case SimpleStruct:
	case SimpleIdentity:
	case QObject:
	case SimpleContainer:
	case FullArray:
		return "_bridgeValue_";
	case OpaqueStruct:
		return "deserialize<_qtType_>(copyFromLStr(_bridgeValue_))";
	default:
		qWarning() << "WARNING: TypeConv::convCode_bridge2Qt(): Don't know how to convert" << qtType;
		return "";
	}
}
