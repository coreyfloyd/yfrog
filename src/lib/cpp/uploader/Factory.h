#pragma once

#include "ImageShackBase.h"
#include "Uploader.h"
#include "UniversalUploader.h"
#include "URLResolver.h"

namespace UPLOAD {

    /**
     * Factory to create instance of UniversalUploader and etc.
     * 
     * @author Alexander Kozlov
     */
    class IMAGESHACK_API Factory
    {
    public:
        static SmartReleasePtr<ISimpleUploader> CreateSimpleUploader();

        template <typename TItem>
        static SmartReleasePtr<IUniversalUploaderURLResolver<TItem> > CreateURLResolver(LPCTSTR pszURL)
        {
            return URLResolver<TItem>::NewInstance(pszURL);
        }

        template <typename TItem, typename TResult>
        static SmartReleasePtr< IUniversalUploader<TItem, TResult> > CreateUploader()
        {
            return UniversalUploader<TItem, TResult>::NewInstance();
        }
    };
}//namespace UPLOAD
