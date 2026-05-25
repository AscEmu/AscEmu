/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#if defined(_WIN32)
    #if defined(SERVER_EXPORTS)
        #define SERVER_DECL __declspec(dllexport)
    #else
        #define SERVER_DECL __declspec(dllimport)
    #endif
    #if defined(SCRIPTLIB)
        #define SCRIPT_DECL __declspec(dllexport)
    #else
        #define SCRIPT_DECL __declspec(dllimport)
    #endif
    #define DECL_LOCAL
#elif defined(__GNUC__) || defined(__clang__)
    #define SERVER_DECL __attribute__((visibility ("default")))
    #define SCRIPT_DECL __attribute__((visibility ("default")))
    #define DECL_LOCAL __attribute__((visibility ("hidden")))
#else
    #define SERVER_DECL
    #define SCRIPT_DECL
    #define DECL_LOCAL
#endif
