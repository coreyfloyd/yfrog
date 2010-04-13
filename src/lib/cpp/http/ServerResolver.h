#pragma once

namespace ImageShack {

// Count of domains loadXX.imageshack.us & vidXX.imageshack.us
#define IMAGESHACK_LOAD_COUNT   10
// Count of domains render.imageshack.us
#define IMAGESHACK_RENDER_COUNT	4

/**
 * URL Resolver to distribute load.
 *
 * @author Alexander Kozlov
 */
class CServerResolver
{
private:
	CStringW m_strServer, m_strServer2;
	CStringW m_strBaseURL;
	CStringW m_strFormat;
    UINT     m_nServerCount;

	// if true than original URL will be returned by GetURL()
	mutable bool m_bFirst;
	mutable size_t m_nLastIndex;
	mutable CAtlArray<CStringW> m_arServers;

public:
	CServerResolver(UINT nBaseURL, UINT nServerURL, UINT nFormatURL, UINT nServerCount);
	CServerResolver(UINT nBaseURL, UINT nServerURL, UINT nServerURL2, UINT nFormatURL, UINT nServerCount);
	CStringW GetURL() const;
	CStringW GetRandomURL() const;

private:
	void CrackBaseURL();
	void InitServerList() const;
};

}//namespace ImageShack

