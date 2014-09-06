#ifndef TYPECONV_H
#define TYPECONV_H

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QMap>

#include <QDebug>

class TypeConv
{
public:
	enum Category {
		Boolean,
		Numeric,
		SimpleStruct,
		OpaqueStruct,
		Identity,
		Container
	};

	static void init(const QJsonArray& conversions, Category category)
	{
		// TODO: More robust checks
		for (const QJsonValue& obj : conversions)
		{
			QString objName = obj.toObject()["qtType"].toString();
			_qt2dll[objName] = obj;
			_categories[objName] = category; // TODO: Use category info in ClassWriter
		}
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
private:
	static QMap<QString, Category> _categories;
	static QJsonObject _qt2dll;
};

#endif // TYPECONV_H
