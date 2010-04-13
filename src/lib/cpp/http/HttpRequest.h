#pragma once

#include "ByteStream.h"

namespace HTTP {

/**
 * Implementation of POST multipart/form-data request.
 *
 * @author Alexander Kozlov
 */
class IMAGESHACK_API MultipartFormDataRequest
{
public:
	MultipartFormDataRequest();

	MultipartFormDataRequest& AddKeyValue(const char* key, const char* value);
	MultipartFormDataRequest& AddKeyValue(const char* key, int value);
	MultipartFormDataRequest& AddKeyValue(const char* key, ULONGLONG value);
	MultipartFormDataRequest& AddKeyValue(const char* key, API::Win32::File &file, const char * content_type = NULL, const char * file_name = NULL);
	MultipartFormDataRequest& AddKeyValue(const char* key, API::Win32::File &file, ULONGLONG offset, DWORD dwCount, const char * content_type = NULL);

	LPCTSTR GetHead() const;
	const ByteStream &GetBody() const;
	HRESULT GetBody(CComVariant &vtBody) const;

	void Reset();

protected:
	void AddKey(const char* key, const char* pszFileName = NULL, const char * content_type = NULL);
	void AddKey(const char* key, const wchar_t* pszFileName, const char * content_type = NULL)
	{
		AddKey(key, (CStringA)pszFileName, content_type);
	}

private:
	ByteStream m_content;

    mutable TCHAR m_szHead[128];

    DWORD m_dwBoundaryRand1, m_dwBoundaryRand2;
	char  m_szTailA[64];

public:
	class Mapper
	{
	private:
		MultipartFormDataRequest &request;
		const char* key;

	public:
		Mapper(MultipartFormDataRequest &req, const char* pszKey) : request(req), key(pszKey) {}
		Mapper(MultipartFormDataRequest *pReq, const char* pszKey) : request(*pReq), key(pszKey) {}

		MultipartFormDataRequest &operator = (const char* value)
		{
			return request.AddKeyValue(key, value);
		}

		MultipartFormDataRequest &operator = (int value)
		{
			return request.AddKeyValue(key, value);
		}

		MultipartFormDataRequest &operator = (ULONGLONG value)
		{
			return request.AddKeyValue(key, value);
		}

		MultipartFormDataRequest &From(API::Win32::File &file)
		{
			return request.AddKeyValue(key, file);
		}

		MultipartFormDataRequest &FromFile(API::Win32::File &file, const char * content_type = NULL, const char * file_name = NULL)
		{
			return request.AddKeyValue(key, file, content_type, file_name);
		}

		MultipartFormDataRequest &FromFile(API::Win32::File &file, LPCWSTR content_type, LPCWSTR file_name = NULL)
		{
			return request.AddKeyValue(key, file, CW2A(content_type), CW2A(file_name));
		}

		MultipartFormDataRequest &FromFile(API::Win32::File &file, ULONGLONG offset, DWORD dwCount, const char * content_type = NULL)
		{
			return request.AddKeyValue(key, file, offset, dwCount, content_type);
		}
	};

public:
	Mapper operator[](const char* key)
	{
		return MultipartFormDataRequest::Mapper(this, key);
	}
};

#define DECLARE_HTTP_DATA(className)    HTTP::MultipartFormDataRequest& operator << (HTTP::MultipartFormDataRequest&, const className&);

#define HTTP_DATA_MAP(className)        HTTP::MultipartFormDataRequest& operator << (HTTP::MultipartFormDataRequest &request, const className &item) {

#define HTTP_ELEMENT(element)           request.AddKeyValue(#element, item.element);
#define HTTP_ELEMENT2(element, member)  request.AddKeyValue(element, item.member);

#define HTTP_LPCTSTR(element)                               if (item.element && *item.element) request.AddKeyValue(#element, EncodeToUTF8(item.element));
#define HTTP_LPCTSTR2(element, member)                      if (item.member && item.member) request.AddKeyValue(element, EncodeToUTF8(item.member));

#define HTTP_STRING(element)                                if (!item.element.IsEmpty()) request.AddKeyValue(#element, EncodeToUTF8(item.element));
#define HTTP_STRING2(element, member)                       if (!item.member.IsEmpty()) request.AddKeyValue(element, EncodeToUTF8(item.member));
#define HTTP_STRING2_IF(element, member, condition)         if ((condition) && !item.member.IsEmpty()) request.AddKeyValue(element, EncodeToUTF8(item.member));
#define HTTP_STRING_CONSTANT(name, value)                   request.AddKeyValue(name, value);
#define HTTP_STRING_CONSTANT_IF(name, value, condition)     if (condition) request.AddKeyValue(name, value);

#define HTTP_BOOL(element, true_constant, false_constant)           request.AddKeyValue(#element, item.element ? true_constant : false_constant);
#define HTTP_BOOL4(element, member, true_constant, false_constant)  request.AddKeyValue(element, item.member ? true_constant : false_constant);

#define HTTP_STRING_SIMPLE_ARRAY(element)            if (item.element.GetSize()) { for (int i = 0; i < item.element.GetSize(); ++i) request.AddKeyValue(#element, EncodeToUTF8(item.element[i])); }
#define HTTP_STRING_SIMPLE_ARRAY2(element, member)   if (item.member.GetSize()) { for (int i = 0; i < item.member.GetSize(); ++i) request.AddKeyValue(element, EncodeToUTF8(item.member[i])); }

#define HTTP_FILE_ELEMENT(element)                                      { API::Win32::File file; if (file.Open((CString)item.element)) request.AddKeyValue(#element, file); }
#define HTTP_FILE_ELEMENT2(element, member)                             { API::Win32::File file; if (file.Open((CString)item.member)) request.AddKeyValue(element, file); }
#define HTTP_FILE_ELEMENT2EX(element, member, file_name, content_type)  { API::Win32::File file; if (file.Open((CString)item.member)) request.AddKeyValue(element, file, content_type, file_name); }
#define HTTP_FILE_ELEMENT2EX_IF(element, member, file_name, content_type, condition) if (condition) { API::Win32::File file; if (file.Open((CString)item.member)) request.AddKeyValue(element, file, content_type, file_name); }

#define HTTP_ENUM(element)              request.AddKeyValue(#element, enum_to_string(item.element));
#define HTTP_ENUM2(element, member)     request.AddKeyValue(element, enum_to_string(item.member));

#define HTTP_DATA_MAP_END()             return request; }

} // namespace HTTP
