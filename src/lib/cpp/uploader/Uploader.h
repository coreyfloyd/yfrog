#pragma once

#include "templates/SmartPtr.h"
using namespace TEMPL;

namespace HTTP {
    class MultipartFormDataRequest;
}

namespace UPLOAD {
    using namespace HTTP;

/**
 * Simple Uploader Listener Interface.
 *
 * @author Alexander Kozlov
 */
class ATL_NO_VTABLE ISimpleUploadListener
{
public:
	/**
	 * Called when progress must be changed.
	 */
	virtual void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal) = 0;

	/**
	 * Called after upload complete.
	 */
	virtual void OnComplete(long nStatus, const CStringA &strContentType, const CStringA &strResponse) = 0;

	/**
	 * Called when upload was cancelled.
	 */
	virtual void OnCancelled() = 0;

	/**
	 * Called on upload error.
	 */
	virtual void OnError() = 0;
	
	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Simple Uploader Interface.
 *
 * @author Alexander Kozlov
 */
class ATL_NO_VTABLE ISimpleUploader
{
public:
	/**
	 * Send/POST request.
	 */
    virtual bool Upload(const CStringW &strURL, const MultipartFormDataRequest &request) = 0;

	/**
	 * Set Upload Listener
	 */
	virtual void SetListener(const SmartReleasePtr<ISimpleUploadListener> &listener) = 0;
	
	/**
	 * Cancel Upload
	 */
	virtual void Cancel() = 0;

	/**
	 * Check "Cancel" status
	 */
	virtual bool IsCanceled() const = 0;
	
	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Universal Uploader Error Action.
 */
typedef enum {
	eActionDefault,
	eActionSkip,
	eActionRetry,
	eActionCancel
} UniversalUploaderErrorAction;

/**
 * Universal Uploader Error info.
 */
struct UniversalUploaderErrorInfo {
	CStringW strErrorCode;
	CStringW strErrorMessage;
};

/**
 * Universal Uploader Cancel Interface.
 *
 * @author Alexander Kozlov
 */
class ATL_NO_VTABLE IUniversalUploaderCancel
{
public:
    /**
     * Stop upload
     */
    virtual void Cancel() = 0;

	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Universal Uploader Listener Interface.
 *
 * @author Alexander Kozlov
 */
template <typename TItem, typename TResult>
class ATL_NO_VTABLE IUniversalUploaderListener
{
public:
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &) = 0;

    /**
     * called when all item processed
     */
    virtual void OnUploadComplete(const CSimpleArray<TItem> &items, const CSimpleArray<TResult> &result) = 0;

    /**
     * called when user press cancel.
     * if function will return false than upload will not be cancelled.
     * if force is true than return value will be ignored
     */
    virtual bool OnUploadCancelled(bool force) = 0;

    /**
     * called when current item successfully processed
     */
    virtual void OnItemComplete(const TItem &item, const TResult &result) = 0;

    /**
     * called when error has been occurred on upload current item.
     */
    virtual UniversalUploaderErrorAction OnItemError(const TItem &item, const UniversalUploaderErrorInfo &eiErrorInfo) = 0;

    /**
     * called when current item was skipped
     */
    virtual void OnItemSkipped(const TItem &item) = 0;
	
	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};


/**
 * Universal Uploader Progress Listener Interface.
 *
 * @author Alexander Kozlov
 */
class ATL_NO_VTABLE IUniversalUploaderProgressListener
{
public:
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &) = 0;

    /**
	 * method will be called before upload for each item
	 */
	virtual void ShowProgress(bool bShow) = 0;
	
    /**
	 * method will be called before pre-process/upload for each item
	 */
	virtual void UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage) = 0;

	/**
	 * called when progress has been changed
	 */
	virtual void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax) = 0;
	
	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Preprocessor Result
 */
typedef enum {
	ePreprocessOK,		// item was preprocessed
	ePreprocessFailed,	// preprocessing FAILED, skip item
	ePreprocessCancel	// preprocessing FAILED, cancel upload
} ePreprocessResult;

/**
 * Universal Uploader Preprocessor Interface.
 *
 * @author Alexander Kozlov
 */
template <typename T>
class ATL_NO_VTABLE IUniversalUploaderPreprocessor
{
public:
	/**
	 * method will be called before request builder for each item.
	 */
	virtual ePreprocessResult PreProgress(T &item) = 0;

	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Request Builder Interface
 *
 * @author Alexander Kozlov
 */
template <typename T>
class ATL_NO_VTABLE IUniversalUploaderRequestBuilder
{
public:
    /**
     * method will be called to create request. Must return MultipartFormDataRequest.
     * if function will return false item will be skipped.
     * method must throw exception to stop upload.
     */
    virtual bool CreateRequest(const T &item, MultipartFormDataRequest &request) = 0;

	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Parser result
 */
enum UniversalUploaderParserResult {
	eSuccessfullyParsed,	// result successfully parsed
	eInvalidResponse,		// invalid response, listener must decide what to do
	eSkipItem				// invalid response, skip item silently
};

/**
 * Response parser interface
 *
 * @author Alexander Kozlov
 */
template <typename T>
class ATL_NO_VTABLE IUniversalUploaderResponseParser
{
public:
    /**
     * Method will be called after receiving response for each item.
     *
     * Parameters:
     *   status - HTTP status
     *   contentType - Response Content-Type
     *   inputStream - response as InputStream
     *
     * Method must throw exception to stop upload.
     */
    virtual UniversalUploaderParserResult Parse(long nStatus, const CStringA &strContentType, const CStringA &strResponse, T &tResult, UniversalUploaderErrorInfo &eiErrorInfo) = 0;

	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * URL Resolver Interface.
 *
 * @author Alexander Kozlov
 */
template <typename T>
class ATL_NO_VTABLE IUniversalUploaderURLResolver
{
public:
    /**
     * Must return URL for POST data request
     */
    virtual CStringW GetURL(const T &item) = 0;

	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

/**
 * Universal Uploader Exception
 */
class UniversalUploaderException
{
public:
	UniversalUploaderException(LPCSTR pszMessage)
		: m_strMessage(pszMessage)
	{
	}

	UniversalUploaderException(LPCWSTR pszMessage)
		: m_strMessage(pszMessage)
	{
	}

	const CStringW &GetMessage() const
	{
		return m_strMessage;
	}

private:
	CStringW m_strMessage;
};

/**
 * Universal Uploader Interface..
 *
 * @author Alexander Kozlov
 */
template <typename TItem, typename TResult>
class ATL_NO_VTABLE IUniversalUploader : public IUniversalUploaderCancel
{
public:
	virtual void SetListener(const SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > &listener) = 0;
	virtual void SetProgress(const SmartReleasePtr<IUniversalUploaderProgressListener> &progress) = 0;
	virtual void SetUrlResolver(const SmartReleasePtr<IUniversalUploaderURLResolver<TItem> > &urlResolver) = 0;
	virtual void SetPreprocessor(const SmartReleasePtr<IUniversalUploaderPreprocessor<TItem> > &preprocessor) = 0;
	virtual void SetRequestBuilder(const SmartReleasePtr<IUniversalUploaderRequestBuilder<TItem> > &requestBuilder) = 0;
	virtual void SetResponseParser(const SmartReleasePtr<IUniversalUploaderResponseParser<TResult> > &responseParser) = 0;

	virtual TItem& GetCurrentItem() = 0; // return current item
	virtual const TItem& GetCurrentItem() const = 0;
	virtual const CSimpleArray<TItem>& GetItems() const = 0;
    virtual int GetCurrentItemIndex() const = 0;

	virtual TResult& GetCurrentResult() = 0; // return current result
	virtual const TResult& GetCurrentResult() const = 0;
	virtual const CSimpleArray<TResult>& GetResult() const = 0;

    /**
     * Start upload of items
     * Parameter:
     *  items - array of items to upload.
     *           if you use default RequestBuilder than each item must be object or "associative" array with key and value pairs.
     * Throws:
     *   Method will throw exception if urlResolver property is not set or items.length is null.
     */
    virtual void Upload(const CSimpleArray<TItem> &items) throw(...) = 0;

    /**
     * Stop upload
     */
    virtual void Cancel() = 0;

	/**
	 * Called to destroy
	 */
	virtual void Release() = 0;
};

}//namespace UPLOAD
