#pragma once

#include "Uploader.h"

namespace UPLOAD {
    namespace STUBS {

/**
 * Implementation of IUniversalUploaderListener.
 *
 * @author Alexander Kozlov
 */
template <typename TItem, typename TResult>
class UniversalUploaderListener : public IUniversalUploaderListener<TItem, TResult>
{
protected:
    UniversalUploaderListener()
    {
        ATLTRACE("UniversalUploaderListener::UniversalUploaderListener()\n");
    }

    ~UniversalUploaderListener()
    {
        ATLTRACE("UniversalUploaderListener::~UniversalUploaderListener()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > NewInstance()
    {
        return SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> >(new UniversalUploaderListener());
    }

public: // IUniversalUploaderListener implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &)
    {
        ATLTRACE("UniversalUploaderListener::InitWithUploader\n");
    }

    /**
     * called when all item processed
     */
    virtual void OnUploadComplete(const CSimpleArray<TItem> &items, const CSimpleArray<TResult> &result)
    {
        ATLTRACE("UniversalUploaderListener::UploadComplete\n");
    }

    /**
     * called when user press cancel.
     * if function will return false than upload will not be cancelled.
     * if force is true than return value will be ignored
     */
    virtual bool OnUploadCancelled(bool force)
    {
        ATLTRACE("UniversalUploaderListener::UploadCancelled(%s)\n", force ? "true" : "false");

        return true;
    }

    /**
     * called when current item successfully processed
     */
    virtual void OnItemComplete(const TItem &item, const TResult &result)
    {
        ATLTRACE("UniversalUploaderListener::ItemComplete\n");
    }

    /**
     * called when error has been occurred on upload current item.
     */
    virtual UniversalUploaderErrorAction OnItemError(const TItem &item, const UniversalUploaderErrorInfo &eiErrorInfo)
    {
        ATLTRACE("UniversalUploaderListener::ItemError\n");

        CString messageFormat;
#ifdef IDS_RETRY_FILE_UPLOAD
        messageFormat.LoadString(IDS_RETRY_FILE_UPLOAD);
#else
        messageFormat = _T("Cannot upload file: %s\nRetry?");
#endif
        CString strMessage = FormatString(messageFormat, (CString)item);

        CString strTitle;
#ifdef IDS_RETRY_FILE_TITLE
        strTitle.LoadString(IDS_RETRY_FILE_TITLE);
#else
        strTitle = _T("Upload Error...");
#endif
        int nID = MessageBox(NULL, strMessage, strTitle, MB_YESNOCANCEL|MB_ICONSTOP);
        switch(nID)
        {
        case IDYES:
            return eActionRetry;
        case IDNO:
            return eActionSkip;
        default:
            return eActionCancel;
        }

        return eActionDefault;
    }

    /**
     * called when current item was skipped
     */
    virtual void OnItemSkipped(const TItem &item)
    {
        ATLTRACE("UniversalUploaderListener::ItemSkipped\n");
    }
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderListener::Release\n");
        delete this;
    }
};

/**
 * Implementation of IUniversalUploaderProgressListener
 *
 * @author Alexander Kozlov
 */
class UniversalUploaderProgressListener : public IUniversalUploaderProgressListener
{
protected:
    UniversalUploaderProgressListener()
    {
        ATLTRACE("UniversalUploaderProgressListener::UniversalUploaderProgressListener()\n");
    }

    ~UniversalUploaderProgressListener()
    {
        ATLTRACE("UniversalUploaderProgressListener::~UniversalUploaderProgressListener()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderProgressListener> NewInstance()
    {
        return SmartReleasePtr<IUniversalUploaderProgressListener>(new UniversalUploaderProgressListener());
    }

public: // IUniversalUploaderProgressListener implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &)
    {
    }

    /**
	 * method will be called before upload for each item
	 */
	virtual void ShowProgress(bool bShow)
    {
        ATLTRACE("UniversalUploaderProgressListener::ShowProgress(%s)\n", bShow ? "true" : "false");
    }	

    /**
	 * method will be called before pre-process/upload for each item
	 */
	virtual void UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage)
    {
        ATLTRACE("UniversalUploaderProgressListener::UpdateProgress(%i,%i,...)\n", iItem, iTotal);
    }	

	/**
	 * called when progress has been changed
	 */
	virtual void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax)
    {
        ATLTRACE("UniversalUploaderProgressListener::OnChangeProgress(%i/%i, %i/%i bytes, %i/%i total)\n", dwProgress, dwProgressMax, dwBytesSent, dwBytesTotal, dwTotalProgress, dwTotalProgressMax);
    }	
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderProgressListener::Release\n");
        delete this;
    }	
};

/**
 * Implementation of IUniversalUploaderPreprocessor
 *
 * @author Alexander Kozlov
 */
template <typename T>
class UniversalUploaderPreprocessor : public IUniversalUploaderPreprocessor<T>
{
protected:
    UniversalUploaderPreprocessor()
    {
        ATLTRACE("UniversalUploaderPreprocessor::UniversalUploaderPreprocessor()\n");
    }

    ~UniversalUploaderPreprocessor()
    {
        ATLTRACE("UniversalUploaderPreprocessor::~UniversalUploaderPreprocessor()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderPreprocessor<T> > NewInstance()
    {
        return SmartReleasePtr<IUniversalUploaderPreprocessor<T> >(new UniversalUploaderPreprocessor());
    }

public: // IUniversalUploaderPreprocessor implementation
	/**
	 * method will be called before request builder for each item.
	 */
	virtual ePreprocessResult PreProgress(T &item)
    {
        ATLTRACE("UniversalUploaderPreprocessor::preProgress\n");

        return ePreprocessOK;
    }

	/**
	 * Called to destroy
	 */
    virtual void Release()
    {
        ATLTRACE("UniversalUploaderPreprocessor::Release\n");
        delete this;
    }	
};

/**
 * Implementation of IUniversalUploaderRequestBuilder
 *
 * @author Alexander Kozlov
 */
template <typename T>
class UniversalUploaderRequestBuilder : public IUniversalUploaderRequestBuilder<T>
{
protected:
    UniversalUploaderRequestBuilder()
    {
        ATLTRACE("UniversalUploaderRequestBuilder::UniversalUploaderListener()\n");
    }

    ~UniversalUploaderRequestBuilder()
    {
        ATLTRACE("UniversalUploaderRequestBuilder::~UniversalUploaderRequestBuilder()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderRequestBuilder<T> > NewInstance()
    {
        return SmartReleasePtr<IUniversalUploaderRequestBuilder<T> >(new UniversalUploaderRequestBuilder());
    }

public: // IUniversalUploaderRequestBuilder implementation
    /**
     * method will be called to create request. Must return MultipartFormDataRequest.
     * if function will return false item will be skipped.
     * method must throw exception to stop upload.
     */
    virtual bool CreateRequest(const T &item, HTTP::MultipartFormDataRequest &request)
    {
        ATLTRACE("UniversalUploaderRequestBuilder::CreateRequest\n");

        request << item;

        return true;
    }

	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderRequestBuilder::Release\n");
        delete this;
    }	
};

/**
 * Implementation of IUniversalUploaderResponseParser
 *
 * @author Alexander Kozlov
 */
template <typename T>
class UniversalUploaderResponseParser : public IUniversalUploaderResponseParser<T>
{
protected:
    UniversalUploaderResponseParser()
    {
        ATLTRACE("UniversalUploaderResponseParser::UniversalUploaderResponseParser()\n");
    }

    ~UniversalUploaderResponseParser()
    {
        ATLTRACE("UniversalUploaderResponseParser::~UniversalUploaderResponseParser()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderResponseParser<T> > NewInstance()
    {
        return SmartReleasePtr<IUniversalUploaderResponseParser<T> >(new UniversalUploaderResponseParser());
    }

public: // IUniversalUploaderResponseParser implementation
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
    virtual UniversalUploaderParserResult Parse(long nStatus, const CStringA &strContentType, const CStringA &strResponse, T &tResult, UniversalUploaderErrorInfo &eiErrorInfo)
    {
        ATLTRACE( _T("UniversalUploaderResponseParser::Parse(%i, %s, ...)\n"), nStatus, strContentType );

        //TODO: check nStatus&strContentType

        if (strResponse >> tResult)
            return eSuccessfullyParsed;

        strResponse >> eiErrorInfo;
        return eInvalidResponse;
    }

	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderResponseParser::Release\n");
        delete this;
    }	
};


    }//namespace STUBS
}//namespace UPLOAD
