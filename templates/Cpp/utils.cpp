#include "utils.h"

LStrPtr
copyIntoLStr(LStrHandle lStr, const QByteArray& bytes)
{
	const int lStrHeaderSize = 4;

	// TODO: Report MgErr value
	MgErr outcome = DSSetHandleSize(lStr, bytes.length() + lStrHeaderSize);
	if (outcome == noErr)
	{
		(*lStr)->cnt = bytes.length();
		std::copy(bytes.constBegin(), bytes.constEnd(), (*lStr)->str);
		return *lStr;
	}

	// TODO: Check if this is a safe design
	return nullptr;
}

// TODO: Make QByteArray converter use this too?
QByteArray
copyFromLStr(LStrHandle lStr)
{
	return QByteArray( (char*)(*lStr)->str, LStrLen(*lStr) );
}
