#pragma once

/**
 * Select object helper class.
 */
class CSelectObject
{
	HDC		m_hDC;
	HGDIOBJ m_hObject;
	HGDIOBJ m_hPrevObject;
public:
	CSelectObject(HDC hDC, HGDIOBJ hObject)
		: m_hDC(hDC)
		, m_hObject(hObject)
	{
		m_hPrevObject = ::SelectObject(m_hDC, hObject);
	}

	HGDIOBJ operator = (HGDIOBJ hObject)
	{
		return ::SelectObject(m_hDC, m_hObject = hObject);
	}

	~CSelectObject()
	{
		if (m_hDC) ::SelectObject(m_hDC, m_hPrevObject);
	}

	operator HGDIOBJ()
	{
		return m_hObject;
	}
};

/**
 * Map Mode helper class.
 */
class CSetMapMode
{
	HDC m_hDC;
	INT m_nPrev;
public:
	CSetMapMode(HDC hDC, INT nMapMode)
		: m_hDC(hDC)
        , m_nPrev(0)
	{
        _ASSERT(m_hDC);
		if (m_hDC) m_nPrev = SetMapMode(nMapMode);
	}
	
	INT SetMapMode(INT nMapMode)
	{
        _ASSERT(m_hDC);
        return m_hDC ? ::SetMapMode(m_hDC, nMapMode) : 0;
	}
	
	INT operator = (INT nMapMode)
	{
        return SetMapMode(nMapMode);
	}
	
	~CSetMapMode()
	{
        SetMapMode(m_nPrev);
	}
};

/**
 * BkMode helper class.
 */
class CSetBkMode 
{
	HDC m_hDC;
	INT m_nPrev;
public:
	CSetBkMode(HDC hDC, INT nBkMode)
		: m_hDC(hDC)
        , m_nPrev(0)
	{
        _ASSERT(m_hDC);
		if (m_hDC) m_nPrev = SetBkMode(nBkMode);
	}
	
	INT SetBkMode(INT nBkMode)
	{
        _ASSERT(m_hDC);
        return m_hDC ? ::SetBkMode(m_hDC, nBkMode) : 0;
	}
	
	INT operator = (INT nBkMode)
	{
        return SetBkMode(nBkMode);
	}
	
	~CSetBkMode()
	{
        SetBkMode(m_nPrev);
	}
};

/**
 * Text Color helper class.
 */
class CSetTextColor 
{
protected:
	HDC         m_hDC;
	COLORREF	m_clrTextColor;
	COLORREF	m_clrPrevTextColor;

public:
	CSetTextColor(HDC hDC, COLORREF clrTextColor)
		: m_hDC(hDC)
        , m_clrPrevTextColor(0)
	{
        _ASSERT(m_hDC);
		if (m_hDC) m_clrPrevTextColor = SetTextColor(clrTextColor);
	}
	
	COLORREF SetTextColor(COLORREF clrTextColor)
	{
        _ASSERT(m_hDC);
        return m_hDC ? ::SetTextColor(m_hDC, m_clrTextColor = clrTextColor) : 0;
	}
	
	COLORREF operator = (COLORREF clrTextColor)
	{
		return SetTextColor(clrTextColor);
	}
	
	~CSetTextColor()
	{
		SetTextColor(m_clrPrevTextColor);
	}

	operator COLORREF()
	{
		return m_clrTextColor;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////

inline void Line(CDC *pDC, int x1, int y1, int x2, int y2)
{
	pDC->MoveTo(x1,y1);
	pDC->LineTo(x2,y2);
}

inline void HorizLine(CDC *pDC, int x1, int x2, int y)
{
	pDC->MoveTo(x1,y);
	pDC->LineTo(x2,y);
}

inline void VertLine(CDC *pDC, int x, int y1, int y2)
{
	pDC->MoveTo(x,y1);
	pDC->LineTo(x,y2);
}

////////////////////////////////////////////////////////////////////////////////////////////////
