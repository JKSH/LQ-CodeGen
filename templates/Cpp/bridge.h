#ifndef BRIDGE_H
#define BRIDGE_H

#include <QtWidgets>
#include <QtSvg>
#include "utils.h"

#include <qwt_thermo.h>
#include <qwt_slider.h>

class Bridge : public QObject
{
	Q_OBJECT

public:
	explicit Bridge(QObject* parent = nullptr) : QObject(parent) {}

	// ASSUMPTION: These will always be registered before any connections are made.
	// No need to lock for thread-safety.
	void registerEventRef_void(LVUserEventRef* ref)   {ref_void   = *ref;}
	void registerEventRef_bool(LVUserEventRef* ref)   {ref_bool   = *ref;}
	void registerEventRef_i32(LVUserEventRef* ref)    {ref_i32    = *ref;}
	void registerEventRef_string(LVUserEventRef* ref) {ref_string = *ref;}

public slots:
	void postLVEvent_void() {
		SignalPacket<bool> packet{(quint64)sender(), senderSignalIndex(), false};
		PostLVUserEvent(ref_void, &packet);
	}
	void postLVEvent_bool(bool value) {
		SignalPacket<bool> packet{(quint64)sender(), senderSignalIndex(), value};
		PostLVUserEvent(ref_bool, &packet);
	}
	void postLVEvent_i32(int value) {
		SignalPacket<qint32> packet{(quint64)sender(), senderSignalIndex(), value};
		PostLVUserEvent(ref_i32, &packet);
	}
	void postLVEvent_string(const QString& value) {
		const int lStrHeaderSize = 4;
		QByteArray bytes = value.toUtf8();
		LStrHandle lStr = (LStrHandle)DSNewHandle(bytes.length() + lStrHeaderSize);
		if (lStr)
		{
			copyIntoLStr(lStr, bytes);
			SignalPacket<LStrHandle> packet{(quint64)sender(), senderSignalIndex(), lStr};
			PostLVUserEvent(ref_string, &packet);
			DSDisposeHandle(lStr);
		}
		// else we've probably run out of memory
	}

//[TEMPLATE]

private:
	/*
		Use 64 bits for 'sender' because LabVIEW's CLFN takes 64 bits
		for all pointer-sized integers, even in 32-bit builds.

		Use 64 bits for 'signalIndex' to avoid alignment problems in
		64-bit builds, if T is pointer-sized.
	*/
	template <typename T>
	struct SignalPacket {
		quint64 sender;
		qint64 signalIndex;
		T value;
	};

	LVUserEventRef ref_void;
	LVUserEventRef ref_bool;
	LVUserEventRef ref_i32;
	LVUserEventRef ref_string;
};

#endif
