#pragma once

#include "HttpRequest.h"

namespace HTTP {

/**
 * Simple HTTP connection implementation.
 *
 * @author Alexander Kozlov
 */
class Connection
{
public:
	Connection();

	bool Open(LPCTSTR pszURL);
	bool Send(LPCTSTR pszURL, LPCTSTR pszHeaders, const char* pszBody);
	bool Send(LPCTSTR pszURL, const MultipartFormDataRequest& request);
	bool GetResponse(CStringA& response);
	bool GetResponse(ByteStream& response);

	void Close();

	BOOL operator ! () const;

	~Connection();

protected:
	bool Connect(LPCTSTR pszURL, LPCTSTR pszMethod);

private:
	DWORD				m_context; 

	Internet			m_internet;
	InternetConnection	m_connection;
	InternetHttpRequest m_request;

private:
	static CLogger logger;
};

} // namespace HTTP
