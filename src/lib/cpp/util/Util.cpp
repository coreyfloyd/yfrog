#include "StdAfx.h"
#include "Util.h"
#include <atlenc.h>

namespace UTIL {

CStringW DecodeFromUTF8(const CStringA &strSource)
{
	CStringW strResult;

	int iCount = MultiByteToWideChar(CP_UTF8, 0, strSource, strSource.GetLength(), strResult.GetBuffer(strSource.GetLength()), strSource.GetLength()+1);
	strResult.ReleaseBuffer(iCount);

	return strResult;
}

CStringA EncodeToBASE64(const BYTE *pSource, int iLength)
{
	int nLength = Base64EncodeGetRequiredLength(iLength);

	CStringA strResult;
	if (!Base64Encode(pSource, iLength, strResult.GetBuffer(nLength), &nLength, ATL_BASE64_FLAG_NOCRLF))
	{
		ATLASSERT(FALSE);
	}

	strResult.ReleaseBuffer(nLength);

	return strResult;
}

CAtlArray<BYTE> &DecodeBASE64(const CStringA &strSource, CAtlArray<BYTE> &arResult)
{
	int nLength = Base64DecodeGetRequiredLength(strSource.GetLength());

	arResult.SetCount(nLength);
	if (!Base64Decode(strSource, strSource.GetLength(), arResult.GetData(), &nLength))
	{
		ATLASSERT(FALSE);
	}
	arResult.SetCount(nLength);

	return arResult;
}

bool GetVersion(LPCTSTR pszFile, VS_FIXEDFILEINFO &rInfo)
{
	DWORD dwHandle = 0;
	DWORD dwSize = ::GetFileVersionInfoSize(pszFile, &dwHandle);
	if (!dwSize) return false;

	CAtlArray<BYTE> buffer;
	if (!buffer.SetCount(dwSize))
		return false;

	ZeroMemory(buffer.GetData(), dwSize);
	if (!::GetFileVersionInfo(pszFile, dwHandle, dwSize, buffer.GetData()))
		return false;

	VS_FIXEDFILEINFO* info = NULL; UINT uint = 0;
	if (!::VerQueryValue(buffer.GetData(), _T("\\"), (LPVOID*)&info, &uint)) 
		return false;

	rInfo = *info;
	return true;
}

CString GetVersion(LPCTSTR pszFile)
{
	try
	{

		VS_FIXEDFILEINFO info;
		ZeroMemory(&info, sizeof(info));
		if (!GetVersion(pszFile, info))
			return CString();

		CString strVersion;
		strVersion.Format( _T("%i.%i.%i.%i"), info.dwFileVersionMS>>16, info.dwFileVersionMS&0xFF, info.dwFileVersionLS>>16, info.dwFileVersionLS&0xFF );
		ATLTRACE( _T("Current File Version: %s %s\n"), pszFile, strVersion );
		return strVersion;
	}
	catch(...)
	{
		ATLTRACE( _T("Exception on GetVersion()\n") );
	}

	return CString();
}

CString GetThisFileVersion()
{
	TCHAR this_file_name[MAX_PATH+1];
	ZeroMemory(this_file_name, sizeof(this_file_name));
	if (!GetModuleFileName(_AtlBaseModule.GetModuleInstance(), this_file_name, MAX_PATH))
		return CString();

    return GetVersion(this_file_name);
}

CString GetProcessVersionInfo(LPCTSTR pszKey)
{
    TCHAR szPath[MAX_PATH] = _T("");
    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
        return CString();

    DWORD dwHandle = 0;
    DWORD dwSize = ::GetFileVersionInfoSize(szPath, &dwHandle);
    if (!dwSize)
        return CString();

    CAtlArray<BYTE> buffer;
    buffer.SetCount(dwSize);
    ZeroMemory(buffer.GetData(), dwSize);

    if (!::GetFileVersionInfo(szPath, dwHandle, dwSize, buffer.GetData()))
        return CString();

    WORD* langInfo = NULL;
    UINT size = 0;

    if (!VerQueryValue(buffer.GetData(), _T("\\VarFileInfo\\Translation"), (LPVOID*)&langInfo, &size) || !size)
        return CString();

    CString sKey = FormatString(_T("\\StringFileInfo\\%04x%04x\\%s"), langInfo[0], langInfo[1], pszKey);

    LPTSTR pValue = NULL;
    if (!::VerQueryValue(buffer.GetData(), sKey, (LPVOID*)&pValue, &size) || !size)
        return CString();

    return pValue;
}

}; //namespace UTIL
