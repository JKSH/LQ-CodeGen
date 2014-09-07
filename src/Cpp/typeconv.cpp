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
		_qt2dll[objName] = obj;
		_categories[objName] = category; // TODO: Use category info in ClassWriter
	}
}

/// This is a lossy (irreversible) conversion.
QString
TypeConv::dllType(const QString& qtType)
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

QString
TypeConv::convCode_toDll(const QString& qtType)
{
	// TODO: Return default if non-existent
	return _qt2dll[qtType].toObject()["qt2dll"].toString();
}
QString
TypeConv::convCode_fromDll(const QString& qtType)
{
	// TODO: Return default if non-existent
	return _qt2dll[qtType].toObject()["dll2qt"].toString();
}
