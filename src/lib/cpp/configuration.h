#pragma once

#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PROPERTY_STRING(name, def_value) \
	__declspec(property(get = get_##name, put = put_##name)) CString name;\
	CString get_##name() const { ATLASSERT(key.IsValid());  CString value; if (!key.QueryValue( _T(#name), value )) return def_value; return value; } \
	void put_##name(const CString &value) { ATLASSERT(key.IsValid()); key.SetValue( _T(#name), value ); } 

#define PROPERTY_DWORD(name, def_value) \
	__declspec(property(get = get_##name, put = put_##name)) DWORD name;\
	DWORD get_##name() const { ATLASSERT(key.IsValid());  DWORD value; if (!key.QueryValue( _T(#name), value )) return def_value; return value; } \
	void put_##name(DWORD value) { ATLASSERT(key.IsValid()); key.SetValue( _T(#name), value ); } 

#define PROPERTY_BOOL(name, def_value) \
	__declspec(property(get = get_##name, put = put_##name)) bool name;\
	bool get_##name() const { ATLASSERT(key.IsValid());  bool value; if (!key.QueryValue( _T(#name), value )) return def_value; return value; } \
	void put_##name(bool value) { ATLASSERT(key.IsValid()); key.SetValue( _T(#name), value ); } 

#define PROPERTY_LONGLONG(name, def_value) \
	__declspec(property(get = get_##name, put = put_##name)) LONGLONG name;\
	LONGLONG get_##name() const { ATLASSERT(key.IsValid());  LONGLONG value; if (!key.QueryValue( _T(#name), value )) return def_value; return value; } \
	void put_##name(LONGLONG value) { ATLASSERT(key.IsValid()); key.SetValue( _T(#name), value ); } 

#define RESOURCE_STRING(name, RESOURCE_ID) \
	__declspec(property(get = get_##name)) CString name;\
	CString get_##name() const { return GetString(RESOURCE_ID); } \

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration
//

class Configuration
{
private:
	RegKey key;

public:
	Configuration(void)
	{
		CString strKey;

		if (IsOSVistaOrHigher())
			strKey = _T("Software\\Microsoft\\Internet Explorer\\LowRegistry\\") _PROJECT_REGKEY;
		else
			strKey = _T("Software\\") _PROJECT_REGKEY;

        CString strProductName = GetProcessVersionInfo(_T("ProductName"));
        if (!strProductName.IsEmpty())
            strKey += (CString)_T("\\") + strProductName;
        else
        {
	        TCHAR szExeFileName[MAX_PATH+1] = _T("");
	        ZeroMemory(szExeFileName, sizeof(szExeFileName));
	        GetModuleFileName(NULL, szExeFileName, MAX_PATH);

            strKey += (CString)_T("\\") + ExtractOnlyFileName(szExeFileName);
        }

		key.CreateEx(HKEY_CURRENT_USER, strKey);
	}

	// Write logs
	PROPERTY_BOOL(log, false)
	PROPERTY_STRING(lastUserName, _T(""))

	RESOURCE_STRING(urlLoginAPI, IDS_LOGIN_API_URL)
	RESOURCE_STRING(urlMyImagesPage, IDS_MY_IMAGES_URL)
	RESOURCE_STRING(urlMyVideosPage, IDS_MY_VIDEOS_URL)
	RESOURCE_STRING(urlForgotPassword, IDS_FORGOT_PASSWORD)
	// Force Server
	PROPERTY_STRING(ForceServer, _T(""))
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
