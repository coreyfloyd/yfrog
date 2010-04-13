#include "stdafx.h"
#include "bytestream.h"
using namespace API::Win32;

ByteStream::ByteStream(int capacity, int grow_by)
	: m_data(NULL)
	, m_size(0)
	, m_count(0)
{
	m_growBy = grow_by > 0 ? grow_by : 4096;
	if (capacity) SetCapacity(capacity);
}

ByteStream::ByteStream(const ByteStream& stream)
	: m_data(NULL)
	, m_size(0)
	, m_count(0)
	, m_growBy(stream.m_growBy)
{
	if (!stream.GetLength())
		return;

	m_data = new BYTE[m_size = m_count = stream.GetLength()];
	CopyMemory(m_data, stream.GetData(), stream.GetLength());
}

ByteStream::~ByteStream()
{
	if (m_data) delete [] m_data;
}

ByteStream &ByteStream::operator=(const ByteStream&stream)
{
	if (m_data) delete [] m_data, m_data = NULL;

	m_data = new BYTE[m_size = m_count = stream.GetLength()];
	CopyMemory(m_data, stream.GetData(), stream.GetLength());

	return *this;
}

ByteStream &ByteStream::Write(const void* lpData, DWORD dwCount)
{
	if (!lpData || !dwCount)
		return *this;

	if (m_size < m_count + dwCount)
		SetCapacity(m_count + dwCount);

	CopyMemory(m_data + m_count, lpData, dwCount);
	m_count += dwCount;

	return *this;
}

ByteStream &ByteStream::FromFile(File &file)
{
	return FromFile(file, 0, file.GetLength());
}

ByteStream &ByteStream::FromFile(File &file, ULONGLONG offset, DWORD dwCount)
{
	LARGE_INTEGER li;
	li.QuadPart = (LONGLONG)offset;
	if (!file.Seek(li))
		return *this;

	if (m_size < m_count + dwCount)
		SetCapacity(m_count + dwCount);

	if (!file.Read(m_data + m_count, dwCount))
		return *this;

	m_count += file.GetNumberOfBytesRead();

	return *this;
}

void ByteStream::Empty()
{
	SetLength(0);
}

void ByteStream::SetLength(DWORD dwLength)
{
	SetCapacity(dwLength);
	m_count = dwLength;
}

void ByteStream::SetCapacity(DWORD dwNewSize)
{
	if (m_size >= dwNewSize)
		return;

	BYTE *new_data = new BYTE[m_size = dwNewSize + m_growBy];

	CopyMemory(new_data, m_data, m_count);
	if (m_data) delete [] m_data, m_data = NULL;

	m_data = new_data;
}

ByteStream &operator << (ByteStream &stream, const char *lpszValue)
{
	return (lpszValue && *lpszValue) ? stream.Write(lpszValue, (DWORD)strlen(lpszValue)) : stream;
}

ByteStream &operator << (ByteStream &stream, char value)
{
	return stream.Write(&value, 1);
}

ByteStream &operator << (ByteStream &stream, int value)
{
	char buf[64];
	sprintf_s(buf, sizeof(buf), "%i", value);
	return stream.Write(buf, (DWORD)strlen(buf));
}

ByteStream &operator << (ByteStream &stream, DWORD value)
{
	char buf[64];
	sprintf_s(buf, sizeof(buf), "%I32u", value);
	return stream.Write(buf, (DWORD)strlen(buf));
}

ByteStream &operator << (ByteStream &stream, unsigned __int64 value)
{
	char buf[255];
	sprintf_s(buf, sizeof(buf), "%I64u", value);
	return stream.Write(buf, (DWORD)strlen(buf));
}
