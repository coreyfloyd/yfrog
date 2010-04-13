
// DesktopClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DesktopClient.h"
#include "DesktopClientDlg.h"
#include "ImageHelper.h"

using namespace UPLOAD;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define TEST_DEVKEY "IA5ZRTV6fb6256ccbc3c38650bdce6e6dcfc9e55"

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// UploaderListenerImpl

CDesktopClientDlg::UploaderListenerImpl::UploaderListenerImpl(CDesktopClientDlg& rDlg, BOOL bOpenWhenComplete)
    : m_rDlg(rDlg)
    , m_bOpenWhenComplete(bOpenWhenComplete)
{
    TRACE("UploaderListenerImpl::UploaderListenerImpl()\n");
}

CDesktopClientDlg::UploaderListenerImpl::~UploaderListenerImpl()
{
    TRACE("UploaderListenerImpl::~UploaderListenerImpl()\n");
}

/**
 * called before upload to have ability to cancel upload
 */
void CDesktopClientDlg::UploaderListenerImpl::InitWithUploader(const SmartReleasePtr<UploaderCancel> &uploader)
{
    TRACE("UploaderListenerImpl::InitWithUploader\n");

    m_rDlg.m_uploader_cancel = uploader;

    m_rDlg.SendMessage(UM_CHANGE_UPLOAD_MODE, TRUE);
}

/**
 * called when all item processed
 */
void CDesktopClientDlg::UploaderListenerImpl::OnUploadComplete(const CSimpleArray<UploadInfo> &items, const CSimpleArray<UploadResult> &result)
{
    TRACE("UploaderListenerImpl::UploadComplete\n");
    for (int i = 0; i < items.GetSize(); ++i)
    {
        UploadInfo item = items[i];
        UploadResult res = result[i];
        TRACE("file: %s - %s\n", (CStringA)item.file, (CStringA)res.files.image.file);
    }

    if (m_bOpenWhenComplete) m_rDlg.m_pAPI->OpenMyImagesPage();

    m_rDlg.PostMessage(UM_CHANGE_UPLOAD_MODE, FALSE);
}

/**
 * called when user press cancel.
 * if function will return false than upload will not be cancelled.
 * if force is true than return value will be ignored
 */
bool CDesktopClientDlg::UploaderListenerImpl::OnUploadCancelled(bool force)
{
    TRACE("UploaderListenerImpl::UploadCancelled(%s)\n", force ? "true" : "false");

    m_rDlg.PostMessage(UM_CHANGE_UPLOAD_MODE, FALSE);

    return true;
}

/**
 * called when current item successfully processed
 */
void CDesktopClientDlg::UploaderListenerImpl::OnItemComplete(const UploadInfo &item, const UploadResult &result)
{
    TRACE("UploaderListenerImpl::ItemComplete(%s,%s)\n", (CStringA)item.file, (CStringA)result.files.image.file);

    m_rDlg.m_arResult.Add(result);
}

/**
 * called when error has been occurred on upload current item.
 */
UploaderErrorAction CDesktopClientDlg::UploaderListenerImpl::OnItemError(const UploadInfo &item, const UploaderErrorInfo &eiErrorInfo)
{
    TRACE("UploaderListenerImpl::ItemError\n");

    CString strMessage = (CString)_T("Cannot upload file: ") + item.file + _T("\nRetry?");
    CString strTitle = _T("Upload Error...");

    int nID = m_rDlg.MessageBox(strMessage, strTitle, MB_YESNOCANCEL|MB_ICONSTOP);
    switch(nID)
    {
    case IDYES:
        return eActionRetry;
    case IDNO:
        return eActionSkip;
    default:
        return eActionCancel;
    }

    return eActionDefault;
}

/**
 * called when current item was skipped
 */
void CDesktopClientDlg::UploaderListenerImpl::OnItemSkipped(const UploadInfo &item)
{
    TRACE("UploaderListenerImpl::ItemSkipped(%s)\n", (CStringA)item.file);

    m_rDlg.m_arResult.Add(UploadResult());
}

/**
* Called to destroy
*/
void CDesktopClientDlg::UploaderListenerImpl::Release()
{
    TRACE("UploaderListenerImpl::Release\n");
    delete this;
}

// ProgressListenerImpl
CDesktopClientDlg::ProgressListenerImpl::ProgressListenerImpl(CDesktopClientDlg& rDlg)
    : m_rDlg(rDlg)
{
    TRACE("ProgressListenerImpl::ProgressListenerImpl()\n");
}

CDesktopClientDlg::ProgressListenerImpl::~ProgressListenerImpl()
{
    TRACE("ProgressListenerImpl::~ProgressListenerImpl()\n");
}

/**
 * called before upload to have ability to cancel upload
 */
void CDesktopClientDlg::ProgressListenerImpl::InitWithUploader(const SmartReleasePtr<UploaderCancel> &)
{
}

/**
 * method will be called before upload for each item
 */
void CDesktopClientDlg::ProgressListenerImpl::ShowProgress(bool bShow)
{
    TRACE("ProgressListenerImpl::ShowProgress(%s)\n", bShow ? "true" : "false");
    m_rDlg.m_ctrlProgress.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
    if (!bShow) m_rDlg.m_ctrlProgress.SetPos(0);
}	

/**
 * method will be called before pre-process/upload for each item
 */
void CDesktopClientDlg::ProgressListenerImpl::UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage)
{
    TRACE("ProgressListenerImpl::UpdateProgress(%i,%i,...)\n", iItem, iTotal);

    m_rDlg.m_ctrlPhotos.SetItemState(iItem-1, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
}	

/**
 * called when progress has been changed
 */
void CDesktopClientDlg::ProgressListenerImpl::OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax)
{
    TRACE("ProgressListenerImpl::OnChangeProgress(%i/%i, %i/%i bytes, %i/%i total)\n", dwProgress, dwProgressMax, dwBytesSent, dwBytesTotal, dwTotalProgress, dwTotalProgressMax);

    if (m_rDlg.m_dwTotalProgressMax != dwTotalProgressMax)
    {
        m_rDlg.m_dwTotalProgressMax = dwTotalProgressMax;
        m_rDlg.m_ctrlProgress.SetRange32(0, dwTotalProgressMax);
    }

    m_rDlg.m_ctrlProgress.SetPos(dwTotalProgress);
}	

/**
 * Called to destroy
 */
void CDesktopClientDlg::ProgressListenerImpl::Release()
{
    TRACE("ProgressListenerImpl::Release\n");
    delete this;
}	

// CDesktopClientDlg dialog

CDesktopClientDlg::CDesktopClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDesktopClientDlg::IDD, pParent)
    , m_strTags(_T(""))
    , m_sizeBigPreview(0,0)
    , m_iCurrentItem(-1)
    , m_rcBigPreview(0,0,0,0)
    , m_dwTotalProgressMax(0)
    , m_bOpenWhenComplete(TRUE)
    , m_bScaleTo(FALSE)
    , m_strScaleTo(_T(""))
    , m_pImglstPreviewHelper(NULL)
    , m_pImglstBigPreviewHelper(NULL)
    , m_nPhotoSearchItem(-2)
    , m_bPublic(false)
    , m_bRemoveBar(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_pAPI = new ImageShackAPI(TEST_DEVKEY);
}

CDesktopClientDlg::~CDesktopClientDlg()
{
    delete m_pAPI;

    delete m_pImglstPreviewHelper;
    delete m_pImglstBigPreviewHelper;
}

void CDesktopClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_LOGGED_AS, m_staticLoggedAs);
    DDX_Control(pDX, IDC_LOG_OUT, m_btnLogOut);
    DDX_Control(pDX, IDC_LOG_IN, m_btnLogIn);
    DDX_Control(pDX, IDC_PHOTOS, m_ctrlPhotos);
    DDX_Text(pDX, IDC_EDIT_TAGS, m_strTags);
    DDX_Control(pDX, IDC_STATIC_VERSION, m_ctrlVersion);
    DDX_Control(pDX, IDC_UPLOAD_PROGRESS, m_ctrlProgress);
    DDX_Check(pDX, IDC_CHECK_OPEN_ON_COMPLETE, m_bOpenWhenComplete);
    DDX_Control(pDX, IDC_SEARCH_PHOTOS, m_editPhotoSearch);
    DDX_Check(pDX, IDC_CHECK_SCALE, m_bScaleTo);
    DDX_Text(pDX, IDC_EDIT_SCALE, m_strScaleTo);
    DDX_Check(pDX, IDC_CHECK_PUBLIC, m_bPublic);
    DDX_Check(pDX, IDC_CHECK_REMOVE_BAR, m_bRemoveBar);
}

BEGIN_MESSAGE_MAP(CDesktopClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    ON_WM_CTLCOLOR()
    ON_WM_DRAWITEM()
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_PHOTOS, &CDesktopClientDlg::OnLvnItemchangedPhotos)
    ON_BN_CLICKED(IDC_LOG_IN, &CDesktopClientDlg::OnBnClickedLogIn)
    ON_BN_CLICKED(IDC_LOG_OUT, &CDesktopClientDlg::OnBnClickedLogOut)
    ON_BN_CLICKED(IDOK, &CDesktopClientDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_CANCEL, &CDesktopClientDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_ADD_PHOTOS, &CDesktopClientDlg::OnBnClickedAddPhotos)
    ON_EN_CHANGE(IDC_SEARCH_PHOTOS, &CDesktopClientDlg::OnEnChangeSearchPhotos)
    ON_EN_KILLFOCUS(IDC_SEARCH_PHOTOS, &CDesktopClientDlg::OnEnKillfocusSearchPhotos)
    ON_NOTIFY(LVN_KEYDOWN, IDC_PHOTOS, &CDesktopClientDlg::OnLvnKeydownPhotos)
    ON_MESSAGE(UM_CHANGE_UPLOAD_MODE, &CDesktopClientDlg::OnChangeUploadMode) 
    ON_WM_DROPFILES()
    ON_WM_SHOWWINDOW()
    ON_NOTIFY(NM_RCLICK, IDC_PHOTOS, &CDesktopClientDlg::OnNMRClickPhotos)
    ON_COMMAND(ID_PHOTO_DELETE, &CDesktopClientDlg::OnPhotoDelete)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

void CDesktopClientDlg::OnDestroy()
{
    delete m_pAPI, m_pAPI = NULL;

    CDialog::OnDestroy();
}

LRESULT CDesktopClientDlg::OnChangeUploadMode(WPARAM bUploadMode, LPARAM)
{
    SetUploadMode(bUploadMode);
    return 0;
}

void CDesktopClientDlg::SetUploadMode(BOOL bUploadMode/* = TRUE*/)
{
    int IDS[] = {
        IDC_LOG_IN, IDC_LOG_OUT,
        IDC_EDIT_TAGS, 
        IDC_CHECK_SCALE, IDC_EDIT_SCALE,
        IDC_CHECK_PUBLIC, IDC_CHECK_REMOVE_BAR,
        IDC_CHECK_OPEN_ON_COMPLETE,
        IDC_SEARCH_PHOTOS, 
        IDC_ADD_PHOTOS
    };

    CWnd* pWnd;
    for (int i = 0; i < sizeof(IDS)/sizeof(IDS[0]); ++i)
    {
        if (pWnd = GetDlgItem(IDS[i])) pWnd->EnableWindow(!bUploadMode);
    }

    if (pWnd = GetDlgItem(IDOK))       pWnd->ShowWindow(bUploadMode ? SW_HIDE : SW_SHOW);
    if (pWnd = GetDlgItem(IDC_CANCEL)) pWnd->ShowWindow(bUploadMode ? SW_SHOW : SW_HIDE);

    if (!bUploadMode)
    {
        m_uploader_cancel.Release();
        RemoveUploaded();
    }
}

int g_EditIDS[] = {
    IDC_EDIT_TAGS,
    IDC_CHECK_SCALE, IDC_EDIT_SCALE,
    IDC_CHECK_PUBLIC, IDC_CHECK_REMOVE_BAR
};

void CDesktopClientDlg::OnFirstImage()
{
    m_ctrlPhotos.SetItemState(0, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);

    CWnd* pWnd;
    for (int i = 0; i < sizeof(g_EditIDS)/sizeof(g_EditIDS[0]); ++i)
    {
        if (pWnd = GetDlgItem(g_EditIDS[i])) pWnd->EnableWindow();
    }

    GetDlgItem(IDOK)->EnableWindow();
}

void CDesktopClientDlg::DisableEdit()
{
    CWnd* pWnd;
    for (int i = 0; i < sizeof(g_EditIDS)/sizeof(g_EditIDS[0]); ++i)
    {
        if (pWnd = GetDlgItem(g_EditIDS[i])) pWnd->EnableWindow(FALSE);
    }

    GetDlgItem(IDOK)->EnableWindow(FALSE);
}

// CDesktopClientDlg message handlers

BOOL CDesktopClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    m_ctrlVersion.SetWindowText(GetThisFileVersion());

    m_brBackground.CreateSolidBrush(RGB(224,224,224));
    m_brRedBackground.CreateSolidBrush(RGB(255,230,230));

    DragAcceptFiles(TRUE);

    CWnd* pBigPreview = GetDlgItem(IDC_BIG_PREVIEW);
    ASSERT(pBigPreview);

    pBigPreview->GetWindowRect(&m_rcBigPreview);
    ScreenToClient(&m_rcBigPreview);
    m_sizeBigPreview = CSize(m_rcBigPreview.Width(), m_rcBigPreview.Height());

    if (m_imglstBigPreview.Create(m_sizeBigPreview.cx, m_sizeBigPreview.cy, ILC_COLOR16|ILC_MASK, 1, 3))
    {
        ImageHelper(m_imglstBigPreview, m_sizeBigPreview, IDB_EMPTY_THUMB_BIG).Add();
        
        m_pImglstBigPreviewHelper = new ImageHelper(m_imglstBigPreview, m_sizeBigPreview, IDB_NO_THUMB_BIG);

        pBigPreview->ModifyStyle(0, SS_OWNERDRAW);
    }

    if (m_imglstPreview.Create(PREVIEW_WIDTH, PREVIEW_HEIGHT, ILC_COLOR16|ILC_MASK, 1, 3))
    {
        m_pImglstPreviewHelper = new ImageHelper(m_imglstPreview);
        m_pImglstPreviewHelper->Add();
    }
    m_ctrlPhotos.SetImageList(&m_imglstPreview, LVSIL_NORMAL);

    CEdit* peditScale = (CEdit*)GetDlgItem(IDC_EDIT_SCALE);
    ASSERT(peditScale);
    peditScale->SetLimitText(5);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDesktopClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDesktopClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDesktopClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CDesktopClientDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    pDC->SetBkMode(TRANSPARENT);

    if (pWnd == this)
        return m_brBackground;

    if (pWnd == &m_editPhotoSearch && m_nPhotoSearchItem == -1)
        return m_brRedBackground;

    TCHAR szClassName[256];
	if (::GetClassName(pWnd->GetSafeHwnd(), szClassName, 256))
	{
		szClassName[255] = NULL;
		if ( 0 == _tcscmp(szClassName, _T("Static")) ||  0 == _tcscmp(szClassName, _T("Button")))
            return m_brBackground;
	}

    return hbr;
}

void CDesktopClientDlg::OnBnClickedLogIn()
{
    UpdateData(FALSE);

    if (m_pAPI->Login())
        OnLogin();
}

void CDesktopClientDlg::OnLogin()
{
    m_staticLoggedAs.SetWindowText(FormatString(IDS_LOGGED_IN_AS_FORMAT, m_pAPI->GetUserInfo()->GetName()));
    m_staticLoggedAs.ShowWindow(SW_SHOW);
    m_btnLogIn.ShowWindow(SW_HIDE);
    m_btnLogOut.ShowWindow(SW_SHOW);
}

void CDesktopClientDlg::OnBnClickedLogOut()
{
    m_pAPI->Logout();
    m_staticLoggedAs.ShowWindow(SW_HIDE);
    m_btnLogOut.ShowWindow(SW_HIDE);
    m_btnLogIn.ShowWindow(SW_SHOW);
}

void CDesktopClientDlg::OnBnClickedAddPhotos()
{
    CFileDialog dlgFile(TRUE, 0, 0, OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT, GetString(IDS_FILE_FILTER), this);

    CString strFileNameBuf;
    const int c_cMaxFiles = 100;
    const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
    dlgFile.GetOFN().lpstrFile = strFileNameBuf.GetBuffer(c_cbBuffSize);
    dlgFile.GetOFN().nMaxFile = c_cbBuffSize;

    if (IDOK == dlgFile.DoModal())
    {
        for (POSITION pos = dlgFile.GetStartPosition(); pos; )
        {
            AddFile(dlgFile.GetNextPathName(pos));
        }
    }

    strFileNameBuf.ReleaseBuffer();
}

void CDesktopClientDlg::OnDropFiles(HDROP hDropInfo)
{
    DropHanler drop(hDropInfo);

    for (UINT i = 0; i < drop.GetCount(); ++i)
    {
        AddFile(drop[i]);
    }
}

bool CDesktopClientDlg::IsSupportedFile(LPCTSTR pszFile)
{
    return -1 != GetString(IDS_SUPPORTED_FORMATS).Find(ExtractFileExtension(pszFile));
}

void CDesktopClientDlg::AddFile(LPCTSTR pszFile)
{
    if (!IsSupportedFile(pszFile))
        return;

    int nNewPos = m_arFiles.GetSize();
    m_arFiles.Add(pszFile);

    int idxImage = m_pImglstPreviewHelper->Add(pszFile);
    int nPos = m_ctrlPhotos.InsertItem(m_ctrlPhotos.GetItemCount(), ExtractOnlyFileName(pszFile), idxImage);
    m_ctrlPhotos.SetItemData(nPos, nNewPos);

    m_pImglstBigPreviewHelper->Add(pszFile);

    if (1 == m_arFiles.GetSize()) // first image
        OnFirstImage();
}

void CDesktopClientDlg::SaveDataToItem(int iItem)
{
    if (iItem < 0 || iItem >= m_arFiles.GetSize())
        return;

    UpdateData();

    UploadInfo& info = m_arFiles[iItem];

    info.tags = m_strTags;
    if (m_bScaleTo)
        info.size = m_strScaleTo + _T("x") + m_strScaleTo;
    else
        info.size.Empty();
    info.is_public = m_bPublic;
    info.remove_bar = m_bRemoveBar;
}

void CDesktopClientDlg::LoadDataForItem(int iItem)
{
    UploadInfo tmpEmpty;
    UploadInfo& info = iItem == -1 ? tmpEmpty : m_arFiles[iItem];

    m_strTags = info.tags;

    if (info.size.IsEmpty())
        m_strScaleTo.Empty();
    else
    {
        int idx = info.size.Find( _T("x") );
        if (idx != -1)
            m_strScaleTo = info.size.Left(idx);
        else
        {
            ASSERT(FALSE);
            m_strScaleTo = info.size;
        }
    }
    m_bScaleTo = !m_strScaleTo.IsEmpty();
    m_bPublic = info.is_public;
    m_bRemoveBar = info.remove_bar;

    UpdateData(FALSE);

    InvalidateRect(&m_rcBigPreview);
}

void CDesktopClientDlg::OnLvnItemchangedPhotos(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    *pResult = 0;

    if (!(pNMLV->uChanged & LVIF_STATE))
        return;

    if (pNMLV->uOldState & LVIS_SELECTED)
    {
        SaveDataToItem(pNMLV->iItem);
        m_iCurrentItem = -1;
        //TODO: disable all controls
    }

    if (pNMLV->uNewState & LVIS_SELECTED)
    {
        LoadDataForItem(pNMLV->iItem);
        m_iCurrentItem = pNMLV->iItem;
    }
}

void CDesktopClientDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (nIDCtl == IDC_BIG_PREVIEW)
    {
        ATLASSERT(lpDrawItemStruct);
        ATLASSERT(lpDrawItemStruct->CtlType == ODT_STATIC);

        DRAWITEMSTRUCT &dis = *lpDrawItemStruct;

        m_imglstBigPreview.Draw(CDC::FromHandle(dis.hDC), m_iCurrentItem+1, CPoint(0,0), ILD_TRANSPARENT);
    }

    CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CDesktopClientDlg::OnBnClickedOk()
{
    SaveDataToItem(m_iCurrentItem);

    ASSERT(m_pAPI);
    m_pAPI->UploadFiles(m_arFiles, UploaderListenerSmartPtr(new UploaderListenerImpl(*this, m_bOpenWhenComplete)), ProgressListenerSmartPtr(new ProgressListenerImpl(*this)));   
}

void CDesktopClientDlg::OnBnClickedCancel()
{
    if (m_uploader_cancel) m_uploader_cancel->Cancel();
}

void CDesktopClientDlg::OnEnChangeSearchPhotos()
{
    int nPhotoSearchItem = -2;

    CString strSearchText;
    m_editPhotoSearch.GetWindowText(strSearchText);

    if (!strSearchText.IsEmpty())
    {
        LVFINDINFO fiSearch = { LVFI_PARTIAL };
        fiSearch.psz = strSearchText;

        nPhotoSearchItem = m_ctrlPhotos.FindItem(&fiSearch);
    }

    if (nPhotoSearchItem != m_nPhotoSearchItem)
    {
        bool bNeedRedraw = ((m_nPhotoSearchItem == -1) || (nPhotoSearchItem == -1));

        if (nPhotoSearchItem >= 0)
        {
            if (nPhotoSearchItem != m_ctrlPhotos.GetSelectionMark())
            {
                m_ctrlPhotos.EnsureVisible(nPhotoSearchItem, FALSE);
                m_ctrlPhotos.SetItemState(nPhotoSearchItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
            }
        }

        m_nPhotoSearchItem = nPhotoSearchItem;

        if (bNeedRedraw) m_editPhotoSearch.InvalidateRect(NULL);
    }
}

void CDesktopClientDlg::OnEnKillfocusSearchPhotos()
{
    m_nPhotoSearchItem = -2;
    m_editPhotoSearch.SetWindowText(_T(""));
}

void CDesktopClientDlg::OnLvnKeydownPhotos(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
    *pResult = 0;

    if (VK_DELETE == pLVKeyDow->wVKey && m_iCurrentItem != -1)
    {
        RemovePhoto(m_iCurrentItem);
    }
}

void CDesktopClientDlg::OnNMRClickPhotos(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    if (pNMItemActivate->iItem == -1)
        return;

    *pResult = 0;

    CMenu popup;
    if (!popup.LoadMenu(IDR_CONTEXTMENU))
        return;

    CMenu* pPopup = popup.GetSubMenu(4);

    CPoint ptMouse = pNMItemActivate->ptAction;
    m_ctrlPhotos.ClientToScreen(&ptMouse);

    pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON, ptMouse.x, ptMouse.y, this);
}

void CDesktopClientDlg::OnPhotoDelete()
{
    RemovePhoto(m_iCurrentItem);
}

void CDesktopClientDlg::RemovePhoto(int iRemovePos)
{
    if (iRemovePos == -1)
        return;

    int iCurrentItem = m_iCurrentItem;
    if (iRemovePos == iCurrentItem)
        m_ctrlPhotos.SetItemState(-1, 0, LVIS_FOCUSED|LVIS_SELECTED);

    m_arFiles.RemoveAt(iRemovePos);
    m_ctrlPhotos.DeleteItem(iRemovePos);

    m_imglstPreview.Remove(iRemovePos+1);
    m_imglstBigPreview.Remove(iRemovePos+1);

    for (int iItem = 0, iCount = m_ctrlPhotos.GetItemCount(); iItem < iCount; ++iItem)
    {
        LVITEM lvItem = {LVIF_IMAGE|LVIF_PARAM, iItem};
        if (!m_ctrlPhotos.GetItem(&lvItem))
            continue;
        if (lvItem.iImage < (iRemovePos+1) && lvItem.lParam < iRemovePos)
            continue;
        if (lvItem.iImage > (iRemovePos+1))
            --lvItem.iImage;
        if (lvItem.lParam > iRemovePos)
            --lvItem.lParam;
        m_ctrlPhotos.SetItem(&lvItem);
    }

    m_ctrlPhotos.Arrange(LVA_ALIGNTOP);

    if (iRemovePos == iCurrentItem)
    {
        if (iCurrentItem >= m_ctrlPhotos.GetItemCount())
            --iCurrentItem;

        if (iCurrentItem >= 0 && iCurrentItem < m_ctrlPhotos.GetItemCount())
            m_ctrlPhotos.SetItemState(iCurrentItem, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
        else
            LoadDataForItem(-1);
    }

    if (!m_ctrlPhotos.GetItemCount()) DisableEdit();
}

void CDesktopClientDlg::RemoveUploaded()
{
    m_ctrlPhotos.SetItemState(-1, 0, LVIS_FOCUSED|LVIS_SELECTED);

    for (int i = m_arResult.GetSize()-1; i >= 0; --i)
    {
        if (m_arResult[i].files.image.file.IsEmpty())
            continue;

        RemovePhoto(i);
    }
    m_arResult.RemoveAll();

    if (m_ctrlPhotos.GetItemCount())
        m_ctrlPhotos.SetItemState(0, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
    else
        LoadDataForItem(-1);
}
