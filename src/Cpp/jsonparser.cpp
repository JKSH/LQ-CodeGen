#include "jsonparser.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

/*

#include <QStringList>
#include <QFile>
#include "class.h"

// MSVC 2013 can't handle this...
static const QStringList
classKeys
{
	"name",
	"base",
	"methods",
	"properties"
};

static QSharedPointer<Methodd>
parseMethod(const QString& prototype)
{
	qDebug() << prototype;

	QString preParamStr = prototype.left(prototype.indexOf('('));
	QString paramStr = prototype.mid(preParamStr.length()+1);
	paramStr = paramStr.left(paramStr.lastIndexOf(')'));

	// TODO: Process post-param items
//	QString postParamStr = prototype.mid( preParamStr.length()+1 + paramStr.length()+1 );

	if (preParamStr.length() == 0)
	{
		qWarning() << "Malformed method:" << prototype;
		return QSharedPointer<Methodd>();
	}

	QStringList preParams = preParamStr.split(' ', QString::SkipEmptyParts);
	QStringList params = paramStr.split(',', QString::SkipEmptyParts);

	bool isConstructor = (preParams.size()==1);
	// TODO: Add "isConstructor" to Methodd

	// TODO: Handle more items in preParams (e.g. "static", "Q_INVOKABLE", etc.)
	QString methodName = preParams.last();
	QString methodType = isConstructor ? "void" : preParams.first();

	QSharedPointer<Methodd> method(new Methodd(methodName, methodType));

	for (QString raw : params)
	{
		if (raw.contains('='))
		{
			// TODO: Handle default parameters, strip from string
			// ASSUMPTION: Default parameters won't contain commas
		}

		raw = raw.trimmed();

		QString paramType = raw.left(raw.lastIndexOf(' '));
		QString paramName = raw.mid(paramType.length()+1);

		qDebug() << "\tParam type:" << paramType;
		qDebug() << "\tParam name:" << paramName;

		method->addParam({paramName, paramType});
	}

	return method;
}

// TODO: Just return the root?
QList<Class*>
JsonParser::parseData(const QString& filename)
{
	_classList.clear();
	_tmpClassMap.clear();

	QFile file(filename);
	if (!file.open(QFile::ReadOnly|QFile::Text))
	{
		qDebug() << "Unable to open" << filename;
		return _classList;
	}

	auto doc = QJsonDocument::fromJson(file.readAll());
	file.close();

	for(const QJsonValue& val : doc.array())
	{
		auto obj = val.toObject();

		// Validation: Pass as a class if it has a name
		if (obj["name"].isString())
		{
			QString name = obj["name"].toString();

			// Check for invalid keys
			auto keys = obj.keys();
			for (const QString& validItem : classKeys)
				keys.removeOne(validItem);
			if (!keys.empty())
				qWarning() << "Unknown key(s) in" << name << ":" << keys;

			Class* c = new Class(name);

			for(const QJsonValue& val : obj["methods"].toArray())
				c->addMethod(parseMethod(val.toString()));

			// TODO: Add properties

			_tmpClassMap.insertMulti(obj["base"].toString(), c);
		}
		else
			qWarning() << "Not a class:" << val;
	}

	Class* rootClass = new Class("");


	// ASSUMPTION: There will always be at least 1 base class
	prepareBase(rootClass);

	rootClass->print();

	if (!_tmpClassMap.isEmpty())
	{
		// TODO: Remove duplicates before reporting
		qWarning() << "Cannot find the following base class(es):";
		qWarning() << _tmpClassMap.keys();
	}

	return _classList;
}

void
JsonParser::prepareBase(Class* base)
{
	// TODO: Reverse list? QMultiMap returns items in reverse-insert order
	QString baseName = base ? base->name() : "";
	for (Class* c : _tmpClassMap.values(baseName))
	{
		c->setBase(base);
		prepareBase(c);
	}

	// Move object from the map to the list
	_classList << base;
	_tmpClassMap.remove(baseName);
}
*/
// TODO: Rename to "mapArray" or similar
QJsonObject
JsonParser::arrayToObject(const QJsonArray& objArray, const QString& key)
{
	QJsonObject map;
	for (const QJsonValue& val : objArray)
	{
		QString objName = val.toObject()[key].toString();
		map[objName] = val;
	}

	return map;
}
