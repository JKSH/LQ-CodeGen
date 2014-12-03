#include "utils.h"

void
copyIntoLStr(LStrHandle lStr, const QByteArray& bytes)
{
	const int lStrHeaderSize = 4;

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
