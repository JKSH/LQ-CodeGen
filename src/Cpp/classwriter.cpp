#include "classwriter.h"

QJsonObject TypeConv::_qt2dll;

ClassWriter::ClassWriter() :
	dllH("labqt.h"),
	dllC("labqt.cpp"),
	bridgeH("bridge.h")
{
	// TODO: Move this upstream
//	qDebug("== Start testing TypeConv");
	QFile typeFile("../../data/types.json");
	typeFile.open(QFile::ReadOnly|QFile::Text);

	auto conversions = QJsonDocument::fromJson(typeFile.readAll());
	TypeConv::init(conversions.array());

	typeFile.close();
/*
	qDebug() << TypeConv::dllType("void");
	qDebug() << TypeConv::dllType("bool");
	qDebug() << TypeConv::dllType("QString");
	qDebug() << TypeConv::dllType("int");
	qDebug("== Stop testing TypeConv");
*/
}

bool
ClassWriter::startWriting()
{
	bool ok = dllH.open(QFile::WriteOnly|QFile::Text)
			&& dllC.open(QFile::WriteOnly|QFile::Text)
			&& bridgeH.open(QFile::WriteOnly|QFile::Text);

	if (!ok)
	{
		qWarning() << "Couldn't open something!";
		return false;
	}

	// TODO: Warn if templates are malformed

	// TODO: Refactor
	QFile template_dllH("../../templates/Cpp/labqt.h");
	if (template_dllH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllH.readAll()).split("//[TEMPLATE]");
		dllH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllH = bits[1];

		template_dllH.close();
	}
	else
		qWarning() << "Could not find labqt.h template";

	QFile template_dllC("../../templates/Cpp/labqt.cpp");
	if (template_dllC.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_dllC.readAll()).split("//[TEMPLATE]");
		dllC.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_dllC = bits[1];

		template_dllC.close();
	}
	else
		qWarning() << "Could not find labqt.cpp template";

	QFile template_bridgeH("../../templates/Cpp/bridge.h");
	if (template_bridgeH.open(QFile::ReadOnly|QFile::Text))
	{
		QStringList bits = QString(template_bridgeH.readAll()).split("//[TEMPLATE]");
		bridgeH.write(bits[0].toUtf8());

		if (bits.count() > 1)
			_footer_bridgeH = bits[1];

		template_bridgeH.close();
	}
	else
		qWarning() << "Could not find bridge.h template";

	return true;
}

void
ClassWriter::stopWriting()
{
	dllH.write(_footer_dllH.toUtf8());
	dllC.write(_footer_dllC.toUtf8());
	bridgeH.write(_footer_bridgeH.toUtf8());

	dllH.close();
	dllC.close();
	bridgeH.close();
}
