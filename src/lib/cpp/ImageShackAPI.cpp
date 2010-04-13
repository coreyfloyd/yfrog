#include "StdAfx.h"
#include "ImageShackAPI.h"
#include "./Factory.h"

#include "http/Connection.h"
using namespace HTTP;

#include "uploader/stubs.h"
using namespace UPLOAD;
using namespace UPLOAD::TLIB;

#include "xml/XmlParser.h"
#include "xml/xml_op.h"
using namespace UTIL;

#include "dialog/LoginDlg.h"
#include "dialog/Progress.h"

using namespace ImageShack;

#define MULTIPLE_READ_BY            100
#define MULTIPLE_TERMINATE_TIMEOUT  7000

namespace API {
namespace ImageShack {

	/**
	 * AuthenticateResponse
	 */
    struct AuthenticateResponse : public IUserInfo
	{
		CStringW id;
		CStringW name;
		CStringW cookie;

        virtual LPCWSTR GetId() const { return id; }
        virtual LPCWSTR GetName() const { return name; }
        virtual LPCWSTR GetCookie() const { return cookie; }

        void Empty()
        {
            id.Empty();
            name.Empty();
            cookie.Empty();
        }
	};

    /**
     * Private data
     */
    struct ImageShackAPIPrivate
    {
	    CString devkey;
	    CString cookie;
	    CString username;
	    CString password;

        AuthenticateResponse auth_info;

        ImageShackAPIPrivate(const CString& strDevKey);
        
        void Empty()
        {
	        cookie.Empty();
	        username.Empty();
	        password.Empty();
            auth_info.Empty();
        }

    private:
        ImageShackAPIPrivate& operator = (const ImageShackAPIPrivate& rPriv);
    };

    CLogger logger("ImageShackAPI");

} // namespace ImageShack
} // namespace API

XML_SERIALIZE_MAP(API::ImageShack::ErrorResponse)
    XML_MAP_PROCESS_ELEMENT("error")
    XML_MAP_ATTRIBUTE2("id", code)
	XML_MAP_TEXT(message)
XML_END_SERIALIZE_MAP()

XML_SERIALIZE_MAP(API::ImageShack::AuthenticateResponse)
	XML_MAP_ELEMENT(id)
	XML_MAP_ELEMENT2("username", name)
	XML_MAP_ELEMENT(cookie)
XML_END_SERIALIZE_MAP()

HTTP_DATA_MAP(API::ImageShack::UploadInfo)
    HTTP_STRING_CONSTANT("xml", "newformat")
    HTTP_FILE_ELEMENT2EX_IF("fileupload", file, (CStringA)ExtractFileName((CString)item.file), (CStringA)item.content_type, !IsHttpURL(item.file))
    HTTP_STRING2_IF("url", file, IsHttpURL(item.file))
    HTTP_STRING2("content-type", content_type)
    HTTP_STRING2("optsize", size)
    HTTP_STRING_CONSTANT_IF("optimage", "1", !item.size.IsEmpty())
    HTTP_STRING(tags)
    HTTP_BOOL4("rembar", remove_bar, "yes", "no")
    HTTP_BOOL4("public", is_public, "yes", "no")
    HTTP_FILE_ELEMENT2("frmupload", thumbnail)
    HTTP_STRING2("key", devkey)
    HTTP_STRING(cookie)
    HTTP_STRING2("a_username", username)
    HTTP_STRING2("a_password", password)
    HTTP_STRING_CONSTANT_IF("nocookie", "1", item.cookie.IsEmpty() && item.username.IsEmpty())
HTTP_DATA_MAP_END()

XML_SERIALIZE_MAP(API::ImageShack::UploadResult::FileInfo)
	XML_MAP_ATTRIBUTE(size)
	XML_MAP_ATTRIBUTE2("content-type", content_type)
	XML_MAP_TEXT(file)
XML_END_SERIALIZE_MAP()

XML_SERIALIZE_MAP(API::ImageShack::UploadResult::Files)
	XML_MAP_ATTRIBUTE(server)
	XML_MAP_ATTRIBUTE(bucket)
	XML_MAP_ELEMENT(image)
	XML_MAP_ELEMENT2("thumb", thumbnail)
XML_END_SERIALIZE_MAP()

XML_SERIALIZE_MAP(API::ImageShack::UploadResult::Resolution)
	XML_MAP_ELEMENT(width)
	XML_MAP_ELEMENT(height)
XML_END_SERIALIZE_MAP()

XML_SERIALIZE_MAP(API::ImageShack::UploadResult::VideoInfo)
	XML_MAP_ELEMENT(status)
	XML_MAP_ELEMENT(duration)
XML_END_SERIALIZE_MAP()

XML_SERIALIZE_MAP(API::ImageShack::UploadResult::Links)
    XML_MAP_ELEMENT(image_link)
    XML_MAP_ELEMENT(image_html)
    XML_MAP_ELEMENT(image_bb)
    XML_MAP_ELEMENT(image_bb2)
    XML_MAP_ELEMENT(yfrog_link)
    XML_MAP_ELEMENT(yfrog_thumb)
    XML_MAP_ELEMENT(thumb_link)
    XML_MAP_ELEMENT(thumb_html)
    XML_MAP_ELEMENT(thumb_bb)
    XML_MAP_ELEMENT(thumb_bb2)
    XML_MAP_ELEMENT(ad_link)
    XML_MAP_ELEMENT(video_embed)
    XML_MAP_ELEMENT(done_page)
XML_END_SERIALIZE_MAP()

XML_SERIALIZE_MAP(API::ImageShack::UploadResult)
	XML_MAP_ELEMENT(files)
	XML_MAP_ELEMENT(resolution)
	XML_MAP_ELEMENT2("video-info", video_info)
	XML_MAP_ELEMENT(visibility)
	XML_MAP_ELEMENT(links)
XML_END_SERIALIZE_MAP()

using namespace API::ImageShack;

ImageShackAPIPrivate::ImageShackAPIPrivate(const CString& strDevKey)
    : devkey(strDevKey)
{
}

ImageShackAPI::ImageShackAPI(const char* pszDevKey)
    : m_pPrivate(0)
{
    m_pPrivate = new ImageShackAPIPrivate((CString)pszDevKey);
}

ImageShackAPI::ImageShackAPI(const ImageShackAPI& rAPI)
    : m_pPrivate(0)
{
    m_pPrivate = new ImageShackAPIPrivate(*rAPI.m_pPrivate);
}

ImageShackAPI::~ImageShackAPI()
{
    if (m_pPrivate) delete m_pPrivate, m_pPrivate = NULL;
}

bool ImageShackAPI::Login()
{
	CLoginDlg dlg(*this);
	if (IDOK != dlg.DoModal())
		return false;

	return IsLoggedIn();
}

bool ImageShackAPI::Login(LPCWSTR pszUserName, LPCWSTR pszPassword, ErrorResponse* pError/* = NULL*/)
{
	// keep the last
	m_pPrivate->Empty();
	m_pPrivate->username = pszUserName;
	m_pPrivate->password = pszPassword;

	CWaitCursor wait;
	Configuration cfg;

	HTTP::MultipartFormDataRequest request;
	request["username"] = EncodeToUTF8(pszUserName);
	request["password"] = EncodeToUTF8(pszPassword);
	request["format"]   = "xml";
	request["nocookie"] = "1";

	HTTP::Connection connection;
	if (!connection.Send(cfg.urlLoginAPI, request))
		return false;

 	CStringA strResponse;
	if (!connection.GetResponse(strResponse) || strResponse.IsEmpty())
		return false;

	((CString)strResponse) >> m_pPrivate->auth_info;

	if (!m_pPrivate->auth_info.cookie.IsEmpty())
		return true;

	// try to get error description
    ErrorResponse errorResponse;
    if (((CString)strResponse) >> errorResponse && pError)
    {
        pError->code = errorResponse.code;
        pError->message = errorResponse.message;
    }

	return false;
}

bool ImageShackAPI::IsLoggedIn() const 
{
	return !m_pPrivate->auth_info.cookie.IsEmpty();
}

IUserInfo* ImageShackAPI::GetUserInfo() const
{
    return IsLoggedIn() ? &m_pPrivate->auth_info : 0;
}

void ImageShackAPI::Logout()
{
    m_pPrivate->Empty();
}

void ImageShackAPI::SetCookie(LPCWSTR pszCookie)
{
    m_pPrivate->Empty();
    m_pPrivate->cookie = pszCookie;
}

void ImageShackAPI::SetAuthInfo(LPCWSTR pszUser, LPCWSTR pszPassword)
{
    m_pPrivate->Empty();
    m_pPrivate->username = pszUser;
    m_pPrivate->password = pszPassword;
}

void ImageShackAPI::UploadFiles(const UploadInfo* pFiles, UINT nCount, const UploaderListenerSmartPtr &uploaderListener, const ProgressListenerSmartPtr &progressListener)
{
    if (!pFiles || !nCount)
        return;

    CSimpleArray<UploadInfo> items;
    for (UINT i = 0; i < nCount; ++i)
    {
        UploadInfo item(pFiles[i]);

        if (IsFileURL(item.file))
            item.file = ToFile((CString)item.file);

        if (item.content_type.IsEmpty())//we need content type to process item
            item.content_type = GetContentType(ExtractFileExtension((CString)item.file));

        // init internal fields
	    item.devkey = m_pPrivate->devkey;
        if (!m_pPrivate->auth_info.cookie.IsEmpty())
	        item.cookie = m_pPrivate->auth_info.cookie;
        else if (!m_pPrivate->cookie.IsEmpty())
	        item.cookie = m_pPrivate->cookie;
        else if (!m_pPrivate->username.IsEmpty())
        {
            item.username = m_pPrivate->username;
            item.password = m_pPrivate->password;
        }

        items.Add(item);
    }

    UploaderSmartPtr uploader = Factory::CreateUploader();

    if (!progressListener) // create default progress dialog
        uploader->SetProgress(CProgress::NewInstance(*uploader));
    else
        uploader->SetProgress(progressListener);

    UploaderListenerSmartPtr autoReleaseListener = AutoReleaseObjectUniversalUploaderListener<UploadInfo, UploadResult, Uploader>::NewInstance(uploader);

    if (uploaderListener)
        uploader->SetListener(UploaderListenerComposite::NewInstance(uploaderListener, autoReleaseListener));
    else
        uploader->SetListener(UploaderListenerComposite::NewInstance(STUBS::UniversalUploaderListener<UploadInfo, UploadResult>::NewInstance(), autoReleaseListener));

    uploader->Upload(items);
}

bool operator >> (const CStringA &strResponse, API::ImageShack::UploadResult &urResult)
{
#ifdef _UNICODE
    return ((CString)strResponse) >> urResult;
#else
    return ::operator >> <API::ImageShack::UploadResult> (strResponse, urResult);
#endif
}

bool operator >> (const CStringA &strResponse, UPLOAD::UniversalUploaderErrorInfo &eiErrorResponse)
{
    ErrorResponse error_response;

    if (!(((CString)strResponse) >> error_response))
        return false;

    eiErrorResponse.strErrorCode = error_response.code;
    eiErrorResponse.strErrorMessage = error_response.message;

    return true;
}

void ImageShackAPI::OpenMyImagesPage()
{
    API::Win32::Shell::Execute(Configuration().urlMyImagesPage);
}

void ImageShackAPI::OpenMyVideosPage()
{
    API::Win32::Shell::Execute(Configuration().urlMyVideosPage);
}
