#include <QDebug>
#include <QJsonDocument>
#include "classwriter.h"

int main(int, char **)
{
	// Read data
	QFile typeFile("../../data/types.json");
	if (!typeFile.open(QFile::ReadOnly|QFile::Text))
	{
		qDebug() << "Unable to open" << typeFile.fileName();
		return -1;
	}
	auto conversions = QJsonDocument::fromJson(typeFile.readAll());
	typeFile.close();

	QFile qobjFile("../../data/objects.json");
	if (!qobjFile.open(QFile::ReadOnly|QFile::Text))
	{
		qDebug() << "Unable to open" << qobjFile.fileName();
		return -1;
	}
	auto doc = QJsonDocument::fromJson(qobjFile.readAll());
	qobjFile.close();


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
