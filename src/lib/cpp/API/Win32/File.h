// File.h: File class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Handle.h"
#include "FileFind.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// File
//////////////////////////////////////////////////////////////////////

class File : public Handle
{
protected:
	DWORD	m_dwActualNumberOfBytes;
	CString m_strFilePath;

public:
	File():m_dwActualNumberOfBytes(0) {}

	BOOL Open(	LPCTSTR lpFileName,
				DWORD dwDesiredAccess			= GENERIC_READ,
				DWORD dwShareMode				= FILE_SHARE_READ,
				DWORD dwCreationDisposition		= OPEN_EXISTING,
				DWORD dwFlagsAndAttributes		= 0,
				HANDLE hTemplateFile			= NULL,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL
			)
	{
		return Create(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	BOOL Open(	BSTR  bstrFileName,
				DWORD dwDesiredAccess			= GENERIC_READ,
				DWORD dwShareMode				= FILE_SHARE_READ,
				DWORD dwCreationDisposition		= OPEN_EXISTING,
				DWORD dwFlagsAndAttributes		= 0,
				HANDLE hTemplateFile			= NULL,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL
			)
	{
		return Create(bstrFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	BOOL Create(LPCTSTR lpFileName,
				DWORD dwDesiredAccess			= GENERIC_WRITE,
				DWORD dwShareMode				= 0,
				DWORD dwCreationDisposition		= CREATE_ALWAYS,
				DWORD dwFlagsAndAttributes		= 0,
				HANDLE hTemplateFile			= NULL,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL
			)
	{
		return Create(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	BOOL Create(BSTR  bstrFileName,
				DWORD dwDesiredAccess			= GENERIC_WRITE,
				DWORD dwShareMode				= 0,
				DWORD dwCreationDisposition		= CREATE_ALWAYS,
				DWORD dwFlagsAndAttributes		= 0,
				HANDLE hTemplateFile			= NULL,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL
			)
	{
		return Create(bstrFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);
	}

	BOOL Create(LPCTSTR lpFileName,                         // file name
				DWORD dwDesiredAccess,                      // access mode
				DWORD dwShareMode,                          // share mode
				LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
				DWORD dwCreationDisposition,                // how to create
				DWORD dwFlagsAndAttributes,                 // file attributes
				HANDLE hTemplateFile                        // handle to template file
			)
	{
		m_strFilePath = lpFileName;
		HANDLE hFile = ::CreateFile(lpFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);

		Attach(hFile);

		return IsValid();
	}

	BOOL Create(BSTR  bstrFileName,                         // file name
				DWORD dwDesiredAccess,                      // access mode
				DWORD dwShareMode,                          // share mode
				LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
				DWORD dwCreationDisposition,                // how to create
				DWORD dwFlagsAndAttributes,                 // file attributes
				HANDLE hTemplateFile                        // handle to template file
			)
	{
		m_strFilePath = bstrFileName;
		HANDLE hFile = ::CreateFileW(bstrFileName,dwDesiredAccess,dwShareMode,lpSecurityAttributes,dwCreationDisposition,dwFlagsAndAttributes,hTemplateFile);

		Attach(hFile);

		return IsValid();
	}

	BOOL Read(	LPVOID lpBuffer,					// data buffer
				DWORD nNumberOfBytesToRead,			// number of bytes to read
				LPOVERLAPPED lpOverlapped  = NULL	// overlapped buffer
			)
	{
		ATLASSERT(IsValid());

		return ::ReadFile(handle, lpBuffer, nNumberOfBytesToRead, &m_dwActualNumberOfBytes, lpOverlapped);
	}

	BOOL Read(bool &bValue)
	{
		return Read(&bValue, sizeof(bValue));
	}

	BOOL Read(DWORD &dwValue)
	{
		return Read(&dwValue, sizeof(dwValue));
	}

	BOOL Read(CStringA &sValue)
	{
		CStringW wValue;
		BOOL bResult = Read(wValue);
		sValue = wValue;
		return bResult;
	}

	BOOL Read(CStringW &sValue)
	{
		ATLASSERT(IsValid());

		sValue.Empty();

		DWORD dwLength = 0;
		if (!Read(&dwLength, sizeof(dwLength)))
			return FALSE;

		if (!dwLength)
			return FALSE;

		DWORD dwActualNumberOfBytes = m_dwActualNumberOfBytes;
		
		BOOL bReturn = Read(sValue.GetBuffer(dwLength), dwLength*sizeof(WCHAR));
		sValue.ReleaseBuffer(m_dwActualNumberOfBytes/sizeof(WCHAR));

		m_dwActualNumberOfBytes += dwActualNumberOfBytes;

		return bReturn;
	}

	BOOL Write(bool bValue)
	{
		return Write(&bValue, sizeof(bValue));
	}

	BOOL Write(DWORD dwValue)
	{
		return Write(&dwValue, sizeof(dwValue));
	}

	BOOL Write(const CStringA &sValue)
	{
		return Write((CStringW)sValue);
	}

	BOOL Write(const CStringW &sValue)
	{
		ATLASSERT(IsValid());

		if (sValue.IsEmpty())
			return Write((DWORD)0);

		DWORD dwLength = sValue.GetLength();
		if (!Write(dwLength))
			return FALSE;

		DWORD dwActualNumberOfBytes = m_dwActualNumberOfBytes;

		BOOL bReturn = Write((const WCHAR*)sValue, dwLength*sizeof(WCHAR));

		m_dwActualNumberOfBytes += dwActualNumberOfBytes;

		return bReturn;
	}

	BOOL Write(	  LPCVOID lpBuffer,                // data buffer
				DWORD nNumberOfBytesToWrite,     // number of bytes to write
				LPOVERLAPPED lpOverlapped = NULL // overlapped buffer
			)
	{
		ATLASSERT(IsValid());

		return ::WriteFile(handle, lpBuffer, nNumberOfBytesToWrite, &m_dwActualNumberOfBytes, lpOverlapped);
	}

	DWORD Seek(LONG lDistanceToMove, DWORD dwMoveMethod = FILE_BEGIN )
	{
		return SetPointer(lDistanceToMove, NULL, dwMoveMethod);
	}

	DWORD Seek(LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod = FILE_BEGIN )
	{
		return SetPointer(lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
	}

	BOOL Seek(LARGE_INTEGER liDistanceToMove, DWORD dwMoveMethod = FILE_BEGIN, PLARGE_INTEGER lpNewFilePointer = NULL)
	{
		return SetPointer(liDistanceToMove, dwMoveMethod, lpNewFilePointer);
	}

	DWORD SeekToBegin()
	{
		return SetPointer(0L, NULL, FILE_BEGIN);
	}

	DWORD SeekToEnd()
	{
		return SetPointer(0L, NULL, FILE_END);
	}

	DWORD SetPointer( LONG lDistanceToMove,        // bytes to move pointer
					PLONG lpDistanceToMoveHigh,  // bytes to move pointer
					DWORD dwMoveMethod           // starting point
					)
	{
		ATLASSERT(IsValid());

		return ::SetFilePointer(handle, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
	}

	BOOL SetPointer(LARGE_INTEGER liDistanceToMove,        // bytes to move pointer
					DWORD dwMoveMethod,           // starting point
					PLARGE_INTEGER lpNewFilePointer = NULL
					)
	{
		ATLASSERT(IsValid());

		return ::SetFilePointerEx(handle, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
	}

	DWORD GetPosition(PLONG lpPositionHigh = NULL) const
	{
		ATLASSERT(IsValid());

		return ::SetFilePointer(handle, 0L, lpPositionHigh, FILE_CURRENT);
	}
	
	DWORD GetSize(LPDWORD lpFileSizeHigh = NULL /* high-order word of file size */) const
	{
		ATLASSERT(IsValid());

		return ::GetFileSize(handle, lpFileSizeHigh);
	}

	BOOL GetSize(PLARGE_INTEGER lpFileSize) const
	{
		ATLASSERT(IsValid());

		return ::GetFileSizeEx(handle, lpFileSize);
	}

	DWORD GetLength(LPDWORD lpFileSizeHigh = NULL /* high-order word of file size */) const // same as GetSize
	{
		return GetSize(lpFileSizeHigh);
	}

	BOOL GetLength(PLARGE_INTEGER lpFileSize) const // same as GetSize
	{
		return GetSize(lpFileSize);
	}

	BOOL SetSize(LONG lSize, PLONG lpSizeHigh = NULL)
	{
		SetPointer(lSize, lpSizeHigh, FILE_BEGIN);

		return ::SetEndOfFile(handle);
	}

	BOOL SetLength(LONG lSize, PLONG lpSizeHigh = NULL) // same as SetSize
	{
		return SetSize(lSize, lpSizeHigh);
	}

	DWORD GetActualNumberOfBytes() const
	{
		ATLASSERT(IsValid());

		return m_dwActualNumberOfBytes;
	}

	DWORD GetNumberOfBytesRead() const
	{
		return GetActualNumberOfBytes();
	}

	DWORD GetNumberOfBytesWrite() const
	{
		return GetActualNumberOfBytes();
	}

	const CString &GetFilePath()
	{
		return m_strFilePath;
	}

	CString GetFileName()
	{
		int idx = m_strFilePath.ReverseFind('\\');
		return idx == -1 ? m_strFilePath : m_strFilePath.Mid(idx+1);
	}
public:
	// static members
	static BOOL Delete(LPCTSTR lpszFileName)
	{
		return ::DeleteFile(lpszFileName);
	}

	static BOOL Remove(LPCTSTR lpszFileName) // same as remove
	{
		return ::DeleteFile(lpszFileName);
	}

	static BOOL Rename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
	{
		return ::MoveFile((LPTSTR)lpszOldName, (LPTSTR)lpszNewName);
	}

	static BOOL GetSize(LPCTSTR pszFile, ULONGLONG& ullFileSize)
	{
		ullFileSize = 0;

		FileFind find;
		if (!find.FindFirst(pszFile))
			return FALSE;

		ULARGE_INTEGER large = { find->nFileSizeLow, find->nFileSizeHigh };
		ullFileSize = large.QuadPart;

		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
