// global.h
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace API {
namespace Win32 {

/**
 * Global Memory Helper
 */
template <class T>
class TGlobalPtr
{
protected:
	T FAR *	m_pTPtr;
	HGLOBAL m_hGlobalMemory;
public:
	TGlobalPtr( HGLOBAL hGlobalMemory )
		: m_hGlobalMemory(hGlobalMemory)
		, m_pTPtr(NULL)
	{
		if (hGlobalMemory) m_pTPtr = (T FAR *)::GlobalLock(hGlobalMemory);
	}

	~TGlobalPtr()
	{
		if (m_pTPtr) ::GlobalUnlock(m_hGlobalMemory);
	}
	
	void UnlockMemory()
	{
		if (m_pTPtr) ::GlobalUnlock(m_hGlobalMemory), m_pTPtr = 0;
	}

	SIZE_T GetSize()
	{
		return ::GlobalSize(m_hGlobalMemory);
	}

	operator T FAR * () const
	{
		return m_pTPtr;
	}


	T &operator * () const
	{
		return *m_pTPtr;
	}
	
	operator HGLOBAL () const
	{
		return m_hGlobalMemory;
	}

	BOOL operator ! ()
	{
		return m_pTPtr == NULL;
	}

	__declspec(property(get = GetDataPtr)) T FAR *ptr;
	T FAR * GetDataPtr() { ATLASSERT(m_pTPtr); return m_pTPtr; }

	__declspec(property(get = IsValid)) BOOL valid;
	BOOL IsValid() { return (BOOL)m_pTPtr; }

	static BOOL SetData(HGLOBAL hGlobalMemory, const T &value)
	{
		T FAR *	pTPtr = (T FAR *)::GlobalLock(hGlobalMemory);
		if (!pTPtr) return FALSE;

		*pTPtr = value;
		::GlobalUnlock(hGlobalMemory);
		return TRUE;
	}
};

//////////////////////////////////////////////////////////////////////////

template <class T>
class TGlobalStructPtr : public TGlobalPtr<T>
{
public:
	TGlobalStructPtr( HGLOBAL hGlobalMemory )
		: TGlobalPtr<T>(hGlobalMemory)
	{
	}
	
	T FAR * operator ->() const
	{
		return m_pTPtr;
	}
};

//////////////////////////////////////////////////////////////////////////

inline BOOL SetClipboardDataOrFree(UINT uFormat, HGLOBAL hMemory)
{
	if (!hMemory) return FALSE;

	HGLOBAL hSetData = ::SetClipboardData(uFormat, hMemory);
	if (!hSetData && hMemory) ::GlobalFree(hMemory);

	return hSetData != NULL;
}

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
