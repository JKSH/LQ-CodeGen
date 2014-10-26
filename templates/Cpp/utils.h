#ifndef UTILS_H
#define UTILS_H

#include <QDataStream>
#include "extcode.h"

LStrPtr copyIntoLStr(LStrHandle lStr, const QByteArray& bytes);
QByteArray copyFromLStr(LStrHandle lStr);

// ASSUMPTION: T is serializable
// TODO: Investigate if it's worth overloading the functions below to take rvalue references.
//       See http://qt-project.org/forums/viewthread/34454
template <typename T> QByteArray
serialize(const T& object)
{
	QByteArray bytes;
	QDataStream stream(&bytes, QIODevice::WriteOnly);
	stream << object;
	return bytes;
}

template <typename T> T
deserialize(const QByteArray& bytes)
{
	QDataStream stream(bytes);
	T object;
	stream >> object;
	return object;
}

#endif // UTILS_H
