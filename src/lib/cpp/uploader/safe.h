#pragma once

#include "Uploader.h"

namespace UPLOAD {
    namespace SAFE {

/**
 * Safe wrapper of IUniversalUploaderListener.
 *
 * @author Alexander Kozlov
 */
template <typename TItem, typename TResult>
class UniversalUploaderListenerWrapper : public IUniversalUploaderListener<TItem, TResult>
{
private:
    SmartReleasePtr< IUniversalUploaderListener<TItem, TResult> > m_listener;

protected:
    UniversalUploaderListenerWrapper(const SmartReleasePtr< IUniversalUploaderListener<TItem, TResult> > &listener)
        : m_listener(listener)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::UniversalUploaderListenerWrapper()\n");
    }

    ~UniversalUploaderListenerWrapper()
    {
        ATLTRACE("UniversalUploaderListenerWrapper::~UniversalUploaderListenerWrapper()\n");
    }

public:
    static SmartReleasePtr< IUniversalUploaderListener<TItem, TResult> > NewInstance(const SmartReleasePtr< IUniversalUploaderListener<TItem, TResult> > &listener)
    {
        return SmartReleasePtr< IUniversalUploaderListener<TItem, TResult> >(new UniversalUploaderListenerWrapper(listener));
    }

public: // IUniversalUploaderListener implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &cancel)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::InitWithUploader\n");
        try
        {
            if (m_listener) m_listener->InitWithUploader(cancel);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }

    /**
     * called when all item processed
     */
    virtual void OnUploadComplete(const CSimpleArray<TItem> &items, const CSimpleArray<TResult> &result)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::UploadComplete\n");
        try
        {
            if (m_listener) m_listener->OnUploadComplete(items, result);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }

    /**
     * called when user press cancel.
     * if function will return false than upload will not be cancelled.
     * if force is true than return value will be ignored
     */
    virtual bool OnUploadCancelled(bool force)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::UploadCancelled\n");
        try
        {
            if (m_listener) return m_listener->OnUploadCancelled(force);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }

        return true;
    }

    /**
     * called when current item successfully processed
     */
    virtual void OnItemComplete(const TItem &item, const TResult &result)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::ItemComplete\n");
        try
        {
            if (m_listener) m_listener->OnItemComplete(item, result);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }

    /**
     * called when error has been occurred on upload current item.
     */
    virtual UniversalUploaderErrorAction OnItemError(const TItem &item, const UniversalUploaderErrorInfo &eiErrorInfo)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::ItemError\n");
        try
        {
            if (m_listener) return m_listener->OnItemError(item, eiErrorInfo);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }

        return eActionDefault;
    }

    /**
     * called when current item was skipped
     */
    virtual void OnItemSkipped(const TItem &item)
    {
        ATLTRACE("UniversalUploaderListenerWrapper::ItemSkipped\n");
        try
        {
            if (m_listener) m_listener->OnItemSkipped(item);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderListenerWrapper::Release\n");
        try
        {
            m_listener.Release();
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
        ATLTRACE("UniversalUploaderListenerWrapper::Release - delete this\n");
        delete this;
    }
};

/**
 * Safe wrapper of IUniversalUploaderProgressListener
 *
 * @author Alexander Kozlov
 */
class UniversalUploaderProgressListenerWrapper : public IUniversalUploaderProgressListener
{
private:
    SmartReleasePtr< IUniversalUploaderProgressListener > m_listener;

protected:
    UniversalUploaderProgressListenerWrapper(const SmartReleasePtr< IUniversalUploaderProgressListener > &listener)
        : m_listener(listener)
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::UniversalUploaderProgressListenerWrapper()\n");
    }

    ~UniversalUploaderProgressListenerWrapper()
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::~UniversalUploaderProgressListenerWrapper()\n");
    }

public:
    static SmartReleasePtr< IUniversalUploaderProgressListener > NewInstance(const SmartReleasePtr< IUniversalUploaderProgressListener > &listener)
    {
        return SmartReleasePtr< IUniversalUploaderProgressListener >(new UniversalUploaderProgressListenerWrapper(listener));
    }

public: // IUniversalUploaderProgressListener implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &cancel)
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::InitWithUploader\n");
        try
        {
            if (m_listener) m_listener->InitWithUploader(cancel);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }

    /**
	 * method will be called before upload for each item
	 */
	virtual void ShowProgress(bool bShow)
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::ShowProgress(%s)\n", bShow ? "true" : "false");
        try
        {
            if (m_listener) m_listener->ShowProgress(bShow);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }	

    /**
	 * method will be called before pre-process/upload for each item
	 */
	virtual void UpdateProgress(int iItem, int iTotal, LPCTSTR pszMessage)
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::ShowProgress(%i,%i,...)\n", iItem, iTotal);
        try
        {
            if (m_listener) m_listener->UpdateProgress(iItem, iTotal, pszMessage);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }	

	/**
	 * called when progress has been changed
	 */
	virtual void OnChangeProgress(DWORD dwProgress, DWORD dwProgressMax, DWORD dwBytesSent, DWORD dwBytesTotal, DWORD dwTotalProgress, DWORD dwTotalProgressMax)
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::ShowProgress(%i/%i, %i/%i bytes, %i/%i total)\n", dwProgress, dwProgressMax, dwBytesSent, dwBytesTotal, dwTotalProgress, dwTotalProgressMax);
        try
        {
            if (m_listener) m_listener->OnChangeProgress(dwProgress, dwProgressMax, dwBytesSent, dwBytesTotal, dwTotalProgress, dwTotalProgressMax);
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
    }	
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderProgressListenerWrapper::Release\n");
        try
        {
            m_listener.Release();
        }
        catch(...)
        {
            ATLTRACE("Exception on call wrapped object\n");
            ATLASSERT(FALSE);
        }
        ATLTRACE("UniversalUploaderProgressListenerWrapper::Release - delete this\n");
        delete this;
    }	
};

    }//namespace SAFE
}//namespace UPLOAD
