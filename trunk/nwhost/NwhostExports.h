/**
 * @file NwhostExports.h
 *
 * @brief This headerfile declares the imports/exports for the hwhost library
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef NWHOST_EXPORTS_H
#define NWHOST_EXPORTS_H

#ifdef __cplusplus
# define DECORATE extern "C"

#ifdef WIN32
# define CCALL __cdecl
#else
# define CCALL
#endif

#else
# define DECORATE
# define CCALL
#endif 

#ifdef _MSC_VER
    #ifdef NWHOST_API_EXPORT
        #define NWHOST_API __declspec(dllexport)
    #else
        #define NWHOST_API __declspec(dllimport)
    #endif
#else
    #define GEN_API 
#endif // _MSC_VER

#endif //NWHOST_EXPORTS_H

