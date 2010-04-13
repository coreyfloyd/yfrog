// DragAndDrop.h: Drag&Drop API
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Handle.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// DropHanler
//////////////////////////////////////////////////////////////////////

class DropHanler
{
private:
    HDROP m_hDrop;
    UINT  m_uCount;
    TCHAR m_szFilePath[MAX_PATH+1];

public:
	DropHanler(HDROP hDrop)
        : m_hDrop(hDrop)
        , m_uCount(0)
	{
        ZeroMemory(m_szFilePath, sizeof(m_szFilePath));

        if (hDrop)
        {
            m_uCount = ::DragQueryFile(m_hDrop, 0xFFFFFFFF, NULL, NULL);
        }
	}

    ~DropHanler()
    {
        if (m_hDrop) DragFinish(m_hDrop);
    }

	HDROP Detach()
	{
		HDROP h = m_hDrop;
		m_hDrop = NULL;
        m_uCount = 0;
		return h;
	}

    UINT GetCount() const
    {
        return m_uCount;
    }

    LPCTSTR operator [] (UINT uIndex)
    {
        ZeroMemory(m_szFilePath, sizeof(m_szFilePath));
        ::DragQueryFile(m_hDrop, uIndex, m_szFilePath, MAX_PATH);
		return m_szFilePath;
    }

    LPCTSTR GetFilePath()
    {
		return operator[](0);
    }

    operator HDROP () const 
	{
		return m_hDrop;
	}

    BOOL operator ! () const
    {
        return !m_hDrop;
    }
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
