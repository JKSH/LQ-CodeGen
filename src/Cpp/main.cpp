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
	QJsonObject moduleSpecs = parseJsonFile("../../data/Qt Core GUI Widgets.json").object();
	if (moduleSpecs.isEmpty())
		return -1;
	QJsonObject types = moduleSpecs["typeCategories"].toObject();

	QJsonObject voidObj;
	voidObj["name"] = "void";

	QJsonObject boolObj;
	boolObj["name"] = "bool";

	QJsonArray numerics = types["numerics"].toArray();
	QJsonArray simpleStructs = types["simpleStructs"].toArray();
	QJsonArray opaqueStructs = types["opaqueStructs"].toArray();
	QJsonArray simpleIdentities = types["simpleIdentities"].toArray();
	QJsonArray qObjects = types["qObjects"].toArray();

	// Process data
	// TypeConv must be initialized before other processing
	TypeConv::init(QJsonArray()<<voidObj, TypeConv::Void);
	TypeConv::init(QJsonArray()<<boolObj, TypeConv::Boolean);
	TypeConv::init(numerics, TypeConv::Numeric);
	TypeConv::init(extractEnums(moduleSpecs["namespaces"].toArray()), TypeConv::Enum);
	TypeConv::init(extractEnums(opaqueStructs), TypeConv::Enum);
	TypeConv::init(extractEnums(simpleIdentities), TypeConv::Enum);
	TypeConv::init(extractEnums(qObjects), TypeConv::Enum);
	TypeConv::init(simpleStructs, TypeConv::SimpleStruct);
	TypeConv::init(opaqueStructs, TypeConv::OpaqueStruct);
	TypeConv::init(types["simpleContainers"].toArray(), TypeConv::SimpleContainer);
	TypeConv::init(types["fullArrays"].toArray(), TypeConv::FullArray);
	TypeConv::init(simpleIdentities, TypeConv::SimpleIdentity);
	TypeConv::init(qObjects, TypeConv::QObject);

	ClassWriter c;
	c.startWriting();
	for(const QJsonValue& val : simpleIdentities)
		c.writeClass(val.toObject());
	for(const QJsonValue& val : qObjects)
		c.writeClass(val.toObject());
	for(const QJsonValue& val : opaqueStructs)
		c.writeClass(val.toObject());
	c.stopWriting();

	return 0;
}
