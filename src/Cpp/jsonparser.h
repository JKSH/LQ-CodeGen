#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <QJsonArray>

namespace JsonParser
{
	QJsonObject arrayToObject(const QJsonArray& array, const QString& key);
}

#endif // JSONPARSER_H
