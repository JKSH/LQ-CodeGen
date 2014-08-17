#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QDebug>
//#include <QMultiMap>
#include <QJsonArray>

class Class;

class JsonParser
{
/*
	void prepareBase(Class* base);

	QList<Class*> _classList;
	QMultiMap<QString, Class*> _tmpClassMap;
*/

public:
//	QList<Class*> parseData(const QString& filename);

	static QJsonObject arrayToObject(const QJsonArray& array, const QString& key);
};

#endif // JSONPARSER_H
