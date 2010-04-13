// Shell.h : Shell API

#pragma once

#include <shellapi.h>
#include <shlobj.h>

namespace API {
namespace Win32 {

    /**
	 * Shell is wrapper for Shell functions.
	 */
	class Shell
	{
	public:
#ifdef __ATLCTRLS_H__
		/**
		 * Return system image list.
		 * Don`t destroy it.
		 */
		static CImageList GetSystemImageList(bool bSmall = false)
		{
			SHFILEINFO shInfo;
			return (HIMAGELIST)::SHGetFileInfo( _T("C:\\"), 0, &shInfo, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | (bSmall ? SHGFI_SMALLICON : SHGFI_ICON) );
		}
#endif//__ATLCTRLS_H__

		/**
		 * Return index of file icon in system image list.
		 * You can use full file name or file ext (".jpeg")
		 */
		static int GetFileIconIndex(LPCTSTR pszFileOrExt)
		{
			SHFILEINFO shInfo;
			ZeroMemory(&shInfo, sizeof(shInfo));

			::SHGetFileInfo
			( pszFileOrExt
			, FILE_ATTRIBUTE_NORMAL
			, &shInfo, sizeof(shInfo)
			, SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX
			);

			return shInfo.iIcon;
		}

		static CString GetSpecialFolderPath(int nFolder, BOOL bCreate = TRUE)
		{
			CString strPath;
			BOOL bResult = ::SHGetSpecialFolderPath(NULL, strPath.GetBuffer(MAX_PATH), nFolder, bCreate);
			strPath.ReleaseBuffer();

			ATLASSERT(bResult && !strPath.IsEmpty());

			return strPath;
		}

#ifndef _WIN32_WCE
		static HINSTANCE Execute(LPCTSTR lpFile, LPCTSTR lpOperation = _T("open"), LPCTSTR lpParameters = NULL, LPCTSTR lpDirectory = NULL, INT nShowCmd = SW_SHOWNORMAL, HWND hwnd = NULL)
		{
			return ::ShellExecute(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
		}
#endif

	private:
		Shell(void)
		{
		}
	};

}; //namespace Win32
}; //namespace API
