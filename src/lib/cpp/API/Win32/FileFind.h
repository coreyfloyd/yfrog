// FileFind.h: FileFind class.
//////////////////////////////////////////////////////////////////////

#pragma once
#pragma warning (push)
#pragma warning (disable : 4800)

#include "Handle.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// FileFind
//////////////////////////////////////////////////////////////////////

class FileFind : public Handle
{
protected:
	WIN32_FIND_DATA m_data;

public:
	FileFind(void)
	{
		ZeroMemory(&m_data, sizeof(m_data));
	}

	~FileFind(void)
	{
		Close();
	}

	// Implements Closable
	virtual void Close()
	{ 
		if (IsValid()) ::FindClose(handle), handle = NULL; 
	}

public:
	BOOL FindFirst(LPCTSTR lpFileName)
	{
		HANDLE hFind = ::FindFirstFile(lpFileName, &m_data);

		Attach(hFind);

		return IsValid();
	}

	BOOL FindNext()
	{
		ATLASSERT(IsValid());

		return ::FindNextFile(handle, &m_data);
	}

	const WIN32_FIND_DATA* operator -> () const
	{
		return &m_data;
	}
};

//////////////////////////////////////////////////////////////////////
// FindFileIterator
//////////////////////////////////////////////////////////////////////

class FindFileIterator
{
protected:
	BOOL	 m_bHasNext;
	FileFind m_file_find;

public:
	FindFileIterator(LPCTSTR lpFileName)
	{
		Find(lpFileName);
	}

	BOOL operator () (LPCTSTR lpFileName)
	{
		return Find(lpFileName);
	}

	operator bool () const
	{
		return (bool)m_bHasNext;
	}

	operator BOOL () const
	{
		return m_bHasNext;
	}

	BOOL operator ++ ()
	{
		return FindNext();
	}

	BOOL operator ++ (int)
	{
		return FindNext();
	}

	const WIN32_FIND_DATA* operator -> () const
	{
		return m_file_find.operator ->();
	}

protected:
	BOOL Find(LPCTSTR lpFileName)
	{
		return m_bHasNext = m_file_find.FindFirst(lpFileName);
	}
	BOOL FindNext()
	{
		return m_bHasNext = m_file_find.FindNext();
	}
};

//////////////////////////////////////////////////////////////////////

#pragma warning (pop)

}; //namespace Win32
}; //namespace API
