// Factory.cpp : Implementation of Factory

#include "stdafx.h"
#include "./Factory.h"
#include "./SimpleUploader.h"

namespace UPLOAD {

SmartReleasePtr<ISimpleUploader> Factory::CreateSimpleUploader()
{
    return CSimpleUploader::NewInstance();
}

}//namespace UPLOAD
