// Library.h: DLL API.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Handle.h"

namespace API {
namespace Win32 {

//////////////////////////////////////////////////////////////////////
// Library
//////////////////////////////////////////////////////////////////////

class Library : public THandle<HMODULE>
{
public:
	Library(void)
	{
	}

	Library(LPCTSTR lpFileName)
	{
		LoadLibrary(lpFileName);
	}

	virtual ~Library(void)
	{
		Close();
	}

public:

	// Implements Closable
	virtual void Close()
	{ 
		if (IsValid()) ::FreeLibrary(handle), handle = NULL; 
	}

public:
	BOOL LoadLibrary(LPCTSTR lpFileName)
	{
		HMODULE hModule = ::LoadLibrary(lpFileName);

		Attach(hModule);

		return IsValid();
	}

	BOOL LoadLibraryEx(LPCTSTR lpFileName, HANDLE hFile, DWORD dwFlags)
	{
		HMODULE hModule = ::LoadLibraryEx(lpFileName, hFile, dwFlags);

		Attach(hModule);

		return IsValid();
	}

	BOOL DisableThreadLibraryCalls()
	{
		ATLASSERT(IsValid());

		return ::DisableThreadLibraryCalls(handle);
	}

	void FreeLibraryAndExitThread(DWORD dwExitCode = 0)
	{
		ATLASSERT(IsValid());

		::FreeLibraryAndExitThread(handle, dwExitCode);
	}

	DWORD GetModuleFileName(LPTSTR lpFilename, DWORD nSize)
	{
		ATLASSERT(IsValid());

		return ::GetModuleFileName(handle, lpFilename, nSize);
	}

	FARPROC GetProcAddress(LPCSTR lpProcName)
	{
		ATLASSERT(IsValid());

		return ::GetProcAddress(handle, lpProcName);
	}

public:
	// static members
#if _WIN32_WINNT >= 0x0502
	static void GetDllDirectory(DWORD nBufferLength, LPTSTR lpBuffer)
	{
		::GetDllDirectory(nBufferLength, lpBuffer);
	}

	static void SetDllDirectory(LPCTSTR lpPathName)
	{
		::SetDllDirectory(lpPathName);
	}
#endif
	static DWORD GetCurrentModuleFileName(LPTSTR lpFilename, DWORD nSize)
	{
		return ::GetModuleFileName(NULL, lpFilename, nSize);
	}

	static HMODULE GetModuleHandle(LPCTSTR lpModuleName)
	{
		return GetModuleHandle(lpModuleName);
	}
};

//////////////////////////////////////////////////////////////////////


}; //namespace Win32
}; //namespace API
