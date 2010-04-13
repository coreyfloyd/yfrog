#pragma once

#include <atlimage.h>
#include <atltypes.h>
#include "draw_utils.h"

#ifdef USE_IMAGEMAGIC
#include "API/Win32/File.h"
#include "convert/Converter.h"
#include "util/Util.h"
using namespace UTIL;
#endif//USE_IMAGEMAGIC

#define PREVIEW_WIDTH  80
#define PREVIEW_HEIGHT 80

/**
 * Helper to create image lists.
 *
 * Stretches images and adds it to image list.
 *
 * @author Alexander Kozlov
 */
class ImageHelper
{
private:
	WTL::CDC		m_dcMem;
	WTL::CDC		m_dcMemMask;
	WTL::CDC		m_dcMemSize;
	WTL::CFont	    m_font;
	WTL::CBitmap    m_bmpTmp;
	WTL::CBitmap    m_bmpTmpMask;
	WTL::CBitmap    m_bmpTmpSize;
	WTL::CBrush     m_brushTmp;
	WTL::CPen       m_penTmp;
	WTL::CBitmap    m_bmpNoThumb;
	HPEN            m_hpenBlack;
	HBRUSH          m_hbrushBlack;
    HIMAGELIST      m_hImageList;
    CSize           m_sizePreview;
#ifndef _INTERNET_EXPLORER
	bool m_bUseWorkAround;
#endif

public:
	ImageHelper(HIMAGELIST hImageList, CSize sizePreview = CSize(PREVIEW_WIDTH, PREVIEW_HEIGHT), UINT uBitmapNoThumbnail = IDB_NO_THUMB)
		: m_hImageList(hImageList)
		, m_hpenBlack( (HPEN)::GetStockObject(BLACK_PEN) )
		, m_hbrushBlack( (HBRUSH)::GetStockObject(BLACK_BRUSH) )
        , m_sizePreview(sizePreview)
	{
		HDC hDC = ::GetDC(NULL);

		m_dcMem.CreateCompatibleDC(hDC);
		m_dcMemMask.CreateCompatibleDC(m_dcMem);
		m_dcMemSize.CreateCompatibleDC(hDC);
		m_bmpTmp.CreateCompatibleBitmap(hDC, m_sizePreview.cx, m_sizePreview.cy);
		m_bmpTmpMask.CreateCompatibleBitmap(m_dcMemMask, m_sizePreview.cx, m_sizePreview.cy);
		m_bmpTmpSize.CreateCompatibleBitmap(hDC, m_sizePreview.cx, m_sizePreview.cy);
		m_brushTmp.CreateSolidBrush(RGB(248,244,99));
		m_penTmp.CreatePen(PS_SOLID, 1, RGB(248,244,99));
		m_font.CreatePointFont(80, _T("Arial"), hDC);

#ifndef _INTERNET_EXPLORER
		m_bUseWorkAround = IsOSVistaOrHigher() && 16 != GetDeviceCaps(hDC, BITSPIXEL);
#endif

		::ReleaseDC(NULL, hDC);

		m_bmpNoThumb.LoadBitmap(uBitmapNoThumbnail);
	}

	void DrawSize(CImage &image)
	{
		if ((HBITMAP)image == m_bmpNoThumb)
			return;

		TCHAR buf[255] = {0};
		_stprintf_s(buf, _T("%ix%i"), image.GetWidth(), image.GetHeight());

		CSelectObject bmp(m_dcMemSize, m_bmpTmpSize);
		CSelectObject font(m_dcMemSize, m_font);
		CSelectObject brush(m_dcMemSize, m_brushTmp);
		CSelectObject pen(m_dcMemSize, m_penTmp);

		CRect rect(0, 0, m_sizePreview.cx, m_sizePreview.cy);
		int nHeight = m_dcMemSize.DrawText(buf, -1, &rect, DT_SINGLELINE|DT_CALCRECT);
		int nWidth = rect.right - rect.left + 2;

		m_dcMemSize.SetBkMode(TRANSPARENT);
		m_dcMemSize.PatBlt(0, 0, m_sizePreview.cx, m_sizePreview.cy, BLACKNESS);
		m_dcMemSize.BitBlt(0, 0, nWidth, nHeight, m_dcMem, (m_sizePreview.cx-nWidth)/2, m_sizePreview.cy-nHeight-1, SRCCOPY);
		m_dcMemSize.RoundRect(0, 0, nWidth, nHeight, 5, 5); 

		CRect rect2(1, 0, m_sizePreview.cx-1, m_sizePreview.cy);
		m_dcMemSize.DrawText(buf, -1, &rect2, DT_SINGLELINE);

		BLENDFUNCTION blend = {AC_SRC_OVER, 0, (255 * 60) / 100, 0/*AC_SRC_ALPHA*/ };
		m_dcMem.AlphaBlend((m_sizePreview.cx-nWidth)/2, m_sizePreview.cy-nHeight-1, nWidth, nHeight, m_dcMemSize, 0, 0, nWidth, nHeight, blend);

		CSelectObject maskPen(m_dcMemMask, m_hpenBlack);
		CSelectObject maskBrush(m_dcMemMask, m_hbrushBlack);
		m_dcMemMask.RoundRect((m_sizePreview.cx-nWidth)/2, m_sizePreview.cy-nHeight-1, (m_sizePreview.cx-nWidth)/2 + nWidth, m_sizePreview.cy-nHeight-1 + nHeight, 5, 5); 
	}

	void AlphaBlendToMemDC(CImage &image, int xDest, int yDest, int nDestWidth, int nDestHeight)
	{
#ifdef _INTERNET_EXPLORER
		image.AlphaBlend(m_dcMem, xDest, yDest, nDestWidth, nDestHeight, 0, 0, image.GetWidth(), image.GetHeight());
#else
		if (!m_bUseWorkAround)
			image.AlphaBlend(m_dcMem, xDest, yDest, nDestWidth, nDestHeight, 0, 0, image.GetWidth(), image.GetHeight());
		else
		{
			BLENDFUNCTION blend = {AC_SRC_OVER, 0, 0xff, 0/*AC_SRC_ALPHA*/ };

			CBitmap bmpTmp;
			bmpTmp.CreateCompatibleBitmap(NULL, image.GetWidth(), image.GetHeight());

			HBITMAP prevMask = m_dcMemMask.SelectBitmap(image);

			HBITMAP prevImage = m_dcMem.SelectBitmap(bmpTmp);
			m_dcMem.BitBlt(0, 0, image.GetWidth(), image.GetHeight(), m_dcMemMask, 0, 0, SRCCOPY);
			m_dcMem.SelectBitmap(prevImage);

			m_dcMemMask.SelectBitmap(bmpTmp);
			m_dcMem.AlphaBlend(xDest, yDest, nDestWidth, nDestHeight, m_dcMemMask, 0, 0, image.GetWidth(), image.GetHeight(), blend);

			m_dcMemMask.SelectBitmap(prevMask);
		}
#endif
	}

	void DrawImage(CImage &image)
	{
		m_dcMem.PatBlt(0, 0, m_sizePreview.cx, m_sizePreview.cy, WHITENESS);
		m_dcMemMask.PatBlt(0, 0, m_sizePreview.cx, m_sizePreview.cy, WHITENESS);
		if (!image.GetWidth() || !image.GetHeight())
			return;

		// use AlphaBlend instead of StretchBlt to prevent color distortion
		if (image.GetWidth() == image.GetHeight())
		{
			AlphaBlendToMemDC(image, 0, 0, m_sizePreview.cx, m_sizePreview.cx);
			m_dcMemMask.PatBlt(0, 0, m_sizePreview.cx, m_sizePreview.cy, BLACKNESS);
		}
		else if (image.GetWidth() > image.GetHeight())
		{
			int height = m_sizePreview.cx*image.GetHeight()/image.GetWidth();
            if (m_sizePreview.cy >= height)
            {
                AlphaBlendToMemDC(image, 0, (m_sizePreview.cy-height)/2, m_sizePreview.cx, height);
                m_dcMemMask.PatBlt(0, (m_sizePreview.cy-height)/2, m_sizePreview.cx, height, BLACKNESS);
            }
            else
            {
			    DWORD width = m_sizePreview.cy*image.GetWidth()/image.GetHeight();
			    AlphaBlendToMemDC(image, (m_sizePreview.cx-width)/2, 0, width, m_sizePreview.cy);
			    m_dcMemMask.PatBlt((m_sizePreview.cx-width)/2, 0, width, m_sizePreview.cy, BLACKNESS);
            }
		}
		else
		{
			int width = m_sizePreview.cy*image.GetWidth()/image.GetHeight();
            if (m_sizePreview.cx >= width)
            {
                AlphaBlendToMemDC(image, (m_sizePreview.cx-width)/2, 0, width, m_sizePreview.cy);
                m_dcMemMask.PatBlt((m_sizePreview.cx-width)/2, 0, width, m_sizePreview.cy, BLACKNESS);
            }
            else
            {
    			DWORD height = m_sizePreview.cx*image.GetHeight()/image.GetWidth();
                AlphaBlendToMemDC(image, 0, (m_sizePreview.cy-height)/2, m_sizePreview.cx, height);
                m_dcMemMask.PatBlt(0, (m_sizePreview.cy-height)/2, m_sizePreview.cx, height, BLACKNESS);
            }
		}
	}

	int Add(HBITMAP hBitmap = NULL)
	{
		if (!hBitmap) hBitmap = m_bmpNoThumb;

		HBITMAP prev = m_dcMem.SelectBitmap(m_bmpTmp);
		HBITMAP prevMask = m_dcMemMask.SelectBitmap(m_bmpTmpMask);

		CImage image;
		image.Attach(hBitmap);
		DrawImage(image);
		DrawSize(image);
		image.Detach();

		m_dcMemMask.SelectBitmap(prevMask);
		m_dcMem.SelectBitmap(prev);

        return ImageList_Add(m_hImageList, m_bmpTmp, m_bmpTmpMask);
	}

	int Add(LPCTSTR pszFileName)
	{
		HBITMAP prev = m_dcMem.SelectBitmap(m_bmpTmp);
		HBITMAP prevMask = m_dcMemMask.SelectBitmap(m_bmpTmpMask);

		CImage image;
#ifdef USE_IMAGEMAGIC
        CString strTempFile;
        if ( 0 == _tcsstr( _T("jpg,jpeg,gif,bmp,png,tiff"), ExtractFileExtension(pszFileName) ) )
        {
            strTempFile = GetTempFilePath(pszFileName) + _T(".bmp");
            if (ImageConverter::Convert(pszFileName, strTempFile))
                pszFileName = strTempFile;
        }
#endif//USE_IMAGEMAGIC

		if (SUCCEEDED(image.Load(pszFileName)))
		{
			DrawImage(image);
			DrawSize(image);
		}
		else
		{
			image.Attach(m_bmpNoThumb);
			DrawImage(image);
			image.Detach();
		}

		m_dcMemMask.SelectBitmap(prevMask);
		m_dcMem.SelectBitmap(prev);

        int idx = ImageList_Add(m_hImageList, m_bmpTmp, m_bmpTmpMask);

#ifdef USE_IMAGEMAGIC
        if (!strTempFile.IsEmpty()) File::Delete(strTempFile);
#endif//USE_IMAGEMAGIC
        return idx;
	}

public:
	// creates image list for file list
	static HIMAGELIST CreateImageList(const CAtlArray<CString> &images)
	{
		HIMAGELIST hImageList = NULL;
#ifdef _INTERNET_EXPLORER
		if (!(hImageList = ImageList_Create(PREVIEW_WIDTH, PREVIEW_HEIGHT, ILC_COLOR32|ILC_MASK, images.GetCount(), 1)))
#else
		if (!(hImageList = ImageList_Create(PREVIEW_WIDTH, PREVIEW_HEIGHT, ILC_COLOR16|ILC_MASK, images.GetCount(), 1)))
#endif
			return NULL;

		ImageHelper helper(hImageList);
		for (size_t i = 0; i < images.GetCount(); ++i)
			helper.Add(ToFile(images[i]));

		return hImageList;
	}

#ifdef __ATLSAFE_H__
	// creates image list for file list
	static HIMAGELIST CreateImageList(const CComSafeArray<BSTR> &images)
	{
		HIMAGELIST hImageList = NULL;
#ifdef _INTERNET_EXPLORER
		if (!(hImageList = ImageList_Create(PREVIEW_WIDTH, PREVIEW_HEIGHT, ILC_COLOR32|ILC_MASK, images.GetCount(), 1)))
#else
		if (!(hImageList = ImageList_Create(PREVIEW_WIDTH, PREVIEW_HEIGHT, ILC_COLOR16|ILC_MASK, images.GetCount(), 1)))
#endif
			return NULL;

		ImageHelper helper(hImageList);
		for (int i = 0, count = images.GetCount(); i < count; ++i)
			helper.Add(ToFile((CString)images[i]));

		return hImageList;
	}
#endif//__ATLSAFE_H__
};
