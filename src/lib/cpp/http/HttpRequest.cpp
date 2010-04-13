#include "stdafx.h"
#include ".\httprequest.h"
using namespace API::Win32;

using namespace HTTP;

const char* CRLF = "\r\n";
const TCHAR* T_CRLF = _T("\r\n");
const char* CONTENT_TYPE = "Content-Type: ";
const char* DEF_CONTENT_TYPE = "application/octet-stream";
const char* CONTENT_TRANSFER_ENCODING = "Content-Transfer-Encoding: binary";

MultipartFormDataRequest::MultipartFormDataRequest()
{
    m_dwBoundaryRand1 = GetTickCount();
    m_dwBoundaryRand2 = rand();

	Reset();
}

void MultipartFormDataRequest::Reset()
{
	// init tail of request
    sprintf(m_szTailA, "--B-O-U-N-D-A-R-Y%u%u--\r\n", m_dwBoundaryRand1, m_dwBoundaryRand2);

	m_content.Empty();
	m_content << m_szTailA;
}

void MultipartFormDataRequest::AddKey(const char* key, const char* pszFileName, const char * content_type/* = NULL*/)
{
	m_content.SetLength(m_content.GetLength() - 4 /* "--" + CRLF */);
	// "--" + m_boundary - already added;
	m_content << CRLF;
	m_content << "Content-Disposition: form-data; name=\"" << key << "\"";
	if (pszFileName && *pszFileName)
	{
		m_content << "; filename=\"" << pszFileName << "\"" << CRLF;
		m_content << CONTENT_TYPE << (content_type ? content_type : DEF_CONTENT_TYPE) << CRLF;
		m_content << CONTENT_TRANSFER_ENCODING;
	}
	m_content << CRLF << CRLF;
}

MultipartFormDataRequest& MultipartFormDataRequest::AddKeyValue(const char* key, const char* value)
{
	AddKey(key);
	m_content << value;
	m_content << CRLF;
	m_content << m_szTailA;
	return *this;
}

MultipartFormDataRequest& MultipartFormDataRequest::AddKeyValue(const char* key, int value)
{
	AddKey(key);
	m_content << value;
	m_content << CRLF;
	m_content << m_szTailA;
	return *this;
}

MultipartFormDataRequest& MultipartFormDataRequest::AddKeyValue(const char* key, ULONGLONG value)
{
	AddKey(key);
	m_content << value;
	m_content << CRLF;
	m_content << m_szTailA;
	return *this;
}

MultipartFormDataRequest& MultipartFormDataRequest::AddKeyValue(const char* key, File &file, const char * content_type/* = NULL*/, const char * file_name/* = NULL*/)
{
	if (file_name && *file_name)
		AddKey(key, file_name, content_type);
	else
		AddKey(key, file.GetFileName(), content_type);

	m_content.FromFile(file);
	m_content << CRLF;
	m_content << m_szTailA;
	return *this;
}

MultipartFormDataRequest& MultipartFormDataRequest::AddKeyValue(const char* key, File &file, ULONGLONG offset, DWORD dwCount, const char * content_type/* = NULL*/)
{
	AddKey(key, file.GetFileName(), content_type);
	m_content.FromFile(file, offset, dwCount);
	m_content << CRLF;
	m_content << m_szTailA;
	return *this;
}

LPCTSTR MultipartFormDataRequest::GetHead() const
{
    _stprintf_s(m_szHead, _T("Content-type: multipart/form-data, boundary=B-O-U-N-D-A-R-Y%u%u\r\nContent-length: %u\r\n"), m_dwBoundaryRand1, m_dwBoundaryRand2, m_content.GetLength());

    return m_szHead;
}

const ByteStream &MultipartFormDataRequest::GetBody() const
{
    return m_content;
}

HRESULT MultipartFormDataRequest::GetBody(CComVariant &vtBody) const
{
	LPSAFEARRAY psa = SafeArrayCreateVector(VT_UI1, 0, m_content.GetLength());
	if (!psa) return E_OUTOFMEMORY;

	HRESULT hr;
	void* pData = NULL;
	if (FAILED(hr = SafeArrayAccessData(psa, &pData)))
		return hr;

	CopyMemory(pData, m_content.GetData(), m_content.GetLength());
	hr = SafeArrayUnaccessData(psa);

	vtBody.vt = VT_ARRAY | VT_UI1;
	vtBody.parray = psa;

	return hr;
}
