// Progress.h : Declaration of the CProgress

#pragma once

#include "../res/resource.h"
#include "ImageShackAPI.h"

/**
 * Upload Progress Dialog.
 */
class CProgress
	: public CDialogImpl<CProgress>
	, public CWinDataExchange<CProgress>
    , public API::ImageShack::ProgressListener
{
private:
	CString				m_strMessage;
	CString				m_strBytesSent;
	API::ImageShack::Uploader& m_uploader;
	BOOL volatile 		m_bCancelActive;

    SmartReleasePtr<API::ImageShack::ProgressListener>::WeekPtr m_self;
    typedef WeekPtrLocker<typename SmartReleasePtr<API::ImageShack::ProgressListener>::WeekPtr> Locker;

    CImageList          m_imglstPreview;
    int                 m_iCurrentPreview;

public:
	CStatic             m_message;
	CStatic             m_bytes_sent;
    CProgressBarCtrl    m_progress;
	CStatic             m_preview;

protected:
	CProgress(API::ImageShack::Uploader& uploader);
	~CProgress();

public:
    static SmartReleasePtr<API::ImageShack::ProgressListener> NewInstance(API::ImageShack::Uploader& uploader)
    {
        CProgress* pProgress = new CProgress(uploader);
        SmartReleasePtr<API::ImageShack::ProgressListener> smartPtr(pProgress);
        //save week ptr
        pProgress->m_self = smartPtr.GetWeekPtr();

        return smartPtr;
    }

public:
    /**
     * called before upload to have ability to cancel upload
     */
    void InitWithUploader(const SmartReleasePtr<API::ImageShack::UploaderCancel> &) {}

	/**
	 * method will be called before upload for each item
	 */
	void ShowProgress(bool bShow);

    /**
	 * method will be called before pre-process/upload for each item
	 */
	void UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage);

    /**
	 * called when progress has been changed
	 */
	void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax);

    /**
	 * Called to destroy
	 */
	void Release();

	enum { IDD = IDD_PROGRESS };

BEGIN_MSG_MAP_EX(CProgress)
	MSG_WM_INITDIALOG(OnInitDialog)
	MSG_WM_DRAWITEM(OnDrawItem)
	COMMAND_HANDLER_EX(IDCANCEL, BN_CLICKED, OnClickedCancel)
END_MSG_MAP()

BEGIN_DDX_MAP(CProgress)
	DDX_CONTROL_HANDLE(IDC_PROGRESS, m_progress)
	DDX_CONTROL_HANDLE(IDC_PROGRESS_MESSAGE, m_message)
	DDX_CONTROL_HANDLE(IDC_BYTES_SENT, m_bytes_sent)
    DDX_CONTROL_HANDLE(ID_PREVIEW, m_preview)
END_DDX_MAP()

protected:
	LRESULT OnInitDialog(HWND, LPARAM);
	LRESULT OnClickedCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl);

    void OnDrawItem(UINT wParam, LPDRAWITEMSTRUCT lpDIS);

    LPTSTR GetSize(ULONGLONG ullSize, LPTSTR pszBuf) const;

    static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual DLGPROC GetDialogProc()
	{
		return DialogProc;
	}
};


