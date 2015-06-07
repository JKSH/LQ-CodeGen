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

/*!
	Goes through the array of \a entities (which can be classes
	or namespaces) and extracts any enums within.

	Returns an array of enums with fully-qualified names.
*/
static QJsonArray
extractEnums(const QJsonArray& entities)
{
	QJsonArray enums;

	for (const QJsonValue& entity : entities)
	{
		QJsonObject entityObj = entity.toObject();
		QString entityName = entityObj["name"].toString();
		for (const QJsonValue& entry : entityObj["enums"].toArray())
		{
			// Fully qualify the name
			auto finalEnum = entry.toObject();
			QString baseName = finalEnum["name"].toString();
			finalEnum["name"] = entityName + "::" + baseName;
			enums << finalEnum;
		}
	}

	return enums;
}

int main(int, char **)
{
	// Read data
	QJsonDocument namespaces = parseJsonFile("../../data/namespaces.json");
	if (namespaces.isNull())
		return -1;

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

	QJsonDocument fullArrays = parseJsonFile("../../data/fullarrays.json");
	if (fullArrays.isNull())
		return -1;

	QJsonDocument simpleIdentities = parseJsonFile("../../data/simpleidentities.json");
	if (simpleIdentities.isNull())
		return -1;

	QJsonDocument qobjects = parseJsonFile("../../data/qobjects.json");
	if (qobjects.isNull())
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
	TypeConv::init(extractEnums(simpleIdentities.array()), TypeConv::Enum);
	TypeConv::init(extractEnums(qobjects.array()), TypeConv::Enum);
	TypeConv::init(extractEnums(namespaces.array()), TypeConv::Enum);
	TypeConv::init(extractEnums(opaqueClasses.array()), TypeConv::Enum);
	TypeConv::init(simpleStructs.array(), TypeConv::SimpleStruct);
	TypeConv::init(opaqueClasses.array(), TypeConv::OpaqueStruct);
	TypeConv::init(simpleContainers.array(), TypeConv::SimpleContainer);
	TypeConv::init(fullArrays.array(), TypeConv::FullArray);
	TypeConv::init(simpleIdentities.array(), TypeConv::QObject);
	TypeConv::init(qobjects.array(), TypeConv::QObject);

	ClassWriter c;
	c.startWriting();
	for(const QJsonValue& val : simpleIdentities.array())
		c.writeClass(val.toObject());
	for(const QJsonValue& val : qobjects.array())
		c.writeClass(val.toObject());
	for(const QJsonValue& val : opaqueClasses.array())
		c.writeClass(val.toObject());
	c.stopWriting();

	return 0;
}
