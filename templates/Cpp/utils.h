#ifndef UTILS_H
#define UTILS_H

#include <QDataStream>
#include <QStringList>
#include "extcode.h"

LStrPtr copyIntoLStr(LStrHandle lStr, const QByteArray& bytes);
QByteArray copyFromLStr(LStrHandle lStr);

// TODO: Move the following to types.h or something

// Structs declared between lv_prolog.h and lv_epilog.h get aligned according to LabVIEW's expectations
#include "lv_prolog.h"
template <typename T>
struct LVArray
{
	static void resize(LVArray<T>** handle, qint32 count)
	{
		int newSize = sizeof(qint32) + count*sizeof(T);

		// ASSUMPTION: This call will only fail if the program is already doomed anyway.
		DSSetHandleSize(handle, newSize);
		(*handle)->dimSize = count;
	}
	static void fromQVector(LVArray<T>** destHandle, const QVector<T>& vector)
	{
		resize(destHandle, vector.size());
		std::copy(vector.constBegin(), vector.constEnd(), (*destHandle)->elt);
	}
	static void fromQList(LVArray<T>** destHandle, const QList<T>& list)
	{
		resize(destHandle, list.size());
		std::copy(list.constBegin(), list.constEnd(), (*destHandle)->elt);
	}
	QVector<T> toVector() const
	{
		QVector<T> vector(dimSize);
		std::copy(elt, elt+dimSize, vector.data());
		return vector;
	}
	QList<T> toList() const
	{
		QList<T> list;
		list.reserve(dimSize);
		for (int i = 0; i < dimSize; ++i)
			list << elt[i];
		return list;
	}

	// WARNING: Check padding requirements for each platform
	qint32 dimSize;
	T elt[1];
};

// Specializations for arrays of complex datatypes
template <>
struct LVArray<LStrHandle>
{
	static void resize(LVArray<LStrHandle>** handle, qint32 count)
	{
		// Just call the primary implementation
		LVArray<void*>::resize((LVArray<void*>**)handle, count);
	}
	static void fromList(LVArray<LStrHandle>** destHandle, const QStringList& list)
	{
		resize(destHandle, list.size());
		for (int i = 0; i < list.size(); ++i)
		{
			const int lStrHeaderSize = 4;
			QByteArray bytes = list[i].toUtf8();
			(*destHandle)->elt[i] = (LStrHandle)DSNewHandle(bytes.length() + lStrHeaderSize);
			copyIntoLStr((*destHandle)->elt[i], bytes);
		}
	}
	QStringList toList() const
	{
		QStringList list;
		list.reserve(dimSize);
		for (int i = 0; i < dimSize; ++i)
			list << copyFromLStr(elt[i]); // TODO: Avoid intermediate QByteArray
		return list;
	}
	// No QVector storage

	qint32 dimSize;
	LStrHandle elt[1];
};
#include "lv_epilog.h"

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
