#include "utils.h"

static const int lStrHeaderSize = 4;

void
copyIntoLStr(LStrHandle lStr, const QByteArray& bytes)
{
	// TODO: Report MgErr value
	MgErr outcome = DSSetHandleSize(lStr, bytes.length() + lStrHeaderSize);
	if (outcome == noErr)
	{
		(*lStr)->cnt = bytes.length();
		std::copy(bytes.constBegin(), bytes.constEnd(), (*lStr)->str);
	}
}

// TODO: Make QByteArray converter use this too?
QByteArray
copyFromLStr(LStrHandle lStr)
{
	return QByteArray( (char*)(*lStr)->str, LStrLen(*lStr) );
}

LStrHandle
newLStr(const QByteArray& bytes)
{
	auto lStr = (LStrHandle)DSNewHandle(bytes.length() + lStrHeaderSize);
	copyIntoLStr(lStr, bytes);
	return lStr;
}
