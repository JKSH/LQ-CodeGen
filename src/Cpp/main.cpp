#include <QDebug>
#include <QJsonDocument>
#include "classwriter.h"

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
	QJsonDocument conversions = parseJsonFile("../../data/types.json");
	if (conversions.isNull())
		return -1;

	QJsonDocument doc = parseJsonFile("../../data/objects.json");
	if (doc.isNull())
		return -1;


	// Process data
	// TypeConv must be initialized before other processing
	TypeConv::init(conversions.array());

	ClassWriter c;
	c.startWriting();
	for(const QJsonValue& val : doc.array())
		c.writeClass(val.toObject());
	c.stopWriting();

	return 0;
}
