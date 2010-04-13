// LoginDlg.h

#pragma once

#include "../res/resource.h"
#include "TDlgToolTips.h"

#include "ImageShackAPI.h"
using namespace API::ImageShack;

/**
 * Login dialog
 */
class CLoginDlg
	: public CDialogImpl<CLoginDlg>
	, public CWinDataExchange<CLoginDlg>
	, public TDataHandling<CLoginDlg>
	, public TDlgToolTips<CLoginDlg>
{
private:
	ImageShackAPI &m_rAPI;
	// mapped properties
	CString m_strLogin;
	CString m_strPassword;

public:
	CLoginDlg(ImageShackAPI &rAPI);

	const CString &GetLogin() const
	{
		return m_strLogin;
	}

	const CString &GetPassword() const
	{
		return m_strPassword;
	}

public:
	enum { IDD = IDD_LOGIN };

protected:
	BEGIN_MSG_MAP_EX(CLoginDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX0(IDOK, OnOK)
		COMMAND_ID_HANDLER_EX0(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX0(IDC_FORGOT_PASSSWORD, OnFogotPassword)
		
		MESSAGE_RANGE_HANDLER_EX0(WM_MOUSEFIRST,WM_MOUSELAST, OnMouse)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CLoginDlg)
		DDX_TEXT(IDC_LOGIN, m_strLogin)
		DDX_TEXT(IDC_PASSWORD, m_strPassword)
	END_DDX_MAP()

protected:
	LRESULT OnInitDialog(HWND, LPARAM);

	int	OnOK();
	void OnCancel();
	void OnFogotPassword();

	LRESULT OnMouse();
};
