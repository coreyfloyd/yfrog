// SimpleUploader.h : Declaration of the CSimpleUploader

#pragma once

#include <atlwin.h>
#include <atlcrack.h>
#include <atlutil.h>
#include "Uploader.h"
#include "http/HttpRequest.h"

namespace UPLOAD {

// Simple message handler
#define UPLOAD_MESSAGE_HANDLER_EX0(msg, func) \
	if(uMsg == msg) \
	{ \
		SetMsgHandled(TRUE); \
		try\
		{\
			func(); \
		}\
		catch (CStringA e)\
		{\
			LOG(e, Internet::GetLastResponseInfo());\
			m_state = postError;\
			PostMessage(UM_ERROR);\
		}\
		catch (...)\
		{\
			LOG("Exception on upload");\
			m_state = postError;\
			PostMessage(UM_ERROR);\
		}\
		lResult = 0;\
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define UM_DISPATCH			(WM_USER + 555)
#define UM_OPEN_REQUEST		(WM_USER + 777 + 1)
#define UM_SEND_REQUEST		(WM_USER + 777 + 2)
#define UM_POST_DATA		(WM_USER + 777 + 3)
#define UM_END_REQUEST		(WM_USER + 777 + 4)
#define UM_READ_RESPONSE	(WM_USER + 777 + 6)
#define UM_DONE				(WM_USER + 777 + 7)
#define UM_ERROR			(WM_USER + 777 + 8)

/**
 * Asynchronous uploader for single request.
 *
 * @author Alexander Kozlov
 */
class CSimpleUploader
	: public CWindowImpl<CSimpleUploader>
	, public ISimpleUploader
{
protected:
	CSimpleUploader();
	~CSimpleUploader();

public:
	DECLARE_WND_CLASS( _T("UploaderWnd") )

	BEGIN_MSG_MAP_EX(CSimpleUploader)
		MESSAGE_HANDLER_EX(UM_DISPATCH, Dispatcher)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_OPEN_REQUEST, DoOpenRequest)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_SEND_REQUEST, DoSendRequest)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_POST_DATA, DoSendData)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_END_REQUEST, DoEndRequest)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_READ_RESPONSE, DoReadResponse)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_DONE, OnDone)
		UPLOAD_MESSAGE_HANDLER_EX0(UM_ERROR, OnError)
		MSG_WM_TIMER(OnTimer)
	END_MSG_MAP()

private:
	bool volatile m_canceled;	// upload was canceled

    SmartReleasePtr<ISimpleUploadListener> m_listener; // listener

	Internet			m_internet;
	InternetConnection	m_connection;
	InternetHttpRequest m_request;

	CUrl		m_url;
	// request
	CString		m_strHeaders;
	DWORD		m_dwPos;
	ByteStream	m_data;
	// Response
	CStringA	m_strResponse;
	// buffers
	DWORD				m_dwBytesWritten;
	INTERNET_BUFFERS	m_ibWriteBuff;
	char				m_szReadBuffer[1024];
	INTERNET_BUFFERSA	m_ibReadBuffA;

	enum {
		postUnknown,
		postInitialization,
		// connection
		postConnecting,
		postWaitConnection,
		postConnected,
		// opening request
		postOpeningRequest,
		postWaitOpeningRequest,
		postRequestOpened,
		// sending request
		postSendingRequest,
		postWaitSendingRequest,
		postRequestSent,
		// posting data
		postPostingData,
		postWaitPostingData,
		postDataPartiallyPosted,
		postDataPosted,
		// ending request
		postEndingRequest,
		postWaitEndingRequest,
		postRequestEnded,
		// reading response
		postReadingResponse,
		postWaitReadingResponse,
		// result codes
		postDone,
		postError
	} m_state;

	void CloseConnections();
	void DoInitialize() throw(...);
	void DoConnect() throw(...);
	void DoConnected() throw(...);
	void DoOpenRequest() throw(...);
	void DoRequestOpened() throw(...);
	void DoSendRequest() throw(...);
	void DoRequestSent() throw(...);
	void DoSendData() throw(...);
	void DoEndRequest() throw(...);
	void DoRequestEnded() throw(...);
	void DoReadStatus() throw(...);
	void DoReadResponse() throw(...);
	void OnDone() throw(...);
	void OnError() throw(...);

    void OnTimer(UINT_PTR nIDEvent);

	LRESULT Dispatcher(UINT, DWORD, DWORD);


	static void CALLBACK StatusCallback(
							HINTERNET hInternet,
							DWORD_PTR dwContext,
							DWORD dwInternetStatus,
							LPVOID lpvStatusInformation,
							DWORD dwStatusInformationLength
							);

	void SafeDeleteListener();

    virtual WNDPROC GetWindowProc()
	{
		return WindowProc;
	}

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    static SmartReleasePtr<ISimpleUploader> NewInstance()
    {
        CSimpleUploader* pUploader = new CSimpleUploader();

        SmartReleasePtr<ISimpleUploader> smartPtr(pUploader);
        //save week ptr
        pUploader->m_self = smartPtr.GetWeekPtr();

        return smartPtr;
    }

private:
    SmartReleasePtr<ISimpleUploader>::WeekPtr m_self;
    typedef WeekPtrLocker<typename SmartReleasePtr<ISimpleUploader>::WeekPtr> Locker;

public: // implementation of SimpleUploader
	virtual bool Upload(const CStringW &strURL, const MultipartFormDataRequest &request);

	virtual void Cancel();

	virtual bool IsCanceled() const
	{
		return m_canceled;
	}

	virtual void SetListener(const SmartReleasePtr<ISimpleUploadListener> &listener);

    virtual void Release()
    {
        delete this;
    }

private:
	static CLogger logger;
};

}//namespace UPLOAD
