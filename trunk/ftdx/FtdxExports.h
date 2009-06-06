/**
 * @file ftdxExports.h
 *
 * @brief This headerfile declares the imports/exports for the ftdx library
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef LIBFTDX_EXPORTS_H
#define LIBFTDX_EXPORTS_H

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
    #ifdef FTDX_API_EXPORT
        #define FTDX_API __declspec(dllexport)
    #else
        #define FTDX_API __declspec(dllimport)
    #endif
#else
    #define FTDX_API 
#endif // _MSC_VER

#endif // LIBFTDX_EXPORTS_H

