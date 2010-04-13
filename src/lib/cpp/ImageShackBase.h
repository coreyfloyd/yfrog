#pragma once

#ifdef IMAGESHACKAPI_EXPORTS
    #define IMAGESHACK_API __declspec(dllexport)
#else
    #define IMAGESHACK_API __declspec(dllimport)
#endif
