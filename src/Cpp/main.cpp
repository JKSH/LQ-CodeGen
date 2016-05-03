/*\
 * Copyright (c) 2016 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

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

// The kernel module must be first in the list.
const QStringList
moduleSpecs
{
	"LQ Core.json",
	"Qt GUI.json",
	"Qt Widgets.json",
	"Qt SVG.json",
	"Qt Windows Extras.json",
	"Qwt.json",
	"LQ Extras.json"
};

int main(int, char **)
{
	// Void and bool types are basic types, required everywhere.
	// Don't initialize them multiple times.
	TypeConv::init({QJsonObject{{"name", "void"}}}, TypeConv::Void);
	TypeConv::init({QJsonObject{{"name", "bool"}}}, TypeConv::Boolean);

	ClassWriter c;
	c.startWriting();

	// Loop across multiple modules, and generate code for each
	// TODO: Support separate DLLs per module. This requires redesigning the Bridge system.
	for (const QString& file : moduleSpecs)
	{
		// Read module specs
		QJsonObject moduleSpecs = parseJsonFile("../../data/"+file).object();
		if (moduleSpecs.isEmpty())
			return -1;
		QJsonObject types = moduleSpecs["typeCategories"].toObject();
		QJsonArray numerics = types["numerics"].toArray();
		QJsonArray simpleStructs = types["simpleStructs"].toArray();
		QJsonArray opaqueStructs = types["opaqueStructs"].toArray();
		QJsonArray simpleIdentities = types["simpleIdentities"].toArray();
		QJsonArray qObjects = types["qObjects"].toArray();

		// Regster types
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

		// Generate code
		for(const QJsonValue& val : simpleIdentities)
			c.writeClass(val.toObject());
		for(const QJsonValue& val : qObjects)
			c.writeClass(val.toObject());
		for(const QJsonValue& val : opaqueStructs)
			c.writeClass(val.toObject());
	}
	c.stopWriting();

	return 0;
}
