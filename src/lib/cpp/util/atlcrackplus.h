#pragma once

#include <atlcrack.h>

///////////////////////////////////////////////////////////////////////////////
// Generic message handlers

#define MESSAGE_HANDLER_EX0(msg, func) \
	if(uMsg == msg) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MESSAGE_RANGE_HANDLER_EX0(msgFirst, msgLast, func) \
	if(uMsg >= msgFirst && uMsg <= msgLast) \
	{ \
		SetMsgHandled(TRUE); \
		lResult = func(); \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define COMMAND_ID_HANDLER_EX0(id, func) \
	if (uMsg == WM_COMMAND && id == LOWORD(wParam)) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define COMMAND_HANDLER_EX0(id, code, func) \
	if (uMsg == WM_COMMAND && code == HIWORD(wParam) && id == LOWORD(wParam)) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define COMMAND_RANGE_CODE_HANDLER_EX0(idFirst, idLast, code, func) \
	if(uMsg == WM_COMMAND && code == HIWORD(wParam) && LOWORD(wParam) >= idFirst  && LOWORD(wParam) <= idLast) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

template <class T>
class TDataHandling
{
protected:
	LRESULT ShowMessageAndSetFocus(UINT nMessage, UINT nCtrlID)
	{
		CString strMessage;
		if (!strMessage.LoadString(nMessage))
			return 0;

		return ShowMessageAndSetFocus(strMessage, nCtrlID);
	}

	LRESULT ShowMessageAndSetFocus(LPCTSTR pszMessage, UINT nCtrlID)
	{
		T* pT = static_cast<T*>(this);

		CString strTitle;
		pT->GetWindowText(strTitle);

		pT->MessageBox(pszMessage, strTitle, MB_OK|MB_ICONEXCLAMATION );
		pT->GetDlgItem(nCtrlID).SetFocus();

		return 0;
	}

	LRESULT ShowMessageAndSetFocus(UINT nMessage, CComboBox &ctrl)
	{
		CString strMessage;
		if (!strMessage.LoadString(nMessage))
			return 0;

		return ShowMessageAndSetFocus(strMessage, ctrl);
	}

	LRESULT ShowMessageAndSetFocus(LPCTSTR pszMessage, CComboBox &ctrl)
	{
		T* pT = static_cast<T*>(this);

		CString strTitle;
		pT->GetWindowText(strTitle);

		pT->MessageBox(pszMessage, strTitle, MB_OK|MB_ICONEXCLAMATION );
		ctrl.SetFocus();
		ctrl.ShowDropDown();

		return 0;
	}
};

#ifdef __ATLCTRLS_H__

class CManagedImageList : public CImageList
{
public:
	~CManagedImageList()
	{
		if (m_hImageList) Destroy();
	}
};

#endif//__ATLCTRLS_H__
