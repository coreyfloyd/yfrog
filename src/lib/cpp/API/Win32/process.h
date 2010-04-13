// Process.h: File class.
//////////////////////////////////////////////////////////////////////

#pragma once
#pragma warning (push)
#pragma warning (disable : 4800)

#include <Tlhelp32.h>
#include "Handle.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// Process
//////////////////////////////////////////////////////////////////////

class Process : public WaitableHandle
{
public:
	Process() {}

	Process(HWND hWnd, DWORD dwDesiredAccess = PROCESS_ALL_ACCESS, BOOL bInheritHandle = FALSE)
	{
		Open(hWnd, dwDesiredAccess, bInheritHandle);
	}

	Process(DWORD dwProcessId, DWORD dwDesiredAccess = PROCESS_ALL_ACCESS, BOOL bInheritHandle = FALSE)
	{
		Open(dwProcessId, dwDesiredAccess, bInheritHandle);
	}

	BOOL Open(	DWORD dwProcessId,								// process identifier
				DWORD dwDesiredAccess	= PROCESS_ALL_ACCESS,	// access flag
				BOOL bInheritHandle		= FALSE					// handle inheritance option
			)
	{
		HANDLE hProcess = ::OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);

		Attach(hProcess);

		return IsValid();
	}

	BOOL Open(	HWND hWnd,										// handle to window
				DWORD dwDesiredAccess	= PROCESS_ALL_ACCESS,	// access flag
				BOOL bInheritHandle		= FALSE					// handle inheritance option
			)
	{
		DWORD dwProcessId = 0;

		if (!::GetWindowThreadProcessId(hWnd, &dwProcessId))
			return FALSE;	

		return Open(dwProcessId, dwDesiredAccess, bInheritHandle);
	}

	BOOL GetExitCode(LPDWORD lpExitCode) const
	{
		ATLASSERT(IsValid());

		return ::GetExitCodeProcess(handle, lpExitCode);
	}

	BOOL IsStillActive() const
	{
		DWORD dwExitCode = 0;

		if (!GetExitCode(&dwExitCode))
			return FALSE;

		return dwExitCode == STILL_ACTIVE;
	}

	BOOL Terminate(UINT uExitCode = 0)
	{
		ATLASSERT(IsValid());

		return ::TerminateProcess(handle, uExitCode);
	}
};

//////////////////////////////////////////////////////////////////////
// Process iterator
//////////////////////////////////////////////////////////////////////

class ProcessIterator
{
private:
	Handle			snapshot;
	PROCESSENTRY32	entry;
	BOOL			valid;

public:
	ProcessIterator()
	{
		ZeroMemory(&entry, sizeof(PROCESSENTRY32));
		entry.dwSize = sizeof(PROCESSENTRY32);

		snapshot.Attach( ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0) );
		valid = snapshot.IsValid() ? ::Process32First(snapshot, &entry) : FALSE;
	}

	operator bool () const
	{
		return valid;
	}

	PROCESSENTRY32* operator -> ()
	{
		return &entry;
	}

	PROCESSENTRY32& operator * ()
	{
		return entry;
	}

	void operator ++ ()
	{
		valid = ::Process32Next(snapshot, &entry);
	}

	void operator ++ (int)
	{
		valid = ::Process32Next(snapshot, &entry);
	}
};

#pragma warning (pop)

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
