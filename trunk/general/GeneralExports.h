/**
 * @file libgeneralExports.h
 *
 * @brief This headerfile declares the imports/exports for the libgeneral library
 * @author Jan Wilmans <jw@dds.nl>  Copyright (C) 2009 Nowind
 *
 */
#ifndef LIBGENERAL_EXPORTS_H
#define LIBGENERAL_EXPORTS_H

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
    #ifdef GEN_API_EXPORT
        #define GEN_API __declspec(dllexport)
    #else
        #define GEN_API __declspec(dllimport)
    #endif
#else
    #define GEN_API 
#endif /// defined _MSC_VER

#endif //LIBGENERAL_EXPORTS_H

