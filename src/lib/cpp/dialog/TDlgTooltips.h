#pragma once

/**
 * ToolTip control for dialogs
 */
template <class T>
class TDlgToolTips
{
protected:
	CToolTipCtrl m_tooltips;

protected:
	BOOL CreateToolTipCtrl()
	{
		if (!m_tooltips.Create(((T*)this)->m_hWnd, NULL, NULL, TTS_NOPREFIX | TTS_BALLOON))
			return FALSE;

		m_tooltips.SetMaxTipWidth(400);
		return TRUE;
	}

	BOOL AddTool(UINT nCtrlID, UINT nToolTipID)
	{
		return m_tooltips.AddTool(CToolInfo(TTF_IDISHWND|TTF_SUBCLASS, ((T*)this)->GetDlgItem(nCtrlID), 0, 0, MAKEINTRESOURCE(nToolTipID)));
	}

	BOOL AddTool(const CComboBox &cbCtrl, UINT nToolTipID)
	{
		if (!m_tooltips.AddTool(CToolInfo(TTF_IDISHWND|TTF_SUBCLASS, (HWND)cbCtrl, 0, 0, MAKEINTRESOURCE(nToolTipID))))
			return FALSE;

		HWND hEdit = FindWindowEx((HWND)cbCtrl, NULL, _T("Edit"), NULL);
		if (!hEdit)
			return FALSE;

		return m_tooltips.AddTool(CToolInfo(TTF_IDISHWND|TTF_SUBCLASS, hEdit, 0, 0, MAKEINTRESOURCE(nToolTipID)));
	}

	void ActivateToolTips(BOOL bActivate = TRUE)
	{
		m_tooltips.Activate(bActivate);
	}
};
