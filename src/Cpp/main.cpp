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

	QJsonDocument simpleStructs = parseJsonFile("../../data/simplestructs.json");
	if (simpleStructs.isNull())
		return -1;

	QJsonDocument opaqueClasses = parseJsonFile("../../data/opaqueclasses.json");
	if (opaqueClasses.isNull())
		return -1;

	QJsonDocument simpleContainers = parseJsonFile("../../data/simplecontainers.json");
	if (simpleContainers.isNull())
		return -1;

	QJsonDocument identities = parseJsonFile("../../data/identities.json");
	if (identities.isNull())
		return -1;

	QJsonObject voidObj;
	voidObj["name"] = "void";

	QJsonObject boolObj;
	boolObj["name"] = "bool";

	// Process data
	// TypeConv must be initialized before other processing
	TypeConv::init(QJsonArray()<<voidObj, TypeConv::Void);
	TypeConv::init(QJsonArray()<<boolObj, TypeConv::Boolean);
	TypeConv::init(numerics.array(), TypeConv::Numeric);
	TypeConv::init(simpleStructs.array(), TypeConv::SimpleStruct);
	TypeConv::init(opaqueClasses.array(), TypeConv::OpaqueStruct);
	TypeConv::init(simpleContainers.array(), TypeConv::SimpleContainer);
	TypeConv::init(identities.array(), TypeConv::Identity);

	ClassWriter c;
	c.startWriting();
	for(const QJsonValue& val : identities.array())
		c.writeClass(val.toObject());
	for(const QJsonValue& val : opaqueClasses.array())
		c.writeClass(val.toObject());
	c.stopWriting();

	return 0;
}
