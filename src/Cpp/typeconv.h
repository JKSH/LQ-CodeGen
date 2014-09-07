#ifndef TYPECONV_H
#define TYPECONV_H

#include <QString>
class QJsonArray;

class TypeConv
{
public:
	enum Category {
		Invalid,
		Void,
		Boolean,
		Numeric,
		SimpleStruct,
		OpaqueStruct,
		Identity,
		Container
	};

	static void init(const QJsonArray& conversions, Category category);

	static QString dllType(const QString& qtType);
	static QString convCode_toDll(const QString& qtType);
	static QString convCode_fromDll(const QString& qtType);
};

#endif // TYPECONV_H
