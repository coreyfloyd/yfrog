#pragma once

#include <atlcoll.h>
#include <atlenc.h>
#include "Logger.h"
#include "API/Win32/regkey.h"

namespace UTIL {

/**
 * Loads string.
 */
inline CString GetString(UINT nID)
{
	CString value;
	value.LoadString(nID);
	return value;
}

/**
 * Loads string. If it`s unavailable pszDefault will be returned.
 */
inline CString GetString(UINT nID, LPCTSTR pszDefault)
{
	CString value;
	if (!value.LoadString(nID))
		return pszDefault;
	return value;
}

/**
 * Returns formatted string
 */
inline CString FormatString(UINT nFormatID, ...)
{
	CString strFormat;
	if (!strFormat.LoadString(nFormatID))
	{
		ATLASSERT(FALSE);
		return CString();
	}

	CString strResult;

	va_list argList;
	va_start( argList, nFormatID );
	strResult.FormatV( strFormat, argList );
	va_end( argList );

	return strResult;
}

/**
 * Returns formatted string
 */
inline CStringA FormatString(LPCSTR pszFormatID, ...)
{
	CStringA strResult;

	va_list argList;
	va_start( argList, pszFormatID );
	strResult.FormatV( pszFormatID, argList );
	va_end( argList );

	return strResult;
}

/**
 * Returns formatted string
 */
inline CStringW FormatString(LPCWSTR pszFormatID, ...)
{
	CStringW strResult;

	va_list argList;
	va_start( argList, pszFormatID );
	strResult.FormatV( pszFormatID, argList );
	va_end( argList );

	return strResult;
}

#ifdef __ATLWIN_H__

/**
 * Returns formatted string
 */
inline CString GetWindowText(CWindow wnd)
{
	if (!wnd)
		return CString();

	CString strResult;
	wnd.GetWindowText(strResult);
	return strResult;
}

#endif//__ATLWIN_H__

#define GetWindowTitle() UTIL::GetWindowText(*this)
#define GetParentTitle() UTIL::GetWindowText(this->GetParent())

/**
 * Determines if URL is a file url.
 */
inline bool IsFileURL(LPCSTR url)
{
	return !_strnicmp(url, "file:///", 8);
}
inline bool IsFileURL(LPCWSTR url)
{
	return !_wcsnicmp(url, L"file:///", 8);
}

/**
 * Determines if URL is a HTTP url.
 */
inline bool IsHttpURL(LPCSTR url)
{
	return !_strnicmp(url, "http://", 7);
}
inline bool IsHttpURL(LPCWSTR url)
{
	return !_wcsnicmp(url, L"http://", 7);
}

/**
 * Determines if file is temporary.
 */
inline bool IsTempFile(LPCTSTR file)
{
	TCHAR path[MAX_PATH+1];
	ZeroMemory(path, sizeof(path));
	::GetTempPath(MAX_PATH, path);

	if (0 == _tcsnicmp(file, _T("file:///"), 8))
		file += 8;

	return 0 == _tcsnicmp(file, path, _tcslen(path));
}

/**
 * URL decoding.
 */
inline CString Decode(CString value)
{
	for (int idx; (idx = value.Find( _T('%') )) != -1; )
	{
		ATLASSERT(idx + 2 < value.GetLength());
		if (idx + 2 >= value.GetLength())
			break;

		DWORD dwCode;
		if (1 != _stscanf_s(value.Mid(idx+1, 2), _T("%x"), &dwCode))
		{ ATLASSERT(FALSE); break; }

		TCHAR szCharString[2] = { (TCHAR)dwCode, 0 };
		int nCount = value.Replace( value.Mid(idx, 3), szCharString );
		ATLASSERT(nCount > 0);
	}

	return value;
}

/**
 * URL to file.
 */
inline CString ToFile(CString url)
{
	//TODO check if this is url but not a file url
	if(!IsFileURL(url))
		return url;

	url = Decode(url);

	url.Delete(0, 8); // remove file:///
	url.Replace( _T('/'), _T('\\') );

	return url;
}

/**
 * File to URL.
 */
inline CString ToFileURL(CString file)
{
	if (IsFileURL(file))
		return file;

	file.Insert(0, _T("file:///"));
	file.Replace( _T('\\'), _T('/') );
	//TODO encode

	return file;
}

/**
 * Extracts File Name from path
 */
inline CString ExtractFileName(const CString &file)
{
	int idx = file.ReverseFind('\\');
	if (-1 == idx) idx = file.ReverseFind('/');
	return idx == -1 ? file : file.Mid(idx+1);
}

/**
* Extracts File Name from path without extension
*/
inline CString ExtractOnlyFileName(const CString &file)
{
	CString fileName = ExtractFileName(file);

	int idx = fileName.ReverseFind('.');
	if (-1 != idx) fileName.Delete(idx, fileName.GetLength() - idx);

	return fileName;
}

/**
 * Extracts File Extension from path
 */
inline CStringA ExtractFileExtension(const CStringA &file)
{
	int idx = file.ReverseFind('.');
	return idx == -1 ? CStringA() : file.Mid(idx+1).MakeLower();
}

inline CStringW ExtractFileExtension(const CStringW &file)
{
	int idx = file.ReverseFind(L'.');
	return idx == -1 ? CStringW() : file.Mid(idx+1).MakeLower();
}

/**
 * Returns temp file path without extension.
 * Max length is (MAX_PATH - 4).
 */
inline CString GetTempFilePath(const CString &strFile)
{
	TCHAR path[MAX_PATH];
	ZeroMemory(path, sizeof(path));
	::GetTempPath(MAX_PATH, path);

	int idx = strFile.ReverseFind( _T('/') );
	if (idx == -1)
		idx = strFile.ReverseFind( _T('\\') );

	if (idx == -1)
		_tcscat_s(path, _T("img"));
	else
	{
		int idx2 = strFile.ReverseFind( _T('.') );
		if (idx2 == -1 || idx2 < idx)
			_tcscat_s(path, strFile.Mid(idx+1));
		else
			_tcscat_s(path, strFile.Mid(idx+1, idx2-idx-1));
	}

	TCHAR bufTick[32] = _T("");
	_itot_s(::GetTickCount(), bufTick, 16);
	_tcscat_s(path, bufTick);

	// extension is mandatory
	if ( _tcslen(path) > MAX_PATH - 5 )
		path[MAX_PATH - 4] = 0;

	return path;
}

inline CString GetContentType(const CString& strFileExt)
{
    API::Win32::RegKey key;
    if (!key.OpenEx(HKEY_CLASSES_ROOT, (CString)_T(".")+strFileExt))
		return CString();

    CString strContentType;
    if (key.QueryValue(_T("Content Type"), strContentType) && !strContentType.IsEmpty())
        return strContentType;

    if (strFileExt == _T("flv"))
        return _T("video/x-flv");

    CString strPerceivedType;
    if (!key.QueryValue(_T("PerceivedType"), strPerceivedType) || strPerceivedType.IsEmpty())
        return CString();
    
    return strPerceivedType + _T("/") + strFileExt;
}

#ifdef _DEBUG
inline void TRACEREFCOUNT(IUnknown *pUnk)
{
	if (!pUnk)
		return;

	pUnk->AddRef();
	ULONG lCount = pUnk->Release();

	CLogger logger("TRACEREFCOUNT");
	LOG( _T("RefCount: %i\n"), lCount );
}
#else
#define TRACEREFCOUNT __noop
#endif // _DEBUG

inline DWORD GetOSMajorVersion()	// Vista or higher OS
{
	OSVERSIONINFO osvi;
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	return osvi.dwMajorVersion;
}

inline bool IsOSVistaOrHigher()	// Vista or higher OS
{
	static bool OS_VistaOrHigher = (GetOSMajorVersion() > 5);		// Vista or higher OS

	return OS_VistaOrHigher;
}

inline BOOL IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

	if (!fnIsWow64Process)
		return FALSE;

	BOOL bIsWow64 = FALSE;

	if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		return FALSE;

	return bIsWow64;
}

// get dll version
CString GetThisFileVersion();
CString GetVersion(LPCTSTR pszFile);
CString GetProcessVersionInfo(LPCTSTR pszKey);

/**
 * Returns DLL path (with slash at the end of line).
 */
inline CString GetThisDLLPath()
{
	TCHAR path[MAX_PATH+1];
	ZeroMemory(path, sizeof(path));
#ifdef __ATLAPP_H__
    if (!GetModuleFileName(WTL::ModuleHelper::GetModuleInstance(), path, MAX_PATH))
		return CString();
#else
	if (!GetModuleFileName(NULL, path, MAX_PATH))
		return CString();
#endif __ATLAPP_H__

	LPTSTR pszPos = _tcsrchr(path, _T('\\') );
	if (pszPos) *(pszPos+1) = 0;

	return path;
}

#ifdef __ATLTIME_H__

inline CString ConvertToRFC3339DATE(const CTime &time)
{
	return time.FormatGmt( _T("%Y-%m-%dT%H:%M:%SZ") );
}

#endif//__ATLTIME_H__

// UTF-8

inline CStringA EncodeToUTF8(const CStringW &strSource)
{
	CStringA strResult;

	int nCnt = AtlUnicodeToUTF8(strSource, strSource.GetLength(), NULL, 0);
	if (nCnt != 0)
	{
		nCnt = AtlUnicodeToUTF8(strSource, strSource.GetLength(), strResult.GetBuffer(nCnt), nCnt+1);
		strResult.ReleaseBuffer(nCnt);
	}

	return strResult;
}

CStringW DecodeFromUTF8(const CStringA &strSource);

inline CStringA EncodeToUTF8(const CStringA &strSource)
{
	return EncodeToUTF8((CStringW)strSource);
}

inline CStringA EncodeToUTF8(LPCTSTR pszSource)
{
	return EncodeToUTF8((CStringW)pszSource);
}

inline CStringW DecodeFromUTF8(const CStringW &strSource)
{
	return DecodeFromUTF8((CStringA)strSource);
}

// BASE-64

CStringA EncodeToBASE64(const BYTE *pSource, int iLength);

inline  CStringA EncodeToBASE64(const CStringA &strSource)
{
	return EncodeToBASE64((const BYTE*)(LPCSTR)strSource, strSource.GetLength());
}

inline  CStringA EncodeToBASE64UTF8(const CStringW &strSource)
{
	return EncodeToBASE64(EncodeToUTF8(strSource));
}

inline  CStringA EncodeToBASE64UTF8(const CStringA &strSource)
{
	return EncodeToBASE64UTF8((CStringW)strSource);
}

inline  CStringA EncodeToBASE64(const CAtlArray<BYTE> &arSource)
{
	return EncodeToBASE64(arSource.GetData(), arSource.GetCount());
}

CAtlArray<BYTE> &DecodeBASE64(const CStringA &strSource, CAtlArray<BYTE> &arResult);

}; //namespace UTIL
