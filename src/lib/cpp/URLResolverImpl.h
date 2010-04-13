#pragma once

#include "uploader/URLResolver.h"
#include "ImageShackAPI.h"
#include "http/ServerResolver.h"

namespace ImageShack {
    using namespace API::ImageShack;
    using namespace UPLOAD;

/**
 * ImageShack URL Resolver implementation.
 *
 * @author Alexander Kozlov
 */
class URLResolver : public IUniversalUploaderURLResolver<UploadInfo>
{
private:
	CServerResolver m_image_resolver;
	CServerResolver m_video_resolver;

public:
    URLResolver();
    ~URLResolver();

public:
    static SmartReleasePtr<UPLOAD::IUniversalUploaderURLResolver<API::ImageShack::UploadInfo> > NewInstance()
    {
        return SmartReleasePtr<UPLOAD::IUniversalUploaderURLResolver<API::ImageShack::UploadInfo> >(new URLResolver());
    }

public:
    /**
     * Must return URL for POST data request
     */
    virtual CStringW GetURL(const UploadInfo &item);

    /**
	 * Called to destroy
	 */
	virtual void Release()
    {
        delete this;
    }
};

}//namespace ImageShack

