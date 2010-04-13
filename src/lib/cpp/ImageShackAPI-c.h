#pragma once

#include "ImageShackBase.h"

extern "C" IMAGESHACK_API void __cdecl ImageShackAPI_UploadFileA(LPCSTR pszDevKey, LPCSTR pszSourceFile);
extern "C" IMAGESHACK_API void __cdecl ImageShackAPI_UploadFileW(LPCSTR pszDevKey, LPCWSTR pszSourceFile);

#ifdef _UNICODE
    #define ImageShackAPI_UploadFile ImageShackAPI_UploadFileW
#else
    #define ImageShackAPI_UploadFile ImageShackAPI_UploadFileA
#endif
