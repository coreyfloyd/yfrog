#include "stdafx.h"
#include "ServerResolver.h"

#include <time.h>

using namespace ImageShack;

CServerResolver::CServerResolver(UINT nBaseURL, UINT nServerURL, UINT nFormatURL, UINT nServerCount)
	: m_bFirst(true)
	, m_nLastIndex((size_t)-1)
    , m_nServerCount(nServerCount)
{
	m_strBaseURL.LoadString(nBaseURL);
	m_strServer.LoadString(nServerURL);
    m_strFormat.LoadString(nFormatURL);

	ATLASSERT(m_strBaseURL.Find(m_strServer) != -1);

	CrackBaseURL();
}

CServerResolver::CServerResolver(UINT nBaseURL, UINT nServerURL, UINT nServerURL2, UINT nFormatURL, UINT nServerCount)
	: m_bFirst(true)
	, m_nLastIndex((size_t)-1)
    , m_nServerCount(nServerCount)
{
	m_strBaseURL.LoadString(nBaseURL);
	m_strServer.LoadString(nServerURL);
	m_strServer2.LoadString(nServerURL2);
    m_strFormat.LoadString(nFormatURL);

	ATLASSERT(m_strBaseURL.Find(m_strServer) != -1);
	ATLASSERT(m_strBaseURL.Find(m_strServer2) != -1);

	CrackBaseURL();
}

void CServerResolver::CrackBaseURL()
{
	CString strForceServer = Configuration().ForceServer;
	if (strForceServer.IsEmpty())
		return;

	m_strBaseURL.Replace(m_strServer, (CStringW)strForceServer);
	ATLTRACE( _T("Cracked URL: %s\n"), m_strBaseURL );
}

CStringW CServerResolver::GetURL() const
{
	if (!m_bFirst)
		return GetRandomURL();

	m_bFirst = false;
	return m_strBaseURL;
}

CStringW CServerResolver::GetRandomURL() const
{
	if (!m_arServers.GetCount())
		InitServerList();

	if (1 == m_arServers.GetCount())
		return m_strBaseURL;

	size_t nIndex, nRetryCount = m_arServers.GetCount();
	do 
	{
		nIndex = abs(rand()) % m_arServers.GetCount();
	} while(m_nLastIndex == nIndex && nRetryCount-- > 0);
	m_nLastIndex = nIndex;

	CStringW strServer(m_strBaseURL);
	strServer.Replace(m_strServer, m_arServers[nIndex]);
	ATLTRACE( _T("Server URL: %s\n"), (CString)strServer );
	return strServer;
}

void CServerResolver::InitServerList() const
{
	srand( (unsigned)time( NULL ) );

    if (m_strFormat.IsEmpty())
    {
		if (!m_strServer.IsEmpty()) m_arServers.Add(m_strServer);
		if (!m_strServer2.IsEmpty()) m_arServers.Add(m_strServer2);
    }
    else
	{
		m_arServers.SetCount(m_nServerCount);
		for (size_t i = 1; i <= m_nServerCount; ++i)
			m_arServers[i-1].Format(m_strFormat, i);
	}
}
