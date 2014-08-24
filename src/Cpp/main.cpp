#include <QDebug>
#include <QJsonDocument>
#include "classwriter.h"

int main(int, char **)
{
	QFile typeFile("../../data/types.json");
	typeFile.open(QFile::ReadOnly|QFile::Text);

	auto conversions = QJsonDocument::fromJson(typeFile.readAll());
	TypeConv::init(conversions.array());

	typeFile.close();

	QFile file("../../data/objects.json");
	if (file.open(QFile::ReadOnly|QFile::Text))
	{
		auto doc = QJsonDocument::fromJson(file.readAll());
		file.close();

		ClassWriter c;
		c.startWriting();
		for(const QJsonValue& val : doc.array())
			c.writeClass(val.toObject());
		c.stopWriting();
	}
	else
		qDebug() << "Unable to open JSON file";

	return 0;
}
