#pragma once

#include "Uploader.h"
#include "safe.h"

namespace UPLOAD {
namespace TLIB {

/**
 * Implementation of IUniversalUploaderListener with Auto Release functionality.
 *
 * @author Alexander Kozlov
*/
template <typename TItem, typename TResult, typename TRelease>
class AutoReleaseObjectUniversalUploaderListener : public IUniversalUploaderListener<TItem, TResult>
{
private:
    SmartReleasePtr<TRelease> m_ptrT;

protected:
    explicit AutoReleaseObjectUniversalUploaderListener(const SmartReleasePtr<TRelease> &ptrT)
        : m_ptrT(ptrT)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::AutoReleaseObjectUniversalUploaderListener()\n");
    }

    ~AutoReleaseObjectUniversalUploaderListener()
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::~AutoReleaseObjectUniversalUploaderListener()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > NewInstance(const SmartReleasePtr<TRelease> &ptrT)
    {
        return SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> >(new AutoReleaseObjectUniversalUploaderListener(ptrT));
    }

public: // IUniversalUploaderListener implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &cancel)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::InitWithUploader\n");
    }

    /**
     * called when all item processed
     */
    virtual void OnUploadComplete(const CSimpleArray<TItem> &items, const CSimpleArray<TResult> &result)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::UploadComplete\n");
        ReleaseObject();
    }

    /**
     * called when user press cancel.
     * if function will return false than upload will not be cancelled.
     * if force is true than return value will be ignored
     */
    virtual bool OnUploadCancelled(bool force)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::UploadCancelled(%s)\n", force ? "true" : "false");

        if (force) ReleaseObject();

        return true;
    }

    /**
     * called when current item successfully processed
     */
    virtual void OnItemComplete(const TItem &item, const TResult &result)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::ItemComplete\n");
    }

    /**
     * called when error has been occurred on upload current item.
     */
    virtual UniversalUploaderErrorAction OnItemError(const TItem &item, const UniversalUploaderErrorInfo &eiErrorInfo)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::ItemError\n");

        return eActionDefault;
    }

    /**
     * called when current item was skipped
     */
    virtual void OnItemSkipped(const TItem &item)
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::ItemSkipped\n");
    }
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("AutoReleaseObjectUniversalUploaderListener::Release\n");
        delete this;
    }

protected:
    void ReleaseObject()
    {
        try
        {
            m_ptrT.Release();
        }
        catch (...)
        {
            ATLTRACE( _T("AutoReleaseObjectUniversalUploaderListener::Exception on Release object\n") );
            ATLASSERT(FALSE);
        }
    }
};

/**
 * Implementation of composite listener.
 *
 * @author Alexander Kozlov
 */
template <typename TItem, typename TResult>
class UniversalUploaderListenerComposite : public IUniversalUploaderListener<TItem, TResult>
{
protected:
    SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > m_listener1;
    SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > m_listener2;

protected:
    UniversalUploaderListenerComposite(const SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > &listener1, const SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > &listener2)
    {
        ATLTRACE("UniversalUploaderListenerComposite::UniversalUploaderListenerComposite()\n");
        m_listener1 = SAFE::UniversalUploaderListenerWrapper<TItem, TResult>::NewInstance(listener1);
        m_listener2 = SAFE::UniversalUploaderListenerWrapper<TItem, TResult>::NewInstance(listener2);
    }

    ~UniversalUploaderListenerComposite()
    {
        ATLTRACE("UniversalUploaderListenerComposite::~UniversalUploaderListenerComposite()\n");
    }

public:
    static SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > NewInstance(const SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > &listener1, const SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> > &listener2)
    {
        return SmartReleasePtr<IUniversalUploaderListener<TItem, TResult> >(new UniversalUploaderListenerComposite(listener1, listener2));
    }

public: // IUniversalUploaderListener implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<IUniversalUploaderCancel> &uploader)
    {
        ATLTRACE("UniversalUploaderListenerComposite::InitWithUploader\n");
        m_listener1->InitWithUploader(uploader);
        m_listener2->InitWithUploader(uploader);
    }

    /**
     * called when all item processed
     */
    virtual void OnUploadComplete(const CSimpleArray<TItem> &items, const CSimpleArray<TResult> &result)
    {
        ATLTRACE("UniversalUploaderListenerComposite::UploadComplete\n");
        m_listener1->OnUploadComplete(items, result);
        m_listener2->OnUploadComplete(items, result);
    }

    /**
     * called when user press cancel.
     * if function will return false than upload will not be cancelled.
     * if force is true than return value will be ignored
     */
    virtual bool OnUploadCancelled(bool force)
    {
        ATLTRACE("UniversalUploaderListenerComposite::UploadCancelled(%s)\n", force ? "true" : "false");

        bool res1 = m_listener1->OnUploadCancelled(force);
        bool res2 = m_listener2->OnUploadCancelled(force);

        return res1 && res2;
    }

    /**
     * called when current item successfully processed
     */
    virtual void OnItemComplete(const TItem &item, const TResult &result)
    {
        ATLTRACE("UniversalUploaderListenerComposite::ItemComplete\n");
        m_listener1->OnItemComplete(item, result);
        m_listener2->OnItemComplete(item, result);
    }

    /**
     * called when error has been occurred on upload current item.
     */
    virtual UniversalUploaderErrorAction OnItemError(const TItem &item, const UniversalUploaderErrorInfo &eiErrorInfo)
    {
        ATLTRACE("UniversalUploaderListenerComposite::ItemError\n");

        UniversalUploaderErrorAction res1 = m_listener1->OnItemError(item, eiErrorInfo);
        UniversalUploaderErrorAction res2 = m_listener2->OnItemError(item, eiErrorInfo);

        if (eActionDefault == res1) return res2;
        if (eActionDefault == res2) return res1;

        if (res1 == eActionCancel || res2 == eActionCancel)
            return eActionCancel;

        if (res1 == eActionSkip || res2 == eActionSkip)
            return eActionSkip;

        return eActionRetry;
    }

    /**
     * called when current item was skipped
     */
    virtual void OnItemSkipped(const TItem &item)
    {
        ATLTRACE("UniversalUploaderListenerComposite::ItemSkipped\n");
        m_listener1->OnItemSkipped(item);
        m_listener2->OnItemSkipped(item);
    }
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        ATLTRACE("UniversalUploaderListenerComposite::Release\n");
        delete this;
    }
};

}//namespace TLIB
}//namespace UPLOAD
