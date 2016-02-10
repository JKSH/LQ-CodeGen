#ifndef UTILS_H
#define UTILS_H

#include <QDataStream>
#include "extcode.h"

void copyIntoLStr(LStrHandle lStr, const QByteArray& bytes);
QByteArray copyFromLStr(LStrHandle lStr);
LStrHandle newLStr(const QByteArray& bytes);
inline LStrHandle newLStr(const QString& string) {return newLStr(string.toUtf8());}

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

	template <typename U>
	static void fromQVector(LVArray<T>** destHandle, const QVector<U>& vector)
	{
		resize(destHandle, vector.size());
		std::copy(vector.constBegin(), vector.constEnd(), (*destHandle)->elt);
	}

	template <typename U>
	static void fromQList(LVArray<T>** destHandle, const QList<U>& list)
	{
		resize(destHandle, list.size());
		std::copy(list.constBegin(), list.constEnd(), (*destHandle)->elt);
	}

	template <typename U>
	QVector<U> toQVector() const
	{
		QVector<U> vector(dimSize);
		std::copy(elt, elt+dimSize, vector.data());
		return vector;
	}

	template <typename U>
	QList<U> toQList() const
	{
		QList<U> list;
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

	template <typename U>
	static void fromQList(LVArray<LStrHandle>** destHandle, const QList<U>& list)
	{
		resize(destHandle, list.size());
		for (int i = 0; i < list.size(); ++i)
			(*destHandle)->elt[i] = newLStr(list[i]);
	}

	template <typename U>
	QList<U> toQList() const
	{
		QList<U> list;
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
