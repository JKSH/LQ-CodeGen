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
	for (const QJsonValue& obj : conversions)
	{
		QString objName = obj.toObject()["name"].toString();
		_bridge2dll[objName] = obj;
		_categories[objName] = category;

		// TODO: Protect against non-pointer parameters?
		// Need to support both base (for constructors) and pointer names (for params)
		if (category == Identity)
		{
			_bridge2dll[objName+'*'] = obj;
			_categories[objName+'*'] = category;
		}
	}
}

TypeConv::Category
TypeConv::category(const QString& qtType)
{
	return _categories.value(qtType, Invalid);
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
	case SimpleStruct:
	case Container:
	case Identity:
		return tmp;
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
	// TODO: Study normalization
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Void:
	case Boolean:
	case SimpleStruct:
		return tmp;
	case Numeric:
	case Container: return _bridge2dll[tmp].toObject()["dllType"].toString();
	case Identity: return "quint32"; // TODO: See if quintptr is any good
	default:
		qWarning() << "WARNING: Unsupported type:" << qtType;
		return tmp;
		// TODO: Decide on a good default to return
	}
}

QString
TypeConv::convCode_bridge2Dll(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Boolean:
	case Numeric:
	case SimpleStruct:
		return "_bridgeValue_";
	case Container: return _bridge2dll[qtType].toObject()["bridge2dll"].toString();
	case Identity: return "(_dllType_)_bridgeValue_";
	default:
		qWarning() << "WARNING: Don't know how to convert from DLL:" << qtType;
		return QString();
	}
}

QString
TypeConv::convCode_dll2Bridge(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Boolean:
	case SimpleStruct:
		return "*_dllValue_";
	case Numeric: return "_dllValue_";
	case Container: return _bridge2dll[qtType].toObject()["dll2bridge"].toString();
	case Identity: return "(_qtType_)_dllValue_";
	default:
		qWarning() << "WARNING: Don't know how to convert to DLL:" << qtType;
		return QString();
	}
}

QString
TypeConv::convCode_qt2Bridge(const QString& qtType)
{
	Q_UNUSED(qtType);
	// TODO: Support cases where bridgeType != qtType
	return "_bridgeValue_";
}

QString
TypeConv::convCode_bridge2Qt(const QString& qtType)
{
	Q_UNUSED(qtType);
	// TODO: Support cases where bridgeType != qtType
	return "_qtValue_";
}
