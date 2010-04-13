#pragma once

#include <windows.h>
#include "API/Win32/File.h"
#include "ImageShackBase.h"

/**
 * ByteStream.
 * Used by MultipartFormDataRequest.
 *
 * @author Alexander Kozlov
 */
class IMAGESHACK_API ByteStream
{
public:
	ByteStream(int capacity = 0, int grow_by = 0);
	ByteStream(const ByteStream&);

	LPBYTE GetData()	const { return m_data;  }
	DWORD  GetLength()	const { return m_count; }
	void   SetLength(DWORD dwLength);
	void   Empty();

	ByteStream &Write(const void* lpData, DWORD dwCount);
    ByteStream &FromFile(API::Win32::File &file);
	ByteStream &FromFile(API::Win32::File &file, ULONGLONG offset, DWORD dwCount);

	void SetCapacity(DWORD dwNewSize);

	ByteStream &operator=(const ByteStream&);

	~ByteStream();

private:
	LPBYTE m_data;
	DWORD  m_count;
	DWORD  m_size;
	DWORD  m_growBy;
};

ByteStream &operator << (ByteStream &stream, int value);
ByteStream &operator << (ByteStream &stream, DWORD value);
ByteStream &operator << (ByteStream &stream, unsigned __int64 value);
ByteStream &operator << (ByteStream &stream, const char *lpszValue);
ByteStream &operator << (ByteStream &stream, char value);
