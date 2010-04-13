#include "StdAfx.h"
#include ".\connection.h"

namespace HTTP {

DEFINE_LOGGER(Connection)

Connection::Connection()
	: m_context(::GetTickCount())
{
}

Connection::~Connection()
{
	Close();
}

bool Connection::Connect(LPCTSTR pszURL, LPCTSTR pszMethod)
{
	LOG("URL: ", pszURL);

    if (!m_internet.Open(HTTP_USER_AGENT))
	{
		LOG("Cannot open internet connection");
		return false;
	}

	CUrl url;
	if (!url.CrackUrl(pszURL))
	{
		LOG("Invalid URL", pszURL);
		return false;
	}

	if (!m_connection.Connect(m_internet, url.GetHostName(), url.GetPortNumber(), INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)&m_context))
	{
		LOG("Cannot connect to server");
		return false;
	}

    TCHAR szPath[2*ATL_URL_MAX_PATH_LENGTH+1];
    _tcscpy_s(szPath, url.GetUrlPath());
	if (url.GetExtraInfoLength())
        _tcscat_s(szPath, url.GetExtraInfo());

	if (!m_request.Open(m_connection, pszMethod, szPath, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE,(DWORD_PTR)&m_context))
	{
		LOG("Cannot open HTTP connection");
		return false;
	}

	return true;
}

BOOL Connection::operator ! () const
{
	return m_request.operator ! ();
}

bool Connection::Open(LPCTSTR pszURL)
{
	if (!Connect(pszURL, METHOD_GET))
		return false;

	if (!m_request.SendRequest())
	{
		// problem while send request
		LOG("Cannot open GET stream");
		return false;
	}

	return true;
}

bool Connection::Send(LPCTSTR pszURL, LPCTSTR pszHeaders, const char* pszBody)
{
	if (!Connect(pszURL, METHOD_POST))
		return false;

	if (!m_request.SendRequest(pszHeaders, _tcslen(pszHeaders), (LPVOID)(LPCSTR)pszBody, strlen(pszBody)))
	{
		// problem while send request
		LOG("Cannot send request");
		return false;
	}

	return true;
}

bool Connection::Send(LPCTSTR pszURL, const MultipartFormDataRequest& request)
{
	if (!Connect(pszURL, METHOD_POST))
		return false;

	LPCTSTR pszHeaders = request.GetHead();
	const ByteStream &body = request.GetBody();

	if (!m_request.SendRequest(pszHeaders, _tcslen(pszHeaders), body.GetData(), body.GetLength()))
	{
		// problem while send request
		LOG("Cannot send request");
		return false;
	}

	return true;
}

bool Connection::GetResponse(CStringA& response)
{
	DWORD dwCode = HTTP_STATUS_OK, dwBufSize = sizeof(DWORD);
	if (m_request.QueryInfo(HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &dwCode, &dwBufSize) && HTTP_STATUS_OK != dwCode)
	{
		// problem while send request
		LOG("Invalid status: %i", dwCode);
		return false;
	}

	response.Empty();

	// read response portion by portion
	const DWORD portion = 1024;
	char szBuffer[portion+1];
	while (true)
	{
		ZeroMemory(szBuffer, sizeof(szBuffer));

		DWORD bytesread = 0;
		if (!m_request.Read(szBuffer, portion, &bytesread))
		{
			// problem while reading response
			LOG("Cannot read data");
			return false;
		}

		if (0 == bytesread) // response was completely read
			break;

		szBuffer[bytesread] = 0;
		response += szBuffer;
	}

	LOG("Response: ", response);

	return true;
}

bool Connection::GetResponse(ByteStream& response)
{
	DWORD dwCode = HTTP_STATUS_OK, dwBufSize = sizeof(DWORD);
	if (m_request.QueryInfo(HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &dwCode, &dwBufSize) && HTTP_STATUS_OK != dwCode)
	{
		// problem while send request
		LOG("Invalid status: %i", dwCode);
		return false;
	}

	response.Empty();

	// read response portion by portion
	const DWORD portion = 1024;
	BYTE buffer[portion];
	while (true)
	{
		ZeroMemory(buffer, sizeof(buffer));

		DWORD bytesread = 0;
		if (!m_request.Read(buffer, portion, &bytesread))
		{
			// problem while reading response
			LOG("Cannot read data");
			return false;
		}

		if (0 == bytesread) // response was completely read
			break;

        response.Write(buffer, bytesread);
	}

	LOG("Response: ", response.GetData(), response.GetLength());

	return true;
}

void Connection::Close()
{ 
	m_request.Close();
	m_connection.Close();
	m_internet.Close();
}

} // namespace HTTP
