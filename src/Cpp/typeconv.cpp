#include "typeconv.h"

#include <QMetaObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>

#include <QDebug>

QJsonObject _qt2dll;
QMap<QString, TypeConv::Category> _categories;

void
TypeConv::init(const QJsonArray& conversions, Category category)
{
	// TODO: More robust checks
	for (const QJsonValue& obj : conversions)
	{
		QString objName = obj.toObject()["name"].toString();
		if (category == Identity)
			objName += '*';
		_qt2dll[objName] = obj;
		_categories[objName] = category;
	}
}

TypeConv::Category
TypeConv::category(const QString& qtType)
{
	return _categories.value(qtType, Invalid);
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
	case Boolean: return tmp;
	case Numeric:
	case Container: return _qt2dll[tmp].toObject()["dllType"].toString();
	case Identity: return "quint32"; // TODO: See if quintptr is any good
	default:
		qWarning() << "WARNING: Unsupported type:" << qtType;
		return tmp;
		// TODO: Decide on a good default to return
	}
}

QString
TypeConv::convCode_toDll(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Boolean:
	case Numeric: return "_qtValue_";
	case Container: return _qt2dll[qtType].toObject()["qt2dll"].toString();
	case Identity: return "(_dllType_)_qtValue_";
	default:
		qWarning() << "WARNING: Don't know how to convert from DLL:" << qtType;
		return QString();
	}
}
QString
TypeConv::convCode_fromDll(const QString& qtType)
{
	QString tmp = QMetaObject::normalizedType(qtType.toUtf8());
	switch (category(tmp))
	{
	case Boolean: return "*_dllValue_";
	case Numeric: return "_dllValue_";
	case Container: return _qt2dll[qtType].toObject()["dll2qt"].toString();
	case Identity: return "(_qtType_)_dllValue_";
	default:
		qWarning() << "WARNING: Don't know how to convert to DLL:" << qtType;
		return QString();
	}
}
