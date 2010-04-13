#pragma once

#include "ImageShackAPI.h"

namespace ImageShack {

    using namespace API::ImageShack;

/**
 * Class Factory of ImageShack uploader
 *
 * @author Alexander Kozlov
 */
class IMAGESHACK_API Factory
{
public:
    /**
     * Creates configured Uploader
     */
    static UploaderSmartPtr CreateUploader();
    static UploaderListenerSmartPtr CreateDefaultUploaderListener();
};

} // namespace ImageShack
