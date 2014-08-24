#include "jsonparser.h"

#include <QJsonArray>
#include <QJsonObject>

// TODO: Rename to "mapArray" or similar
QJsonObject
JsonParser::arrayToObject(const QJsonArray& objArray, const QString& key)
{
	QJsonObject map;
	for (const QJsonValue& val : objArray)
	{
		QString objName = val.toObject()[key].toString();
		map[objName] = val;
	}

	return map;
}
