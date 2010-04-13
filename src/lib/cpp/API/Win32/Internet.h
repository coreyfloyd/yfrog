// Internet.h: Internet class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Handle.h"
#include <wininet.h>

namespace API {
namespace Win32 {

#define METHOD_GET		_T("GET")
#define METHOD_POST		_T("POST")
#define METHOD_HEAD		_T("HEAD")

//////////////////////////////////////////////////////////////////////
// InternetHandle
//////////////////////////////////////////////////////////////////////

class InternetHandle : public THandle<HINTERNET>
{
public:
	InternetHandle(void)
	{
	}

	virtual ~InternetHandle(void)
	{
		Close();
	}

	// Implements Closable
	virtual void Close()
	{ 
		if (IsValid()) ::InternetCloseHandle(handle), handle = NULL; 
	}
};


//////////////////////////////////////////////////////////////////////
// TOptionable
//////////////////////////////////////////////////////////////////////

template <class TOptionableHandler>
class TOptionable : virtual public TOptionableHandler
{
public:
	BOOL QueryOption(DWORD dwOption, LPVOID lpBuffer, LPDWORD lpdwBufferLength)
	{
		return ::InternetQueryOption( handle, dwOption, lpBuffer, lpdwBufferLength );
	}

	BOOL SetOption(DWORD dwOption, LPVOID lpBuffer, DWORD dwBufferLength)
	{
		return ::InternetSetOption(handle, dwOption, lpBuffer, dwBufferLength);
	}
};

//////////////////////////////////////////////////////////////////////
// Internet
//////////////////////////////////////////////////////////////////////

class Internet : public TOptionable<InternetHandle>
{
public:
	Internet(void)
	{
	}

public:
	BOOL Open(LPCTSTR lpszAgent, DWORD dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG, LPCTSTR lpszProxyName = NULL, LPCTSTR lpszProxyBypass = NULL, DWORD dwFlags = 0)
	{
		HINTERNET hInet = ::InternetOpen(lpszAgent, dwAccessType, lpszProxyName, lpszProxyBypass, dwFlags);

		Attach(hInet);

		return IsValid();
	}

	INTERNET_STATUS_CALLBACK SetStatusCallback(INTERNET_STATUS_CALLBACK lpfnInternetCallback)
	{
		return InternetSetStatusCallback(handle, lpfnInternetCallback);
	}

public:
	// static members
	static BOOL AttemptConnect()
	{
		return ERROR_SUCCESS == ::InternetAttemptConnect(0);
	}

	static BOOL CheckConnection(LPCTSTR lpszUrl = NULL,DWORD dwFlags = FLAG_ICC_FORCE_CONNECTION)
	{
		return ::InternetCheckConnection(lpszUrl, dwFlags, 0);
	}

	static CString GetLastResponseInfo(LPDWORD lpdwError = NULL)
	{
		DWORD dwError;
		if (!lpdwError) lpdwError = &dwError;

		CString strBuffer;
		DWORD   dwBufferLength;
		if (InternetGetLastResponseInfo(lpdwError, strBuffer.GetBuffer(dwBufferLength = 512), &dwBufferLength))
		{
			strBuffer.ReleaseBuffer(dwBufferLength);
			return strBuffer;
		}

		strBuffer.ReleaseBuffer(0);

		if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
			return CString();

		if (!InternetGetLastResponseInfo(lpdwError, strBuffer.GetBuffer(dwBufferLength), &dwBufferLength))
			return CString();

		strBuffer.ReleaseBuffer(dwBufferLength);
		return strBuffer;
	}

	static BOOL GetCookie(LPCTSTR pchURL, LPTSTR pchCookieData, LPDWORD pcchCookieData)
	{
		return ::InternetGetCookie(pchURL,NULL/*not implemented*/,pchCookieData,pcchCookieData);
	}
};

//////////////////////////////////////////////////////////////////////
// InternetConnection
//////////////////////////////////////////////////////////////////////

class InternetConnection : public InternetHandle
{
public:
	InternetConnection(void)
	{
	}

public:
	BOOL Connect(HINTERNET hInternet, LPCTSTR lpszServerName, INTERNET_PORT nServerPort, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext, LPCTSTR lpszUsername = NULL, LPCTSTR lpszPassword = NULL)
	{
		return Connect(hInternet,lpszServerName,nServerPort,lpszUsername,lpszPassword,dwService,dwFlags,dwContext);
	}

	BOOL Connect(HINTERNET hInternet, LPCTSTR lpszServerName, INTERNET_PORT nServerPort, LPCTSTR lpszUsername, LPCTSTR lpszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext)
	{
		HINTERNET hConnection = ::InternetConnect(hInternet,lpszServerName,nServerPort,lpszUsername,lpszPassword,dwService,dwFlags,dwContext);

		Attach(hConnection);

		return IsValid();
	}
};

//////////////////////////////////////////////////////////////////////
// TReadable
//////////////////////////////////////////////////////////////////////

template <class TReadableHandler>
class TReadable : virtual public TReadableHandler
{
public:
	BOOL Read(LPVOID lpBuffer, DWORD dwNumberOfBytesToRead, LPDWORD lpdwNumberOfBytesRead)
	{
		ATLASSERT(IsValid());
		return ::InternetReadFile(handle, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
	}

	BOOL ReadExA(LPINTERNET_BUFFERSA lpBuffersOut, DWORD dwFlags, DWORD_PTR dwContext)
	{
		ATLASSERT(IsValid());
		return ::InternetReadFileExA(handle,lpBuffersOut,dwFlags,dwContext);
	}
};

//////////////////////////////////////////////////////////////////////
// TReadable
//////////////////////////////////////////////////////////////////////

template <class TWritableHandler>
class TWritable : virtual public TWritableHandler
{
public:
	BOOL Write(LPCVOID lpBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD lpdwNumberOfBytesWritten)
	{
		ATLASSERT(IsValid());
		return ::InternetWriteFile(handle, lpBuffer, dwNumberOfBytesToWrite, lpdwNumberOfBytesWritten);
	}
};

//////////////////////////////////////////////////////////////////////
// THttpQueryInfo
//////////////////////////////////////////////////////////////////////

template <class TQueryInfoHandler>
class THttpQueryInfo : virtual public TQueryInfoHandler
{
public:
	BOOL QueryInfo(DWORD dwInfoLevel, LPVOID lpvBuffer, LPDWORD lpdwBufferLength, LPDWORD lpdwIndex = NULL)
	{
		return ::HttpQueryInfo(handle, dwInfoLevel, lpvBuffer, lpdwBufferLength, lpdwIndex);
	}
};

//////////////////////////////////////////////////////////////////////
// InternetHttpRequest
//////////////////////////////////////////////////////////////////////

class InternetHttpRequest : public TReadable<InternetHandle>, public TWritable<InternetHandle>, public THttpQueryInfo<InternetHandle>
{
public:
	InternetHttpRequest(void)
	{
	}

public:
	BOOL Open(HINTERNET hConnect, LPCTSTR lpszVerb, LPCTSTR lpszObjectName, DWORD dwFlags, DWORD_PTR dwContext, LPCTSTR* lpszAcceptTypes = NULL, LPCTSTR lpszReferer = NULL, LPCTSTR lpszVersion = NULL)
	{
		return Open(hConnect,lpszVerb,lpszObjectName,lpszVersion,lpszReferer,lpszAcceptTypes,dwFlags,dwContext);
	}

	BOOL Open(HINTERNET hConnect, LPCTSTR lpszVerb, LPCTSTR lpszObjectName, LPCTSTR lpszVersion, LPCTSTR lpszReferer, LPCTSTR* lpszAcceptTypes, DWORD dwFlags, DWORD_PTR dwContext)
	{
		HINTERNET hRequest = ::HttpOpenRequest(hConnect,lpszVerb,lpszObjectName,lpszVersion,lpszReferer,lpszAcceptTypes,dwFlags,dwContext);

		Attach(hRequest);

		return IsValid();
	}

	BOOL SendRequest(LPCTSTR lpszHeaders = NULL, DWORD dwHeadersLength = -1, LPVOID lpOptional = NULL, DWORD dwOptionalLength = 0)
	{
		return ::HttpSendRequest(handle, lpszHeaders, lpszHeaders == NULL ? 0 : dwHeadersLength, lpOptional, lpOptional == NULL ? 0 : dwOptionalLength);
	}

	BOOL SendRequestEx(LPINTERNET_BUFFERS lpBuffersIn, LPINTERNET_BUFFERS lpBuffersOut, DWORD_PTR dwContext)
	{
		return ::HttpSendRequestEx(handle, lpBuffersIn, lpBuffersOut, 0, dwContext);
	}

	BOOL EndRequest(DWORD_PTR dwContext = 0)
	{
		return ::HttpEndRequest(handle,NULL,HSR_INITIATE,dwContext);
	}
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
