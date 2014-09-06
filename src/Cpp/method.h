#ifndef METHOD_H
#define METHOD_H

#include "typeconv.h"
#include <QJsonObject>

class Param;

class Method
{
public:
	Method(const QString& className, const QJsonObject& methodObj);

	// Method info
	bool isValid() const;
	bool isConstructor() const;
	QString name() const;
	QString qualifiedName(const QString& separator) const;

	// Return type names
	QString returnType_bridge() const;
	QString returnType_dll() const;

	// Param lists
	QList<Param> paramList_raw() const;
	QList<Param> paramList_bridge() const;
	QList<Param> paramList_dll() const;

	// Param list codification
	static QString paramsToCode_funcCall(const QList<Param>& params);
	static QString paramsToCode_prototype(const QList<Param>& params);

private:
	QString _className;
	QJsonObject _data;
};

#endif // METHOD_H
