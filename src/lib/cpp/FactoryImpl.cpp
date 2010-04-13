// Factory.cpp : Implementation of Factory

#include "stdafx.h"
#include "./Factory.h"
#include "uploader/Factory.h"
#include "./URLResolverImpl.h"

using namespace API::ImageShack;

namespace ImageShack {

UploaderSmartPtr Factory::CreateUploader()
{
    UploaderSmartPtr uploader = UPLOAD::Factory::CreateUploader<UploadInfo, UploadResult>();

    uploader->SetUrlResolver(URLResolver::NewInstance());
    
    return uploader;
}

UploaderListenerSmartPtr Factory::CreateDefaultUploaderListener()
{
    return STUBS::UniversalUploaderListener<UploadInfo, UploadResult>::NewInstance();
}

} //namespace ImageShack
