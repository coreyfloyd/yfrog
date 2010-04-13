// ConsoleApp.cpp : Defines the entry point for the console application.
//
// Single file upload
//

#include "stdafx.h"
#include <atlbase.h>
#include <atlapp.h>
#include <atldlgs.h>

#include "ImageShackAPI.h"
using namespace API::ImageShack;

HANDLE gEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

class UploaderListenerImpl : public UploaderListener
{
public:
    UploaderListenerImpl()
    {
        printf("UploaderListenerImpl::UploaderListenerImpl()\n");
    }

    ~UploaderListenerImpl()
    {
        printf("UploaderListenerImpl::~UploaderListenerImpl()\n");
    }

public: // IUploaderListenerImpl implementation
    /**
     * called before upload to have ability to cancel upload
     */
    virtual void InitWithUploader(const SmartReleasePtr<UploaderCancel> &)
    {
        printf("UploaderListenerImpl::InitWithUploader\n");
    }

    /**
     * called when all item processed
     */
    virtual void OnUploadComplete(const CSimpleArray<UploadInfo> &items, const CSimpleArray<UploadResult> &result)
    {
        printf("UploaderListenerImpl::UploadComplete\n");
        for (int i = 0; i < result.GetSize(); ++i)
        {
            UploadResult res = result[i];
            printf("file: %s\n", (CStringA)res.files.image.file);
        }
    }

    /**
     * called when user press cancel.
     * if function will return false than upload will not be cancelled.
     * if force is true than return value will be ignored
     */
    virtual bool OnUploadCancelled(bool force)
    {
        printf("UploaderListenerImpl::UploadCancelled(%s)\n", force ? "true" : "false");

        return true;
    }

    /**
     * called when current item successfully processed
     */
    virtual void OnItemComplete(const UploadInfo &item, const UploadResult &result)
    {
        printf("UploaderListenerImpl::ItemComplete(%s,%s)\n", (CStringA)item.file, (CStringA)result.files.image.file);
    }

    /**
     * called when error has been occurred on upload current item.
     */
    virtual UploaderErrorAction OnItemError(const UploadInfo &item, const UploaderErrorInfo &eiErrorInfo)
    {
        printf("UploaderListenerImpl::ItemError\n");

        CString strMessage = (CString)_T("Cannot upload file: ") + (CString)item.file + _T("\nRetry?");
        CString strTitle = _T("Upload Error...");

        int nID = MessageBox(NULL, strMessage, strTitle, MB_YESNOCANCEL|MB_ICONSTOP);
        switch(nID)
        {
        case IDYES:
            return UPLOAD::eActionRetry;
        case IDNO:
            return UPLOAD::eActionSkip;
        default:
            return UPLOAD::eActionCancel;
        }

        return UPLOAD::eActionDefault;
    }

    /**
     * called when current item was skipped
     */
    virtual void OnItemSkipped(const UploadInfo &item)
    {
        printf("UploaderListenerImpl::ItemSkipped(%s)\n", (CStringA)item.file);
    }
	
	/**
	 * Called to destroy
	 */
	virtual void Release()
    {
        printf("UploaderListenerImpl::Release\n");
        delete this;

        ::SetEvent(gEvent);
    }
};

void TestAPI()
{
    try
    {
	    ImageShackAPI api("IA5ZRTV6fb6256ccbc3c38650bdce6e6dcfc9e55"/*Test DevKey*/);

        // trying to login
        if (!api.Login())
            printf("Upload will anonymous!\n");
        else
            printf("user cookie %s\n", (CStringA)api.GetUserInfo()->GetCookie());

        // we need file to upload
        CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_FORCESHOWHIDDEN, _T("Images\0*.jpg;*.jpeg;*.png;*.gif;*.bmp;*.tif;*.tiff\0\0"));
        if (IDOK != dlg.DoModal())
            return;

        UploadInfo file(dlg.m_ofn.lpstrFile);

        api.UploadFiles(&file, 1, UploaderListenerSmartPtr(new UploaderListenerImpl));

        ::WaitForSingleObject( gEvent, INFINITE );

        if (api.IsLoggedIn()) api.OpenMyImagesPage();
    }
    catch (UploaderException e)
    {
        MessageBox(0, (CString)e.GetMessage(), _T("Exception"), 0);
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
	CoInitialize(NULL);

    TestAPI();

    CoUninitialize();

    // close global event
    CloseHandle(gEvent);

    return 0;
}
