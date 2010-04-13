#include "StdAfx.h"
#include "ImageShackAPI.h"
using namespace API::ImageShack;

extern "C" IMAGESHACK_API void ImageShackAPI_UploadFileA(LPCSTR pszDevKey, LPCSTR pszSourceFile)
{
    ImageShackAPI api(pszDevKey);

    UploadInfo info((CStringW)pszSourceFile);

    api.UploadFiles(&info, 1);
}

extern "C" IMAGESHACK_API void ImageShackAPI_UploadFileW(LPCSTR pszDevKey, LPCWSTR pszSourceFile)
{
    ImageShackAPI api(pszDevKey);

    UploadInfo info((CStringW)pszSourceFile);

    api.UploadFiles(&info, 1);
}
