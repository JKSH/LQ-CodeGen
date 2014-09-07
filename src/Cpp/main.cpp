#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "classwriter.h"
#include "typeconv.h"

static QJsonDocument
parseJsonFile(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QFile::ReadOnly|QFile::Text))
	{
		qDebug() << "Unable to open" << file.fileName();
		return QJsonDocument();
	}
	auto doc = QJsonDocument::fromJson(file.readAll());
	file.close();
	return doc;
}

int main(int, char **)
{
	// Read data
	QJsonDocument numerics = parseJsonFile("../../data/numerics.json");
	if (numerics.isNull())
		return -1;

	QJsonDocument conversions = parseJsonFile("../../data/types.json");
	if (conversions.isNull())
		return -1;

	QJsonDocument identities = parseJsonFile("../../data/identities.json");
	if (identities.isNull())
		return -1;


	// Process data
	// TypeConv must be initialized before other processing
	TypeConv::init(numerics.array(), TypeConv::Numeric);
	TypeConv::init(conversions.array(), TypeConv::Invalid); // TODO: Categorize properly

	ClassWriter c;
	c.startWriting();
	for(const QJsonValue& val : identities.array())
		c.writeClass(val.toObject());
	c.stopWriting();

	return 0;
}
