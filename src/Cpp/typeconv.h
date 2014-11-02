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

	static Category category(const QString& qtType);
	static QString bridgeType(const QString& qtType);
	static QString dllType(const QString& qtType);
	static QString convCode_bridge2Dll(const QString& qtType);
	static QString convCode_dll2Bridge(const QString& qtType);
	static QString convCode_qt2Bridge(const QString& qtType);
	static QString convCode_bridge2Qt(const QString& qtType);
};

#endif // TYPECONV_H
