/**
 * @file NowindExports.h
 *
 * @brief This headerfile declares the imports/exports for the general library
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef LIBNOWIND_EXPORTS_H
#define LIBNOWIND_EXPORTS_H

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
    #ifdef NOWIND_API_EXPORT
        #define NOWIND_API __declspec(dllexport)
    #else
        #define NOWIND_API __declspec(dllimport)
    #endif
#else
    #define NOWIND_API 
#endif // _MSC_VER

#endif //LIBNOWIND_EXPORTS_H

