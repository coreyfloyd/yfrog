#pragma once

namespace UTIL {

/**
 * Logger implementation.
 *
 * @author Alexander Kozlov
 */
class CLogger
{
public:
	CLogger(LPCSTR prefix = NULL);

	void log(LPCSTR message) const;
	void log(LPCOLESTR message) const;
	void log(LPCSTR format, DWORD dwValue) const;
	void log(LPCSTR format, DWORD dwValue, DWORD dwValue2) const;
	void log(LPCOLESTR format, DWORD dwValue) const;
	void log(LPCSTR message, LPCSTR text) const;
	void log(LPCSTR message, LPCOLESTR text) const;
	void log(LPCOLESTR message, LPCOLESTR text) const;

	void log(LPCSTR format, void* pValue) const
	{
		log(format, (DWORD)pValue);
	}

    void log(LPCSTR message, LPBYTE lpData, DWORD dwCount) const;

	bool IsLogEnabled() const
	{
		return m_bLogEnabled;
	}

private:
	char  m_prefix[64];
	bool  m_bLogEnabled;
	TCHAR m_file[MAX_PATH+64];
};

inline void CLogger::log(LPCSTR message) const
{
	ATLASSERT(message);

	log(message, (LPCSTR)NULL);
}

inline void CLogger::log(LPCOLESTR message) const
{
#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	ATLASSERT(message);

	CW2A pszMessage(message);
	log(pszMessage, (LPCSTR)NULL);
}

inline void CLogger::log(LPCSTR message, LPCOLESTR text) const
{
#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	if (!text)
		log(message);
	else
	{
		ATLASSERT(message);

		CW2A pszText(text);
		log(message, pszText);
	}
}

inline void CLogger::log(LPCOLESTR message, LPCOLESTR text) const
{
#ifndef _DEBUG
	// In DEBUG mode always format and call to log(..) to TRACE
	if (!m_bLogEnabled)
		return;
#endif

	if (!text)
		log(message);
	else
	{
		ATLASSERT(message);

		CW2A pszMessage(message), pszText(text);
		log(pszMessage, pszText);
	}
}

#ifdef _USE_LOGGING
	#define LOG logger.log
#else
	#define LOG __noop
#endif

#define DEFINE_LOGGER(class_name) CLogger class_name::logger(#class_name);

}; //namespace UTIL
