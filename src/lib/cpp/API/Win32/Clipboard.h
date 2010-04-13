// Clipboard.h: Clipboard class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Handle.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// Clipboard
//////////////////////////////////////////////////////////////////////

class Clipboard
{
private:
	BOOL m_bOpened;

public:
	Clipboard(HWND hwnd = NULL)
	{
		m_bOpened = ::OpenClipboard(hwnd);
	}

	BOOL Empty()
	{
		ATLASSERT(m_bOpened);

		return ::EmptyClipboard();
	}

	BOOL IsFormatAvailable(UINT format)
	{
		ATLASSERT(m_bOpened);

		return ::IsClipboardFormatAvailable(format);
	}

	HANDLE GetData(UINT uFormat)
	{
		ATLASSERT(m_bOpened);

		return ::GetClipboardData(uFormat);
	}

	~Clipboard()
	{
		if (m_bOpened) ::CloseClipboard();
	}

	bool operator ! () const
	{
		return !m_bOpened;
	}

public:
	static BOOL Empty(HWND hwnd)
	{
		Clipboard clipboard(hwnd);
		return clipboard.Empty();
	}
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
