#pragma once

/**
 * Progress interface.
 */
class ATL_NO_VTABLE IProgress
{
public:
	/**
	 * Show/Hide progress.
	 */
	virtual void ShowProgress(bool bShow) = 0;

	/**
	 * Called before item upload will start.
	 */
	virtual void SetItemName(const CString &strFile, int iFile, int iFileCount) = 0;

	/**
	 * Called when progress must be changed.
	 */
	virtual void ProgressChange(int iProgress, int iProgressMax, ULONGLONG ullUploaded, ULONGLONG ullFileSize) = 0;

	/**
	 * Called when upload complete.
	 */
	virtual void Set100Percent() = 0;

	/**
	 * Show error message.
	 *
	 * Must return YES, NO or CANCEL.
	 */
	virtual int ShowErrorMessage(LPCTSTR pszMessage, LPCTSTR pszTitle, UINT nType = MB_YESNOCANCEL|MB_ICONSTOP, LPCTSTR pszDetails = NULL) = 0;

	/**
	 * Get Progress.
	 */
	virtual CWindow GetProgressDlg() = 0;

	/**
	 * Check if Cancel was pressed.
	 */
	virtual bool IsCancelPressed() const = 0;

	/**
	 * Delete instance.
	 */
	virtual void Release() = 0;
};
