// DesktopClientDlg.h : header file
//
#pragma once

#include "ImageShackAPI.h"
#include "afxwin.h"
#include "afxcmn.h"

using namespace API;
using namespace API::ImageShack;

class ImageHelper;

#define UM_CHANGE_UPLOAD_MODE   (WM_USER + 1)

// CDesktopClientDlg dialog
class CDesktopClientDlg : public CDialog
{
// Construction
public:
	CDesktopClientDlg(CWnd* pParent = NULL);	// standard constructor
	~CDesktopClientDlg();

// Dialog Data
	enum { IDD = IDD_DESKTOPCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
    ImageShackAPI* m_pAPI;

    class UploaderListenerImpl : public UploaderListener
    {
    private:
        CDesktopClientDlg& m_rDlg;
        BOOL m_bOpenWhenComplete;

    public:
        UploaderListenerImpl(CDesktopClientDlg& rDlg, BOOL bOpenWhenComplete);
        ~UploaderListenerImpl();

    public: // IUploaderListenerImpl implementation
        virtual void InitWithUploader(const SmartReleasePtr<UploaderCancel> &);
        virtual void OnUploadComplete(const CSimpleArray<UploadInfo> &items, const CSimpleArray<UploadResult> &result);
        virtual bool OnUploadCancelled(bool force);
        virtual void OnItemComplete(const UploadInfo &item, const UploadResult &result);
        virtual UploaderErrorAction OnItemError(const UploadInfo &item, const UploaderErrorInfo &eiErrorInfo);
        virtual void OnItemSkipped(const UploadInfo &item);
        virtual void Release();
    };

    class ProgressListenerImpl : public ProgressListener
    {
    private:
        CDesktopClientDlg& m_rDlg;

    public:
        ProgressListenerImpl(CDesktopClientDlg& rDlg);
        ~ProgressListenerImpl();

    public: // IProgressListenerImpl implementation
        virtual void InitWithUploader(const SmartReleasePtr<UploaderCancel> &);
	    virtual void ShowProgress(bool bShow);
	    virtual void UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage);
	    virtual void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax);
	    virtual void Release();
    };

    CString m_strTags;

	HICON m_hIcon;
    CBrush m_brBackground;
    CBrush m_brRedBackground;

    CListCtrl m_ctrlPhotos;
    CStatic m_staticLoggedAs;
    CButton m_btnLogOut;
    CButton m_btnLogIn;

    CStatic m_ctrlVersion;
    CProgressCtrl m_ctrlProgress;
    DWORD m_dwTotalProgressMax;

    int m_iCurrentItem;
    ATL::CSimpleArray<UploadInfo>   m_arFiles;
    ATL::CSimpleArray<UploadResult> m_arResult;
    CImageList m_imglstPreview;
    ImageHelper* m_pImglstPreviewHelper;
    CImageList m_imglstBigPreview;
    ImageHelper* m_pImglstBigPreviewHelper;
    CSize m_sizeBigPreview;
    CRect m_rcBigPreview;
    CEdit m_editPhotoSearch;
    CEdit m_editTagSearch;
    int   m_nTagSearchItem;
    int   m_nPhotoSearchItem;

    CString m_strTitle;
    CString m_strDescription;
    BOOL m_bOpenWhenComplete;
    BOOL m_bScaleTo;
    CString m_strScaleTo;
    BOOL m_bPublic;
    BOOL m_bRemoveBar;

    SmartReleasePtr<UploaderCancel> m_uploader_cancel;

    bool IsSupportedFile(LPCTSTR pszFile);
    void AddFile(LPCTSTR pszFile);
    void OnLogin();
    void SaveDataToItem(int iItem);
    void LoadDataForItem(int iItem);
    void RemovePhoto(int iRemovePos);
    void RemoveUploaded();

    void OnFirstImage();
    void DisableEdit();
    void SetUploadMode(BOOL bUploadMode = TRUE);

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedLogIn();
    afx_msg void OnBnClickedLogOut();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnLvnItemchangedPhotos(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedAddPhotos();
    afx_msg void OnEnChangeSearchPhotos();
    afx_msg void OnEnKillfocusSearchPhotos();
    afx_msg void OnLvnKeydownPhotos(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg LRESULT OnChangeUploadMode(WPARAM bUploadMode, LPARAM);
    afx_msg void OnNMRClickPhotos(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnPhotoDelete();
    afx_msg void OnDestroy();
};
