/*\
 * Copyright (c) 2018 Sze Howe Koh
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
\*/

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
	static QString funcCallBody_inDll(const Method& method);
	static QString funcCallBody_inLambda(const Method& method);

	QFile dllH;
	QFile dllC;

	QString _footer_dllH;
	QString _footer_dllC;
};

#endif // CLASSWRITER_H
