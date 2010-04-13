// RegKey.h: Registry API.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Handle.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// RegKey
//////////////////////////////////////////////////////////////////////

class RegKey : public THandle<HKEY>
{
public:
	RegKey(void)
	{
	}

	RegKey(HKEY hKey, LPCTSTR lpSubKey = NULL)
	{
		Create(hKey, lpSubKey);
	}

	virtual ~RegKey(void)
	{
		Close();
	}

public:

	// Implements Closable
	virtual void Close()
	{ 
		if (IsValid()) ::RegCloseKey(handle), handle = NULL; 
	}

public:
	BOOL Create(HKEY hKey, LPCTSTR lpSubKey = NULL)
	{
		HKEY hResultKey = NULL;

		if (ERROR_SUCCESS != ::RegCreateKey(hKey, lpSubKey, &hResultKey))
			Close();
		else
			Attach(hResultKey);

		return IsValid();
	}

	BOOL CreateEx(HKEY hKey, LPCTSTR lpSubKey, LPTSTR lpClass = NULL, DWORD dwOptions = REG_OPTION_NON_VOLATILE, REGSAM samDesired = KEY_ALL_ACCESS, LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL, LPDWORD lpdwDisposition = NULL)
	{
		HKEY hResultKey = NULL;

		if (ERROR_SUCCESS != ::RegCreateKeyEx(hKey, lpSubKey, 0, lpClass, dwOptions, samDesired, lpSecurityAttributes, &hResultKey, lpdwDisposition))
			Close();
		else
			Attach(hResultKey);

		return IsValid();
	}

	BOOL OpenEx(HKEY hKey, LPCTSTR lpSubKey, REGSAM samDesired = KEY_ALL_ACCESS)
	{
		HKEY hResultKey = NULL;

		if (ERROR_SUCCESS != ::RegOpenKeyEx(hKey, lpSubKey, 0, samDesired, &hResultKey))
			Close();
		else
			Attach(hResultKey);

		return IsValid();
	}

	LONG QueryValueEx(LPCTSTR lpValueName, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) const
	{
		ATLASSERT(IsValid());

		return ::RegQueryValueEx(handle, lpValueName, NULL, lpType, lpData, lpcbData);
	}

	LONG SetValueEx(LPCTSTR lpValueName, DWORD dwType, const BYTE* lpData, DWORD cbData)
	{
		ATLASSERT(IsValid());

		return ::RegSetValueEx(handle, lpValueName, 0, dwType, lpData, cbData);
	}

	LONG QueryValueSize(LPCTSTR lpValueName, LPDWORD lpcbData, LPDWORD lpType = NULL) const
	{
		ATLASSERT(IsValid());

		return ::RegQueryValueEx(handle, lpValueName, NULL, lpType, NULL, lpcbData);
	}

	BOOL QueryValue(LPCTSTR lpValueName, DWORD &dwValue) const
	{
		ATLASSERT(IsValid());

		dwValue = 0;

		DWORD dwSize = 0, dwType = 0;
		if (ERROR_SUCCESS != QueryValueSize(lpValueName, &dwSize, &dwType))
			return FALSE;

		if (!dwSize || dwType != REG_DWORD)
			return FALSE;

		return ERROR_SUCCESS == QueryValueEx(lpValueName, &dwType, reinterpret_cast<BYTE*>(&dwValue), &dwSize);
	}

	BOOL QueryValue(LPCTSTR lpValueName, bool &bValue) const
	{
		DWORD dwValue = 0;
		if (!QueryValue(lpValueName, dwValue))
			return FALSE;

		bValue = (TRUE == dwValue);
		return TRUE;
	}

	BOOL QueryValue(LPCTSTR lpValueName, LONGLONG &llValue) const
	{
		ATLASSERT(IsValid());

		llValue = 0;

		DWORD dwSize = 0, dwType = 0;
		if (ERROR_SUCCESS != QueryValueSize(lpValueName, &dwSize, &dwType))
			return FALSE;

		if (!dwSize || dwType != REG_QWORD)
			return FALSE;

		return ERROR_SUCCESS == QueryValueEx(lpValueName, &dwType, reinterpret_cast<BYTE*>(&llValue), &dwSize);
	}

	BOOL QueryValue(LPCSTR lpValueName, CStringA &sValue) const
	{
		ATLASSERT(IsValid());

		sValue.Empty();

		DWORD dwSize = 0, dwType = REG_SZ;
		if (ERROR_SUCCESS != ::RegQueryValueExA(handle, lpValueName, NULL, &dwType, NULL, &dwSize))
			return FALSE;

		// TODO: check dwType and convert to string

		if (!dwSize)
			return TRUE;

		// Reading...
		char* buf = new char[dwSize];
		ZeroMemory(buf, dwSize);

		LONG lResult = ::RegQueryValueExA(handle, lpValueName, NULL, &dwType, reinterpret_cast<BYTE*>(buf), &dwSize);
		if (ERROR_SUCCESS == lResult)
			sValue = buf;

		delete [] buf, buf = NULL;

		return ERROR_SUCCESS == lResult;
	}

	BOOL QueryValue(LPCWSTR lpValueName, CStringW &sValue) const
	{
		ATLASSERT(IsValid());

		sValue.Empty();

		DWORD dwSize = 0, dwType = REG_SZ;
		if (ERROR_SUCCESS != ::RegQueryValueExW(handle, lpValueName, NULL, &dwType, NULL, &dwSize))
			return FALSE;

		// TODO: check dwType and convert to string

		if (!dwSize)
			return TRUE;

		// Reading...
		char* buf = new char[dwSize];
		ZeroMemory(buf, dwSize);

		LONG lResult = ::RegQueryValueExW(handle, lpValueName, NULL, &dwType, reinterpret_cast<BYTE*>(buf), &dwSize);
		if (ERROR_SUCCESS == lResult)
			sValue = (LPCWSTR)buf;

		delete [] buf, buf = NULL;

		return ERROR_SUCCESS == lResult;
	}

	BOOL QueryValue(LPCOLESTR lpValueName, CComBSTR &bsValue) const
	{
		ATLASSERT(IsValid());

		bsValue.Empty();

		DWORD dwSize = 0, dwType = REG_SZ;
		if (ERROR_SUCCESS != ::RegQueryValueExW(handle, lpValueName, NULL, &dwType, NULL, &dwSize))
			return FALSE;

		// TODO: check dwType and convert to string

		if (!dwSize)
			return TRUE;

		// Reading...
		
		CComBSTR buf(dwSize/sizeof(OLECHAR) - 1);
		ZeroMemory(buf, dwSize);

		LONG lResult = ::RegQueryValueExW(handle, lpValueName, NULL, &dwType, (BYTE*)(BSTR)buf, &dwSize);
		if (ERROR_SUCCESS == lResult)
			bsValue.Attach(buf.Detach());

		return ERROR_SUCCESS == lResult;
	}

	LONG SetValue(LPCTSTR lpValueName, const CStringA &sValue)
	{
		return SetValueEx(lpValueName, REG_SZ, (BYTE*)(LPCSTR)sValue, ((DWORD)sValue.GetLength())+1);
	}

	LONG SetValue(LPCOLESTR lpValueName, const CStringW &wsValue)
	{
		ATLASSERT(IsValid());

		return ::RegSetValueExW(handle, lpValueName, 0, REG_SZ, (BYTE*)(LPCOLESTR)wsValue, ((DWORD)wsValue.GetLength()+1)*sizeof(WCHAR));
	}

	LONG SetValue(LPCOLESTR lpValueName, BSTR bsValue)
	{
		ATLASSERT(IsValid());

		return ::RegSetValueExW(handle, lpValueName, 0, REG_SZ, (BYTE*)bsValue, ((DWORD)::SysStringLen(bsValue) + 1)*sizeof(OLECHAR));
	}

	LONG SetValue(LPCTSTR lpValueName, DWORD dwValue)
	{
		return SetValueEx(lpValueName, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
	}

	LONG SetValue(LPCTSTR lpValueName, bool bValue)
	{
		return SetValue(lpValueName, (DWORD)(bValue ? TRUE : FALSE));
	}

	LONG SetValue(LPCTSTR lpValueName, LONGLONG llValue)
	{
		return SetValueEx(lpValueName, REG_QWORD, (BYTE*)&llValue, sizeof(llValue));
	}

	LONG DeleteValue(LPCSTR lpValueName)
	{
		ATLASSERT(IsValid());

		return ::RegDeleteValueA(handle, lpValueName);
	}

	LONG DeleteValue(LPCOLESTR lpValueName)
	{
		ATLASSERT(IsValid());

		return ::RegDeleteValueW(handle, lpValueName);
	}

	DWORD DeleteKey(LPCTSTR lpSubKey)
	{
		ATLASSERT(IsValid());

		return ::SHDeleteKey(handle, lpSubKey);
	}

	LONG EnumValue(DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcValueName, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
	{
		ATLASSERT(IsValid());
		return RegEnumValue(handle, dwIndex, lpValueName, lpcValueName, NULL, lpType, lpData, lpcbData);
	}

	BOOL IsKeyEmpty() const
	{
		ATLASSERT(IsValid());

		TCHAR szBuf[255];
		DWORD dwBufSize = sizeof(szBuf)/sizeof(TCHAR);
		FILETIME ftTemp;
		return ERROR_NO_MORE_ITEMS == RegEnumKeyEx(handle, 0, szBuf, &dwBufSize, NULL, NULL, NULL, &ftTemp);
	}

	BOOL IsValueSet(LPCTSTR lpValueName) const
	{
		DWORD cbData = 0;
		return ERROR_SUCCESS == QueryValueSize(lpValueName, &cbData) && cbData > 0;
	}
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
