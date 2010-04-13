#pragma once

#include "Uploader.h"
#include "templates.h"
#include "stubs.h"
#include "safe.h"
#include "Factory.h"
#include "API/Win32/Thread.h"
#include "http/HttpRequest.h"

#define MAX_TOTAL_PROGRESS 10000

namespace UPLOAD {
    using namespace TLIB;

/**
 * Universal Uploader implementation.
 *
 * @author Alexander Kozlov
 */
template <typename TItem, typename TResult>
class UniversalUploader
    : public IUniversalUploader<TItem, TResult>
{
protected:
    UniversalUploader(void)
        : m_iCurrentItemIndex(-1)
        , m_cancelled(false)
        , m_state(constructed)
    {
        ATLTRACE( _T("UniversalUploader::UniversalUploader()\n") );
    }

    ~UniversalUploader(void)
    {
        ATLTRACE( _T("UniversalUploader::~UniversalUploader()\n") );
        // week reference to self is not valid in destructor
        m_self = 0;
    }

public:
    static SmartReleasePtr< IUniversalUploader<TItem, TResult> > NewInstance()
    {
        SmartReleasePtr<UniversalUploader> smartPtr(new UniversalUploader());
        //save week ptr
        smartPtr->m_self = smartPtr.GetWeekPtr();

        return smartPtr;
    }

protected:
    typedef typename SmartReleasePtr<UniversalUploader>::WeekPtr UploaderWeekPtr;
    typedef WeekPtrLocker<UploaderWeekPtr> Locker;

    UploaderWeekPtr m_self;

protected:
    SmartReleasePtr< IUniversalUploaderListener<TItem, TResult> > m_listener;
    SmartReleasePtr< IUniversalUploaderProgressListener > m_progress;
    SmartReleasePtr< IUniversalUploaderURLResolver<TItem> > m_urlResolver;
    SmartReleasePtr< IUniversalUploaderPreprocessor<TItem> > m_preprocessor;
    SmartReleasePtr< IUniversalUploaderRequestBuilder<TItem> > m_requestBuilder;
    SmartReleasePtr< IUniversalUploaderResponseParser<TResult> > m_responseParser;

    CSimpleArray<TItem> m_items;
    int m_iCurrentItemIndex;

    CSimpleArray<TResult> m_result;

    typedef enum { constructed, destroying, destroyed, prepareUpload, preprocessingItem, initializeUpload, startUploading, uploadingInProgress, uploadComplete, cancellingUpload, uploadCancelled } eUniversalUploaderState;
    eUniversalUploaderState m_state;

    bool m_cancelled;

    SmartReleasePtr<ISimpleUploader> m_uploader;

public:// IUniversalUploader
	virtual void SetListener(const SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > &listener)
    {
        m_listener = listener;
    }
	virtual void SetProgress(const SmartReleasePtr<IUniversalUploaderProgressListener> &progress)
    {
        m_progress = progress;
    }
	virtual void SetUrlResolver(const SmartReleasePtr<IUniversalUploaderURLResolver<TItem> > &urlResolver)
    {
        m_urlResolver = urlResolver;
    }
	virtual void SetPreprocessor(const SmartReleasePtr<IUniversalUploaderPreprocessor<TItem> > &preprocessor)
    {
        m_preprocessor = preprocessor;
    }
	virtual void SetRequestBuilder(const SmartReleasePtr<IUniversalUploaderRequestBuilder<TItem> > &requestBuilder)
    {
        m_requestBuilder = requestBuilder;
    }
	virtual void SetResponseParser(const SmartReleasePtr<IUniversalUploaderResponseParser<TResult> > &responseParser)
    {
        m_responseParser = responseParser;
    }

	virtual TItem& GetCurrentItem() // return current item
    {
        ATLASSERT(m_iCurrentItemIndex >= 0 && m_iCurrentItemIndex < m_items.GetSize());
        return m_items[m_iCurrentItemIndex];
    }
	virtual const TItem& GetCurrentItem() const
    {
        ATLASSERT(m_iCurrentItemIndex >= 0 && m_iCurrentItemIndex < m_items.GetSize());
        return m_items[m_iCurrentItemIndex];
    }
	virtual const CSimpleArray<TItem>& GetItems() const
    {
        return m_items;
    }
    virtual int GetCurrentItemIndex() const
    {
        return m_iCurrentItemIndex;
    }

	virtual TResult& GetCurrentResult() // return current result
    {
        ATLASSERT(m_iCurrentItemIndex >= 0 && m_iCurrentItemIndex < m_result.GetSize());

        if (!m_items.GetSize())
            throw UniversalUploaderException("There is no items");

        return m_result[m_result.GetSize()-1];
    }

    virtual const TResult& GetCurrentResult() const
    {
        ATLASSERT(m_iCurrentItemIndex >= 0 && m_iCurrentItemIndex < m_result.GetSize());

        if (!m_result.GetSize())
            throw UniversalUploaderException("There is no result items");

        return m_result[m_result.GetSize()-1];
    }

    virtual const CSimpleArray<TResult>& GetResult() const
    {
        return m_result;
    }

    /**
     * Start upload of items
     * Parameter:
     *  items - array of items to upload.
     *           if you use default RequestBuilder than each item must be object or "associative" array with key and value pairs.
     * Throws:
     *   Method will throw exception if urlResolver property is not set or items.length is null.
     */
    virtual void Upload(const CSimpleArray<TItem> &items) throw(...)
    {
        LOG("Upload");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        if (!items.GetSize())
            throw UniversalUploaderException("Nothing to upload");

        if (constructed != m_state)
            throw UniversalUploaderException("Illegal Uploader State");

        m_items = items;

        LOG("Upload: call createStubs()");
        CreateStubs();

        LOG("Upload: Starting working thread");

        m_working_thread = SmartPtr<WorkingThread>(new WorkingThread(m_self));
    }

    /**
     * Stop upload
     */
    virtual void Cancel()
    {
        LOG("Cancel");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        LOG("Cancel: call listener.OnUploadCancelled(false)");
        if (!m_listener->OnUploadCancelled(false))
        {
            LOG("Cancel: call listener.OnUploadCancelled(false) return false! upload will not be cancelled!");
            return;
        }

        m_cancelled = true;

        LOG("Cancel: in state %i", m_state);

        try
        {
            switch (m_state)
            {
            case startUploading:
            case uploadingInProgress:
                m_state = cancellingUpload;
                LOG("Cancel: call progress.showProgress(false)");
                m_progress->ShowProgress(false);
                LOG("Cancel: call m_uploader->Cancel()...");
                ATLASSERT(m_uploader);
                m_uploader->Cancel();
                break;
            case preprocessingItem:
                m_state = cancellingUpload;
//                 LOG("Cancel: call preprocessor->Cancel()");
//                 m_preprocessor->Cancel();
                OnUploadCancelled();
                break;
            default:
                LOG("Cancel: nothing to do...");
                OnUploadCancelled();
                break;
            }
        }
        catch(UniversalUploaderException ex)
        {
            LOG("cancel: Exception: ", ex.GetMessage());
        }
    }

	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        delete this;
    }

protected:
    void Destroy()
    {
        try
        {
            ATLTRACE("destroy\n");

            m_state = destroying;

            m_listener.Release();
            m_progress.Release();
            m_urlResolver.Release();
            m_preprocessor.Release();
            m_requestBuilder.Release();
            m_responseParser.Release();

            if (m_upload_listener)
            {
                ((SimpleUploadListener&)*m_upload_listener).DestroyReference();
                m_upload_listener.Release();
            }
            m_uploader.Release();

            m_iCurrentItemIndex = -1;
            m_items.RemoveAll();
            m_result.RemoveAll();

            m_state = destroyed;
        }
        catch(...)
        {
            LOG("Exception on Destroy Uploader");
            ATLASSERT(FALSE);
        }
    }

    void CreateStubs() throw(...)
    {
        ATLTRACE("createStubs\n");

        if (!m_urlResolver)
        {
            ATLASSERT("Invalid using of UniversalUploader. UniversalUploader.urlResolver must be set!\n");

            throw UniversalUploaderException("You must set UniversalUploader.urlResolver!");
        }

        if (!m_listener)
        {
            ATLTRACE("create listener stub: UniversalUploaderListener\n");
            m_listener = STUBS::UniversalUploaderListener<TItem, TResult>::NewInstance();
        }
        else
        {
            ATLTRACE("wrap listener with: SAFE::UniversalUploaderListenerWrapper\n");
            m_listener = SAFE::UniversalUploaderListenerWrapper<TItem, TResult>::NewInstance(m_listener);
        }
        m_listener->InitWithUploader(SmartReleasePtr<UniversalUploader>(m_self));

        if (!m_progress)
        {
            ATLTRACE("create progress stub: UniversalUploaderProgressListener\n");
            m_progress = STUBS::UniversalUploaderProgressListener::NewInstance();
        }
        else
        {
            ATLTRACE("wrap progress with: SAFE::UniversalUploaderProgressListenerWrapper\n");
            m_progress = SAFE::UniversalUploaderProgressListenerWrapper::NewInstance(m_progress);
        }
        m_progress->InitWithUploader(SmartReleasePtr<UniversalUploader>(m_self));

        if (!m_preprocessor)
        {
            ATLTRACE("create preprocessor stub: UniversalUploaderPreprocessor\n");
            m_preprocessor = STUBS::UniversalUploaderPreprocessor<TItem>::NewInstance();
        }

        if (!m_requestBuilder)
        {
            ATLTRACE("create requestBuilder stub: UniversalUploaderRequestBuilder\n");
            m_requestBuilder = STUBS::UniversalUploaderRequestBuilder<TItem>::NewInstance();
        }

        if (!m_responseParser)
        {
            ATLTRACE("create responseParser stub: UniversalUploaderResponseParser\n");
            m_responseParser = STUBS::UniversalUploaderResponseParser<TResult>::NewInstance();
        }

        ATLTRACE("stubs has been created\n");
    }

    // start upload of next item
    bool UploadNextItem() throw(...)
    {
        LOG("UploadNextItem");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        if (++m_iCurrentItemIndex < m_items.GetSize())
        {
            ePreprocessResult result = PrepareUpload();
            if (result == ePreprocessOK)
                return UploadItem();
            //TODO: implement async 
            if (result == ePreprocessFailed)
                return SkipAndUploadNextItem();

            ATLASSERT(result == ePreprocessCancel);
            throw UniversalUploaderException("Preprocessing failed!");
        }

        LOG("UploadNextItem: call OnUploadComplete()");
        OnUploadComplete();
        return true;
    }

    // fire listener->OnItemSkipped() and call UploadNextItem()
    bool SkipAndUploadNextItem() throw(...)
    {
        LOG("skipAndUploadNextItem");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        LOG("SkipAndUploadNextItem: call listener->OnItemSkipped()");
        m_listener->OnItemSkipped(GetCurrentItem());

        LOG("SkipAndUploadNextItem: call UploadNextItem()");
        return UploadNextItem();
    }

    // prepare and pre-process upload
    // method returns pre-progress response
    ePreprocessResult PrepareUpload() throw(...)
    {
        LOG("prepareUpload");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        if (m_cancelled)
            throw "Upload has been cancelled";

        m_state = prepareUpload;

        LOG("prepareUpload: call progress.showProgress() to show/reinit progress");
        // must be called before item process to show/reinit progress
        m_progress->ShowProgress(true);

        // pre-processing
        if (m_cancelled)
            throw UniversalUploaderException("Upload has been cancelled");

        m_state = preprocessingItem;

        LOG("prepareUpload: call preprocessor.preProgress()");
        return m_preprocessor->PreProgress(GetCurrentItem());
    }

    // start upload of current item
    bool UploadItem() throw(...)
    {
        LOG("uploadItem");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        if (m_cancelled)
            throw UniversalUploaderException("Upload has been cancelled");

        m_state = initializeUpload;

        LOG("UploadItem: call progress->UpdateProgress() to update progress after preprocessor");
        // must be called after preprocessor
        m_progress->UpdateProgress(m_iCurrentItemIndex+1, m_items.GetSize(), GetString(IDS_UPLOADING));

        MultipartFormDataRequest request;
        LOG("UploadItem: call requestBuilder->CreateRequest()");
        if (!m_requestBuilder->CreateRequest(GetCurrentItem(), request)) // skip it
            return SkipAndUploadNextItem();

        LOG("UploadItem: call m_urlResolver->GetURL()");
        CStringW url = m_urlResolver->GetURL(GetCurrentItem());
        LOG("UploadItem: URL: ", url);

        if (!m_uploader)
        {
            // create uploader
            m_uploader = Factory::CreateSimpleUploader();
            // create listener
            m_upload_listener = SmartReleasePtr<ISimpleUploadListener>(new SimpleUploadListener(this, m_working_thread));
            ATLTRACE("UploadItem: set upload listener\n");
            m_uploader->SetListener(m_upload_listener);
        }

        ATLASSERT(m_uploader);


        m_state = startUploading;

        if (!m_uploader->Upload(url, request))
            throw UniversalUploaderException("Uploader internal error");

        LOG("UploadItem: upload started");

        return true;
    }

    // item upload complete inner handler
    void OnUploadComplete()
    {
        LOG("OnUploadComplete");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        LOG("OnUploadComplete: call listener.OnUploadComplete()");
        m_listener->OnUploadComplete(m_items, m_result);

        LOG("OnUploadComplete: call progress.showProgress(false)");
        m_progress->ShowProgress(false);

        LOG("OnUploadComplete: call Destroy()");
        Destroy();
    }

    // item upload canceled inner handler
    void OnUploadCancelled()
    {
        LOG("OnUploadCancelled");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        m_cancelled = true;
        m_state = uploadCancelled;

        LOG("OnUploadCancelled: call listener->OnItemSkipped()");
        m_listener->OnItemSkipped(GetCurrentItem());

        LOG("OnUploadCancelled: call listener.OnUploadCancelled(true)");
        m_listener->OnUploadCancelled(true);

        LOG("OnUploadCancelled: call progress.showProgress(false)");
        m_progress->ShowProgress(false);

        LOG("OnUploadCancelled: call Destroy()");
        Destroy();
    }

    // item upload complete inner listener. Will be called after upload complete and response parser will decide if response is ok or failed
    void OnItemComplete(long nStatus, const CStringA &strContentType, const CStringA &strResponse)
    {
        LOG("OnItemComplete");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);
        try
        {
            if (m_cancelled) // no action if upload was cancelled
            {
                LOG("upload was cancelled!");
                return;
            }

            LOG("OnItemComplete: getting response");

            TResult result;
            UniversalUploaderErrorInfo eiErrorInfo;

            UniversalUploaderParserResult status = m_responseParser->Parse(nStatus, strContentType, strResponse, result, eiErrorInfo);
            switch (status)
            {
            case eSkipItem:
                LOG("OnItemComplete: FAILED to parse response! Call SkipAndUploadNextItem()");
                SkipAndUploadNextItem();
                break;
            case eInvalidResponse:
                LOG("OnItemComplete: FAILED to parse response! Call OnItemError()");
                OnItemError(eiErrorInfo);
                break;
            case eSuccessfullyParsed:
                LOG("OnItemComplete: response was parsed successfully");
                m_result.Add(result);

                LOG("OnItemComplete: call OnChangeProgress(100, 100)");
                OnChangeProgress(100, 100);

                LOG("OnItemComplete: call listener->OnItemComplete()");
                m_listener->OnItemComplete(GetCurrentItem(), GetCurrentResult());

                LOG("OnItemComplete: call UploadNextItem()");
                UploadNextItem();
                break;
            default:
                throw UniversalUploaderException(FormatString( _T("Parser returned invalid status: %i"), status ));
            }
        }
        catch(UniversalUploaderException ex)
        {
            LOG("onItemComplete: Exception: ", ex.GetMessage());

            LOG("OnItemComplete: call onUploadCancelled()");
            OnUploadCancelled();
        }
    }

    // item upload error inner listener
    void OnItemError(const UniversalUploaderErrorInfo &eiErrorInfo)
    {
        LOG("OnItemError");
        // object should not be destroyed till end of method complete
        Locker selflock(m_self);

        if (m_cancelled) // no action if upload was cancelled
        {
            LOG("upload was cancelled!");
            return;
        }

        try
        {
            LOG("OnItemError: call listener.OnItemError()");
            UniversalUploaderErrorAction action = m_listener->OnItemError(GetCurrentItem(), eiErrorInfo);

            LOG("OnItemError: listener.OnItemError() -> " + action);

            if (eActionSkip == action)
                SkipAndUploadNextItem();
            else if (eActionRetry == action)
                UploadItem();
            else
                throw UniversalUploaderException("Cancel upload");
        }
        catch(UniversalUploaderException ex)
        {
            LOG("onItemError: Exception in onItemError: ", ex.GetMessage());

            LOG("OnItemError: call onUploadCancelled()");
            OnUploadCancelled();
        }
    }

    /**
     * Called when progress must be changed.
     */
    void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent = 0, DWORD dwBytesTotal = 0) const
    {
        DWORD dwTotal = (DWORD)(((LONGLONG)m_iCurrentItemIndex*MAX_TOTAL_PROGRESS + (LONGLONG)dwProgress*MAX_TOTAL_PROGRESS/dwProgressMax)/(LONGLONG)m_items.GetSize());

        m_progress->OnChangeProgress(dwProgress, dwProgressMax, dwBytesSent, dwBytesTotal, dwTotal, MAX_TOTAL_PROGRESS);
    }

protected:
    class WorkingThread : protected Runnable
    {
    private:
        UploaderWeekPtr m_uploader;
        API::Win32::Thread m_thread;

    public:
        WorkingThread(UploaderWeekPtr uploader)
            : m_uploader(uploader)
        {
            ATLTRACE( _T("WorkingThread::WorkingThread()\n") );

            m_uploader.AddRef();

            if (!m_thread.CreateCOM(this))
            {
                m_uploader.Release();
                throw UniversalUploaderException("Cannot create working thread");
            }
        }

        ~WorkingThread()
        {
            ATLTRACE( _T("WorkingThread::~WorkingThread: Posting WM_QUIT to thread queue\n") );
            m_thread.PostMessage(WM_QUIT);
        }

        virtual void Run()
        {
	        try
	        {
                LOG("Upload: Starting upload %i items", m_uploader->GetItems().GetSize());

                {
                    Locker lockuploader(m_uploader);
                    m_uploader.Release();

                    try
                    {
                        m_uploader->UploadNextItem();
                    }
                    catch(UniversalUploaderException ex)
                    {
                        LOG("Exception: ", ex.GetMessage());

                        LOG("upload: call OnUploadCancelled()");
                        m_uploader->OnUploadCancelled();
                        // throw to skip message loop
                        throw ex;
                    }
                    catch(...)
                    {
                        LOG("Unexpected exception on UploadNextItem");

                        LOG("call OnUploadCancelled()");
                        m_uploader->OnUploadCancelled();
                        // throw to skip message loop
                        throw UniversalUploaderException("Unexpected exception on UploadNextItem");
                    }
                }

                for (MSG msg; GetMessage(&msg, NULL, 0, 0); )
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
	        }
            catch(UniversalUploaderException ex)
            {
                ATLTRACE( _T("Catched exception: %s\n"), ex.GetMessage() );
            }
	        catch (...)
	        {
                LOG("Unexpected exception in working thread");
                ATLASSERT(FALSE);
	        }
        }

    private:
        static CLogger logger;
    };
    SmartPtr<WorkingThread> m_working_thread;

    class SimpleUploadListener : public ISimpleUploadListener
    {
    private:
        UniversalUploader* m_pUploader;
        SmartPtr<WorkingThread> m_working_thread;

    public:
        SimpleUploadListener(UniversalUploader *pUploader, SmartPtr<WorkingThread> working_thread)
            : m_pUploader(pUploader)
            , m_working_thread(working_thread)
        {
        }

        /**
	     * Called when progress must be changed.
	     */
	    virtual void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal)
        {
            if (!m_pUploader)
                return;

            if (m_pUploader->m_state == startUploading)
                m_pUploader->m_state = uploadingInProgress;

            m_pUploader->OnChangeProgress(dwProgress, dwProgressMax, dwBytesSent, dwBytesTotal);
        }

	    /**
	     * Called after upload complete.
	     */
	    virtual void OnComplete(long nStatus, const CStringA &strContentType, const CStringA &strResponse)
        {
            if (!m_pUploader)
                return;

            m_pUploader->m_state = uploadComplete;
            m_pUploader->OnItemComplete(nStatus, strContentType, strResponse);
        }

	    /**
	     * Called when upload was cancelled.
	     */
	    virtual void OnCancelled()
        {
            if (m_pUploader) m_pUploader->OnUploadCancelled();
        }

	    /**
	     * Called on upload error.
	     */
	    virtual void OnError()
        {
            //TODO: get real error
            if (m_pUploader) m_pUploader->OnItemError(UniversalUploaderErrorInfo());
        }
    	
	    /**
	     * Called to destroy reference
	     */
	    void DestroyReference()
        {
            m_pUploader = 0;
        }
    	
	    /**
	     * Called to destroy
	     */
	    virtual void Release()
        {
            delete this;
        }
    };

    SmartReleasePtr<ISimpleUploadListener> m_upload_listener;

private:
    static CLogger logger;
};

template <typename TItem, typename TResult>
CLogger UniversalUploader<TItem, TResult>::logger;

template <typename TItem, typename TResult>
CLogger UniversalUploader<TItem, TResult>::WorkingThread::logger;

}//namespace UPLOAD
