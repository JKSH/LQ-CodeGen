#ifndef CLASSWRITER_H
#define CLASSWRITER_H

#include <QFile>

class Method;
class QJsonObject;

class ClassWriter
{
public:
	ClassWriter();

	bool startWriting();
	void stopWriting();

	void writeClass(const QJsonObject& classObj);

private:
	static QString funcCallBody_inBridge(const Method& method);

	QFile dllH;
	QFile dllC;
	QFile bridgeH;

	QString _footer_dllH;
	QString _footer_dllC;
	QString _footer_bridgeH;

	QString currentClass;
};

#endif // CLASSWRITER_H
