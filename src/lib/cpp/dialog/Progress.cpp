// Progress.cpp : Implementation of CProgress

#include "stdafx.h"
#include "Progress.h"
#include "ImageHelper.h"
using namespace API::ImageShack;

#define MAX_PROGRESS 10000

// CProgress

CProgress::CProgress(API::ImageShack::Uploader& uploader)
	: m_uploader(uploader)
    , m_preview(*this)
	, m_bCancelActive(FALSE)
    , m_iCurrentPreview(-1)
{
}

CProgress::~CProgress()
{
    if (m_imglstPreview) m_imglstPreview.Destroy();

    // week reference to self is not valid in destructor
    m_self = 0;

    if (m_hWnd && IsWindow())
        DestroyWindow(), m_hWnd = NULL;
}

void CProgress::Release()
{
    delete this;
}

INT_PTR CALLBACK CProgress::DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CProgress* pThis = (CProgress*)hWnd;

    // object should not be destroyed till processing message
    Locker selflock(pThis->m_self);

	return __super::DialogProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CProgress::OnInitDialog(HWND, LPARAM)
{
	DoDataExchange(FALSE);

	m_message.SetWindowText( _T("") );
	m_bytes_sent.SetWindowText( _T("") );
	m_progress.SetRange32(0, MAX_PROGRESS);
	m_progress.SetPos(0);

	GetDlgItem(IDCANCEL).EnableWindow(m_bCancelActive = FALSE);

    CString strUploadingTo;
    if (strUploadingTo.LoadString(IDS_UPLOADING_TO))
    {
        strUploadingTo.Replace( _T("%domain%"), GetString(IDS_DOMAIN) );
        strUploadingTo.Replace( _T("%version%"), GetThisFileVersion() );
        SetWindowText(strUploadingTo);
    }
    
	HICON hIcon = ::LoadIcon( _AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(IDI_LOGO) );
	ATLASSERT(hIcon);
	if (hIcon) SetIcon(hIcon, TRUE);

    return TRUE;// Let the system set the focus
}

void CProgress::ShowProgress(bool bShow)
{
	if (bShow && !m_hWnd)
		Create(NULL);

	m_strMessage.Empty();
	m_strBytesSent.Empty();
	m_message.SetWindowText( _T("") );
	m_bytes_sent.SetWindowText( _T("") );
	m_progress.SetPos(0);

	ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

/**
 * method will be called before pre-process/upload for each item
 */
void CProgress::UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage)
{
    CString strFile = (CString)m_uploader.GetCurrentItem().file;
    CString strFileName = ExtractFileName(strFile);

    if (iTotal > 1)
        m_strMessage.Format(IDS_PROGRESS_MESSAGE_MULTI_FORMAT, pszMessage, strFileName, iItem, iTotal);
    else
        m_strMessage.Format(IDS_PROGRESS_MESSAGE_FORMAT, pszMessage, strFileName);

    m_message.SetWindowText(m_strMessage);

    m_iCurrentPreview = iItem-1;

 	if (!m_imglstPreview)
    {
        CWindowDC dc(*this);
    
        if (m_imglstPreview.Create(PREVIEW_WIDTH, PREVIEW_HEIGHT, ILC_COLOR24|ILC_MASK, iTotal, 1))
        {
            m_preview.ModifyStyle(0, SS_OWNERDRAW);
            m_preview.InvalidateRect(NULL);
        }
    }

    if (m_imglstPreview && m_imglstPreview.GetImageCount() <= iItem)
    {
        ImageHelper(m_imglstPreview).Add(strFile);
        m_preview.InvalidateRect(NULL);
    }
}

/**
 * called when progress has been changed
 */
void CProgress::OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax)
{
	if (!m_bCancelActive)
	{
		GetDlgItem(IDCANCEL).EnableWindow(m_bCancelActive = TRUE);
	}

    LONGLONG nNewPos;

    if (dwProgress == dwProgressMax)
    {
        if (m_bCancelActive) GetDlgItem(IDCANCEL).EnableWindow(m_bCancelActive = FALSE);

        nNewPos = MAX_PROGRESS;
    }
    else
    {
	    nNewPos = ((LONGLONG)dwProgress * MAX_PROGRESS)/dwProgressMax;
    }

#ifdef _DEBUG
    int iLower, iUpper;
    m_progress.GetRange(iLower, iUpper);
    ATLASSERT(nNewPos >= iLower && nNewPos <= iUpper);
#endif // _DEBUG

	m_progress.SetPos(nNewPos);

    if (dwBytesTotal)
    {
        TCHAR szBuf[128], szBufTotal[128];
        GetSize(dwBytesSent, szBuf);
        GetSize(dwBytesTotal, szBufTotal);
        m_strBytesSent.Format(IDS_BYTES_SENT_FORMAT, szBuf, szBufTotal);
        m_bytes_sent.SetWindowText(m_strBytesSent);
    }
}

#define KBYTE (1024)
#define MBYTE (1024 * KBYTE)
#define GBYTE (1024 * MBYTE)

LPTSTR CProgress::GetSize(ULONGLONG ullSize, LPTSTR pszBuf) const
{
    _tcscpy(pszBuf, _T(""));

	if (!ullSize)
        return pszBuf;

	if (ullSize >= (((ULONGLONG)100) * GBYTE))
		_stprintf(pszBuf, _T("%I64i GB"), ullSize/GBYTE);
	else if (ullSize >= GBYTE)
		_stprintf(pszBuf, _T("%I64i.%I64i GB"), ullSize/GBYTE, (ullSize * 10/GBYTE)%10);
	else if (ullSize >= 100 * MBYTE)
		_stprintf(pszBuf, _T("%I64i MB"), ullSize/MBYTE);
	else if (ullSize >= MBYTE)
		_stprintf(pszBuf, _T("%I64i.%I64i MB"), ullSize/MBYTE, (ullSize * 10/MBYTE)%10);
	else if (ullSize >= 100 * KBYTE)
		_stprintf(pszBuf, _T("%I64i KB"), ullSize/KBYTE);
	else if (ullSize >= KBYTE)
		_stprintf(pszBuf, _T("%I64i.%I64i KB"), ullSize/KBYTE, (ullSize * 10/KBYTE)%10);
	else
		_stprintf(pszBuf, _T("%I64i B"), ullSize);

    return pszBuf;
}

LRESULT CProgress::OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl)
{
	ATLASSERT(m_bCancelActive);

	if (m_bCancelActive) m_uploader.Cancel();

    return 0;
}

void CProgress::OnDrawItem(UINT wParam, LPDRAWITEMSTRUCT lpDIS)
{
	if (!m_hWnd)
		return;

	ATLASSERT(lpDIS);
	ATLASSERT(wParam == ID_PREVIEW);
	ATLASSERT(lpDIS->CtlType == ODT_STATIC);
	ATLASSERT(m_iCurrentPreview != -1);

	DRAWITEMSTRUCT &dis = *lpDIS;

    m_imglstPreview.Draw(dis.hDC, m_iCurrentPreview, CPoint(0,0), ILD_TRANSPARENT);
}
