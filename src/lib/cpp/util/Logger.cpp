#include "stdafx.h"
#include "Logger.h"

using namespace UTIL;

CLogger::CLogger(LPCSTR prefix/* = NULL*/)
	: m_bLogEnabled(false)
{
	ZeroMemory(m_prefix, sizeof(m_prefix));
	if (prefix)
	{
		strcpy_s(m_prefix, prefix);
		strcat_s(m_prefix, "\t- ");
	}

	ZeroMemory(m_file, sizeof(m_file));

	Configuration cfg;
	m_bLogEnabled = cfg.log;

#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	if (!IsOSVistaOrHigher())
	{
		GetModuleFileName( _AtlBaseModule.GetModuleInstance(), m_file, MAX_PATH );
		_tcscat_s(m_file, MAX_PATH, _T(".log"));
	}
	else if(SUCCEEDED(::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, m_file)))
	{
		_tcscat_s(m_file, _T("Low"));

#ifndef _PROJECT_NAME
#error Please set _PROJECT_NAME to Project Name
#endif

		_tcscat_s(m_file, _T("\\"));
		_tcscat_s(m_file, _PROJECT_NAME);

		static bool bCreate = true;
		if (bCreate) { ::CreateDirectory(m_file, NULL); bCreate = false; }

		_tcscat_s(m_file, _T("\\"));
		_tcscat_s(m_file, _PROJECT_NAME);
		_tcscat_s(m_file, _T(".log"));
	}

	ATLTRACE( _T("Log file: %s. Log %s\n"), m_file, m_bLogEnabled ? _T("ENABLED") : _T("DISABLED") );
}

void CLogger::log(LPCSTR format, DWORD dwValue) const
{
	ATLASSERT(format);

#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	ATLASSERT(format);

	CHAR buffer[513];
	ZeroMemory(buffer, sizeof(buffer));
	sprintf_s(buffer, 512, format, dwValue);
	log(buffer);
}

void CLogger::log(LPCSTR format, DWORD dwValue, DWORD dwValue2) const
{
	ATLASSERT(format);

#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	ATLASSERT(format);

	CHAR buffer[513];
	ZeroMemory(buffer, sizeof(buffer));
	sprintf_s(buffer, 512, format, dwValue, dwValue2);
	log(buffer);
}

void CLogger::log(LPCOLESTR format, DWORD dwValue) const
{
	ATLASSERT(format);

#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	ATLASSERT(format);

	WCHAR buffer[513];
	ZeroMemory(buffer, sizeof(buffer));
	swprintf_s(buffer, 512, format, dwValue);
	log(buffer);
}

void CLogger::log(LPCSTR message, LPCSTR text) const
{
	ATLASSERT(message || text);
	ATLTRACE("%s%s%s\n", *m_prefix ? m_prefix : "", message ? message : "", text ? text : "");

#ifdef _USE_LOGGING
	if (!m_bLogEnabled)
		return;

	if (!message && !text)
		return;

	File f;
	for (int iTry = 0; iTry < 10; ++iTry)
	{
		if (f.Create(m_file, GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS))
			break;

		::Sleep(50);
	}

	if (!f.IsValid())
	{
		ATLTRACE("Cannot open log file %s", m_file);
		return;
	}

	f.SeekToEnd();

	if (*m_prefix) f.Write(m_prefix, strlen(m_prefix));
	if (message) f.Write(message, strlen(message));
	if (text)	 f.Write(text, strlen(text));

	f.Write("\r\n", 2);
#endif
}

void CLogger::log(LPCSTR message, LPBYTE lpData, DWORD dwCount) const
{
	ATLASSERT(message || lpData);
	ATLTRACE("%s%s[binary data]\n", *m_prefix ? m_prefix : "", message ? message : "");

#ifdef _USE_LOGGING
	if (!m_bLogEnabled)
		return;

	if (!message && !lpData)
		return;

	File f;
	for (int iTry = 0; iTry < 10; ++iTry)
	{
		if (f.Create(m_file, GENERIC_WRITE, FILE_SHARE_READ, OPEN_ALWAYS))
			break;

		::Sleep(50);
	}

	if (!f.IsValid())
	{
		ATLTRACE("Cannot open log file %s", m_file);
		return;
	}

	f.SeekToEnd();

	if (*m_prefix) f.Write(m_prefix, strlen(m_prefix));
	if (message) f.Write(message, strlen(message));

    if (*m_prefix || message) f.Write("\r\n", 2);
	if (lpData)	 f.Write(lpData, dwCount);

	f.Write("\r\n", 2);
#endif
}
