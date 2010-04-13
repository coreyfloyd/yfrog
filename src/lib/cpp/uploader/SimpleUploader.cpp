// SimpleUploader.cpp : Implementation of CSimpleUploader

#include "stdafx.h"
#include "SimpleUploader.h"

// Timer ID to cancel upload
#define TID_CANCEL_RESPONSE                 666
// wait before close sessions on cancel in ms
#define WAIT_FOR_CANCEL_RESPONSE_TIMEOUT    10000

using namespace UPLOAD;

DEFINE_LOGGER(CSimpleUploader)

// CSimpleUploader
CSimpleUploader::CSimpleUploader()
	: m_canceled(false)
	, m_dwPos(0)
	, m_state(postUnknown)
	, m_dwBytesWritten(0)
{
	LOG("Uploader has been created 0x%x", (void*)this);

	ZeroMemory(&m_ibWriteBuff, sizeof(m_ibWriteBuff));
	ZeroMemory(&m_ibReadBuffA, sizeof(m_ibReadBuffA));
	ZeroMemory(&m_szReadBuffer, sizeof(m_szReadBuffer));
}

CSimpleUploader::~CSimpleUploader()
{
	LOG("Destroying uploader 0x%x", (void*)this);
    // week reference to self is not valid in destructor
    m_self = 0;

    if (m_hWnd && IsWindow())
    {
        if (m_canceled) // kill timer
            KillTimer(TID_CANCEL_RESPONSE);

        DestroyWindow(), m_hWnd = NULL;
    }

    CloseConnections();
}

LRESULT CALLBACK CSimpleUploader::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSimpleUploader* pThis = (CSimpleUploader*)hWnd;

    // object should not be destroyed till processing message
    Locker selflock(pThis->m_self);

	return __super::WindowProc(hWnd, uMsg, wParam, lParam);
}

void CSimpleUploader::CloseConnections()
{
	m_request.Close();
	m_connection.Close();

	if (m_internet) InternetSetStatusCallback(m_internet, NULL), m_internet.Close();
}

void CSimpleUploader::DoInitialize() throw(...)
{
	LOG("Initializing...");

	m_state = postInitialization;
	m_canceled = false;

	ATLASSERT(!m_internet);
	ATLASSERT(!m_connection);
	ATLASSERT(!m_request);

	ZeroMemory(&m_ibWriteBuff, sizeof(m_ibWriteBuff));
	ZeroMemory(&m_ibReadBuffA, sizeof(m_ibReadBuffA));
	ZeroMemory(&m_szReadBuffer, sizeof(m_szReadBuffer));
	m_strResponse.Empty(), m_dwBytesWritten = 0;

	if (!m_internet.Open(HTTP_USER_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_ASYNC))
		throw (CStringA)FormatString( _T("Cannot open internet connection: %i"), GetLastError());

	if (INTERNET_INVALID_STATUS_CALLBACK == m_internet.SetStatusCallback((INTERNET_STATUS_CALLBACK)StatusCallback))
		throw (CStringA)"Cannot set callback";

	DWORD dwTimeOut = 0, dwLength = sizeof(dwTimeOut);
	if (m_internet.QueryOption(INTERNET_OPTION_RECEIVE_TIMEOUT, &dwTimeOut, &dwLength))
	{
		LOG("RECEIVE TIMEOUT: %i ms", dwTimeOut);
	}
	dwTimeOut = 0, dwLength = sizeof(dwTimeOut);
	if (m_internet.QueryOption(INTERNET_OPTION_SEND_TIMEOUT, &dwTimeOut, &dwLength))
	{
		LOG("SEND TIMEOUT: %i ms", dwTimeOut);
	}

	if (!m_hWnd)
	{
		if (!Create(NULL, NULL, NULL, WS_POPUP))
			LOG("Cannot create reflection window");
	}
}

void CSimpleUploader::DoConnect() throw(...)
{
	LOG("Connecting...");

	m_state = postConnecting;
	if (!m_connection.Connect(m_internet, m_url.GetHostName(), m_url.GetPortNumber(), INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)m_hWnd))
	{
		if (GetLastError() != ERROR_IO_PENDING)
			throw (CStringA)FormatString( _T("InternetConnect failed, error: %i"), GetLastError());

		m_state = postWaitConnection;
		LOG("Connecting async...");
	}
	else
	{
		LOG("Connected sync.");
		DoConnected();
	}
}

void CSimpleUploader::DoConnected() throw(...)
{
	LOG("Connected.");
	m_state = postConnected;
	PostMessage(m_canceled ? UM_DONE : UM_OPEN_REQUEST);
}

void CSimpleUploader::DoOpenRequest() throw(...)
{
	LOG("Opening Request...");
	m_state = postOpeningRequest;

    TCHAR szPath[2*ATL_URL_MAX_PATH_LENGTH+1];
    _tcscpy_s(szPath, m_url.GetUrlPath());
	if (m_url.GetExtraInfoLength())
        _tcscat_s(szPath, m_url.GetExtraInfo());

    if (!m_request.Open(m_connection, METHOD_POST, szPath, INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, (DWORD_PTR)m_hWnd))
	{
		if (GetLastError() != ERROR_IO_PENDING)
			throw (CStringA)FormatString( _T("HttpOpenRequest failed, error: %i"), GetLastError());

		m_state = postWaitOpeningRequest;
		LOG("Opening Request async...");
	}
	else
	{
		LOG("Request Opened sync.");
		DoRequestOpened();
	}
}

void CSimpleUploader::DoRequestOpened() throw(...)
{
	LOG("Request Opened.");
	m_state = postRequestOpened;
	PostMessage(m_canceled ? UM_DONE : UM_SEND_REQUEST);
}

void CSimpleUploader::DoSendRequest() throw(...)
{
	LOG("Sending Request...");

	ZeroMemory(&m_ibWriteBuff, sizeof(m_ibWriteBuff));
	m_ibWriteBuff.dwStructSize= sizeof(m_ibWriteBuff);
	m_ibWriteBuff.dwBufferTotal = m_data.GetLength();
	m_ibWriteBuff.lpcszHeader = m_strHeaders;
	m_ibWriteBuff.dwHeadersLength = m_strHeaders.GetLength();

	m_state = postSendingRequest;
	if (!m_request.SendRequestEx(&m_ibWriteBuff, NULL, (DWORD_PTR)m_hWnd))
	{
		if (GetLastError() != ERROR_IO_PENDING)
			throw (CStringA)FormatString( _T("HttpSendRequestEx failed, error: %i"), GetLastError());

		m_state = postWaitSendingRequest;
		LOG("Sending Request async...");
	}
	else
	{
		LOG("Request sent sync.");
		DoRequestSent();
	}
}

void CSimpleUploader::DoRequestSent() throw(...)
{
	LOG("Request sent.");

	m_state = postRequestSent;
	PostMessage(m_canceled ? UM_END_REQUEST : UM_POST_DATA);
}

void CSimpleUploader::DoSendData() throw(...)
{
	LOG("Sending data...");

	long lTotal15 = (m_data.GetLength() * 115 / 100);
	m_listener->OnChangeProgress((lTotal15 * 5 / 100) + m_dwPos, lTotal15, m_dwPos, m_data.GetLength());

	m_state = postPostingData;
	if(m_dwPos < m_data.GetLength())
	{
		DWORD dwBytesToProcess = min(1024, m_data.GetLength() - m_dwPos);
		if (!m_request.Write(m_data.GetData() + m_dwPos, dwBytesToProcess, &m_dwBytesWritten))
		{
			if (GetLastError() != ERROR_IO_PENDING)
				throw (CStringA)FormatString( _T("InternetWriteFile failed, error: %i"), GetLastError());

			m_state = postWaitPostingData;
			LOG("Sending data async...");
			m_dwPos += dwBytesToProcess;
		}
		else
		{
			LOG("Data Sent sync.");
			m_dwPos += m_dwBytesWritten;
			m_state = postDataPartiallyPosted;
			PostMessage(m_canceled ? UM_END_REQUEST : UM_POST_DATA);
		}
	}
	else
	{
		LOG("All data sent...");
		m_state = postDataPosted;
		PostMessage(UM_END_REQUEST);
	}
}

void CSimpleUploader::DoEndRequest() throw(...)
{
	LOG("Sending END...");

	m_state = postEndingRequest;
	if (!m_request.EndRequest((DWORD_PTR)m_hWnd))
	{
		if (GetLastError() != ERROR_IO_PENDING)
			throw (CStringA)FormatString( _T("HttpEndRequest failed, error: %i"), GetLastError());

		m_state = postWaitEndingRequest;
		LOG("Sending END async...");
	}
	else
	{
		LOG("END sent sync.");
		DoRequestEnded();
	}
}

void CSimpleUploader::DoRequestEnded() throw(...)
{
	LOG("Request ended");

	m_state = postRequestEnded;
	PostMessage(UM_READ_RESPONSE);
}

void CSimpleUploader::DoReadResponse() throw(...)
{
	LOG("Reading response...");

	ZeroMemory(&m_ibReadBuffA, sizeof(m_ibReadBuffA));
	ZeroMemory(&m_szReadBuffer, sizeof(m_szReadBuffer));
	m_ibReadBuffA.dwStructSize = sizeof(m_ibReadBuffA);
	m_ibReadBuffA.lpvBuffer = m_szReadBuffer;
	m_ibReadBuffA.dwBufferLength = sizeof(m_szReadBuffer) - 1;

 	m_state = postReadingResponse;
	if (!m_request.ReadExA(&m_ibReadBuffA, IRF_NO_WAIT, (DWORD_PTR)m_hWnd))
	{
		if (GetLastError() != ERROR_IO_PENDING)
			throw (CStringA)FormatString( _T("InternetReadFile failed, error: %i"), GetLastError());

		m_state = postWaitReadingResponse;
		LOG("Reading response async...");
	}
	else
	{
		LOG("Response read sync.");

        if (0 == m_ibReadBuffA.dwBufferLength) // response was completely read
        {
            LOG("Response was read");
            PostMessage(UM_DONE);
        }
        else
        {
            m_szReadBuffer[m_ibReadBuffA.dwBufferLength] = 0;
            m_strResponse += m_szReadBuffer;

            PostMessage(UM_READ_RESPONSE);
        }
	}
}

void CSimpleUploader::OnDone() throw(...)
{
	try
	{
	    LOG("Response:", m_strResponse);

        if (postDone == m_state) // may be we already process this state
            return;

        m_state = postDone;
		CloseConnections();

		if (m_canceled)
		{
			LOG("Upload was canceled");

			m_listener->OnCancelled();
		}
		else
		{
			LOG("Upload complete");

			//TODO: http-status and content-type
			m_listener->OnComplete(0, CStringA(), m_strResponse);
		}
	}
	catch(...)
	{
		LOG("EXCEPTION OnDone 0x%x", (void*)this);
	}
}

void CSimpleUploader::OnError() throw(...)
{
	try
	{
		m_state = postError;
		CloseConnections();

		LOG("Upload error!");
	
		ATLASSERT(m_listener);
		m_listener->OnError();
	}
	catch (...)
	{
		LOG("Exception in OnError()");
	}
}

void CSimpleUploader::Cancel()
{
	try
	{
		LOG("Canceling upload");

        if (!m_canceled) // not yet cancelled
            SetTimer(TID_CANCEL_RESPONSE, WAIT_FOR_CANCEL_RESPONSE_TIMEOUT);

		m_canceled = true;
	}
	catch(...)
	{
		LOG("EXCEPTION on Cancel 0x%x", (void*)this);
	}
}

void CSimpleUploader::OnTimer(UINT_PTR nIDEvent)
{
    if (TID_CANCEL_RESPONSE != nIDEvent)
        return;

    ATLASSERT(m_canceled);
    OnDone();
}

LRESULT CSimpleUploader::Dispatcher(UINT, DWORD dwInternetStatus, DWORD dwParam)
{
	try
	{
		switch (dwInternetStatus)
		{
		case INTERNET_STATUS_HANDLE_CREATED:
			LOG("Dispatch: HANDLE_CREATED, State: %i", m_state);
			{
				switch(m_state)
				{
				case postWaitConnection:
					{
						if (!m_connection) m_connection.Attach((HINTERNET)dwParam);

						DoConnected();
					}
					break;
				case postWaitOpeningRequest:
					{
						if (!m_request)
							m_request.Attach((HINTERNET)dwParam);
					}
					break;
				}
			}
			break;
		case INTERNET_STATUS_REQUEST_COMPLETE:
			LOG("Dispatch: REQUEST_COMPLETE, State: %i", m_state);
			if (0 == dwParam)
			{
				switch(m_state)
				{
				case postWaitOpeningRequest:
					DoRequestOpened();
					break;
				case postWaitSendingRequest:
					DoRequestSent();
					break;
				case postWaitPostingData:
					m_state = postDataPartiallyPosted;
					PostMessage(m_canceled ? UM_END_REQUEST : UM_POST_DATA);
					break;
				case postWaitEndingRequest:
					DoRequestEnded();
					break;
				case postWaitReadingResponse:
					PostMessage(UM_READ_RESPONSE);
					break;
				default:
					LOG("Dispatch: ALERT! REQUEST_COMPLETE for State: %i", m_state);
				}
			}
			else
			{
				LOG("Dispatch: Upload error: %i", dwParam);
				PostMessage(UM_ERROR);
			}
			break;
		}
	}
	catch (...)
	{
		LOG("Exception in Dispatcher");
	}

	return 0;
}

void CALLBACK CSimpleUploader::StatusCallback(
									 HINTERNET hInternet,
									 DWORD_PTR dwContext,
									 DWORD dwInternetStatus,
									 LPVOID lpvStatusInformation,
									 DWORD dwStatusInformationLength
									 )
{
	if (!dwContext)
		return;

	try
	{
		switch (dwInternetStatus)
		{
		case INTERNET_STATUS_HANDLE_CREATED:
			{
				INTERNET_ASYNC_RESULT *pRes = (INTERNET_ASYNC_RESULT *)lpvStatusInformation;
				LOG("HANDLE_CREATED: 0x%X", pRes->dwResult);
				::PostMessage((HWND)dwContext, UM_DISPATCH, dwInternetStatus, pRes->dwResult);
			}
			break;
		case INTERNET_STATUS_RESOLVING_NAME:
			LOG("RESOLVING_NAME");
			break;
		case INTERNET_STATUS_CONNECTING_TO_SERVER:
			LOG("CONNECTING_TO_SERVER");
			break;
		case INTERNET_STATUS_CONNECTED_TO_SERVER:
			LOG("CONNECTED_TO_SERVER");
			break;
		case INTERNET_STATUS_SENDING_REQUEST:
			LOG("SENDING_REQUEST");
			break;
		case INTERNET_STATUS_REQUEST_SENT:
			LOG("REQUEST_SENT %i bytes", *((DWORD*)lpvStatusInformation));
			break;
		case INTERNET_STATUS_REQUEST_COMPLETE:
			{
				INTERNET_ASYNC_RESULT *pRes = (INTERNET_ASYNC_RESULT *)lpvStatusInformation;
				LOG("REQUEST_COMPLETE: error %i", pRes->dwError);
				::PostMessage((HWND)dwContext, UM_DISPATCH, dwInternetStatus, pRes->dwError);
			}
			break;
		case INTERNET_STATUS_RECEIVING_RESPONSE:
			LOG("RECEIVING_RESPONSE");
			break;
		case INTERNET_STATUS_RESPONSE_RECEIVED:
			LOG("RESPONSE_RECEIVED %i bytes", *((DWORD*)lpvStatusInformation));
			break;
		case INTERNET_STATUS_CLOSING_CONNECTION:
			LOG("CLOSING_CONNECTION");
			break;
		case INTERNET_STATUS_CONNECTION_CLOSED:
			LOG("CONNECTION_CLOSED");
			break;
		case INTERNET_STATUS_HANDLE_CLOSING:
			LOG("HANDLE_CLOSING");
			break;
		case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
			LOG("INTERMEDIATE_RESPONSE");
			break;
		case INTERNET_STATUS_NAME_RESOLVED:
			LOG("NAME_RESOLVED");
			break;
		case INTERNET_STATUS_REDIRECT:
			LOG("REDIRECT");
			break;
		case INTERNET_STATUS_STATE_CHANGE:
			LOG("STATE_CHANGE");
			break;
		case INTERNET_STATUS_COOKIE_SENT:
			LOG("COOKIE_SENT");
			break;
		case INTERNET_STATUS_COOKIE_RECEIVED:
			LOG("COOKIE_RECEIVED");
			break;
		case INTERNET_STATUS_COOKIE_HISTORY:
			LOG("COOKIE_HISTORY");
			break;
		case INTERNET_STATUS_PRIVACY_IMPACTED:
			LOG("PRIVACY_IMPACTED");
			break;
		default:
			LOG("INTERNET STATUS: %i", dwInternetStatus);
			break;
		}
	}
	catch (...)
	{
		LOG("Exception in StatusCallback");
	}
}

void CSimpleUploader::SetListener(const SmartReleasePtr<ISimpleUploadListener> &listener)
{
	try
	{
		m_listener = listener;
	}
	catch (...)
	{
		LOG("Exception in SetListener");
	}
}

bool CSimpleUploader::Upload(const CStringW &strURL, const MultipartFormDataRequest &request)
{
	try
	{
		if (m_state != postUnknown && m_state != postDone && m_state != postError)
			throw (CStringA)FormatString(_T("Invalid state %i"), m_state);

		DoInitialize();

		if (!m_url.CrackUrl((CString)strURL))
			throw (CStringA)"Invalid URL: " + (CStringA)strURL;

		m_strHeaders = request.GetHead(), m_data = request.GetBody(), m_dwPos = 0;

		DoConnect();

		return true;
	}
	catch (CStringA e)
	{
		LOG(e, Internet::GetLastResponseInfo());

		m_state = postError;

		return false;
	}
	catch (...)
	{
		LOG("Exception on upload");

		m_state = postError;

		return false;
	}
}
