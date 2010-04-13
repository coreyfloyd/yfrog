// ProcessEx.h: ProcessEx class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Process.h"
#include "Thread.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// ProcessEx
//////////////////////////////////////////////////////////////////////

class ProcessEx : public Process
{
public:
	STARTUPINFO			si;
	PROCESS_INFORMATION pi;

	Thread				thread;

public:
	ProcessEx()
	{
		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );
	}

	virtual ~ProcessEx()
	{
		Close();
	}

public:
	BOOL Create(LPCTSTR lpApplicationName,                 // name of executable module
				LPTSTR lpCommandLine		= NULL,        // command line string
				LPCTSTR lpCurrentDirectory	= NULL,         // current directory name
				DWORD dwCreationFlags		= 0,           // creation flags
				LPSECURITY_ATTRIBUTES lpProcessAttributes	= NULL, // SD
				LPSECURITY_ATTRIBUTES lpThreadAttributes	= NULL, // SD
				BOOL bInheritHandles		= FALSE,       // handle inheritance option
				LPVOID lpEnvironment		= NULL         // new environment block
			)
	{
		Close();

		// Start the child process.
		if (!CreateProcess(lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, &si, &pi))
			return FALSE;

		Attach(pi.hProcess);
		thread.Attach(pi.hThread, pi.dwThreadId);

		return TRUE;
	}

	BOOL CreateAsUser(HANDLE hToken,
					  LPCTSTR lpApplicationName,
					  LPTSTR lpCommandLine = NULL,
					  LPCTSTR lpCurrentDirectory = NULL,
					  DWORD dwCreationFlags = 0,
					  LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL,
					  LPSECURITY_ATTRIBUTES lpThreadAttributes = NULL,
					  BOOL bInheritHandles = FALSE,
					  LPVOID lpEnvironment = NULL
					)
	{
		Close();

		// Start the child process as USER.
		if (!CreateProcessAsUser(hToken,lpApplicationName,lpCommandLine,lpProcessAttributes,lpThreadAttributes,bInheritHandles,dwCreationFlags,lpEnvironment,lpCurrentDirectory, &si, &pi))
			return FALSE;

		Attach(pi.hProcess);
		thread.Attach(pi.hThread, pi.dwThreadId);

		return TRUE;
	}

	virtual void Close()
	{ 
		thread.Close();
		Process::Close();
	}
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
