#include "class.h"

#include <QDebug>
#include <QJsonDocument>
#include "classwriter.h"

int main(int, char **)
{
/*
	qDebug() << QMetaObject::normalizedType("const QString &");
	qDebug() << QMetaObject::normalizedType("const QString *");
	qDebug() << QMetaObject::normalizedType("const QString * const");


	qDebug() << QMetaObject::normalizedType("char");
	qDebug() << QMetaObject::normalizedType("char&");
	qDebug() << QMetaObject::normalizedType("const char");
	qDebug() << QMetaObject::normalizedType("const char&");
	qDebug() << QMetaObject::normalizedType("const char *");
	qDebug() << QMetaObject::normalizedType("const char * const");
*/
	QFile file("../data/objects.json");
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
