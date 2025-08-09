/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#ifdef _WIN32
    #ifndef SCRIPTLIB
        #define SERVER_DECL __declspec(dllexport)
        #define SCRIPT_DECL __declspec(dllimport)
    #else
        #define SERVER_DECL __declspec(dllimport)
        #define SCRIPT_DECL __declspec(dllexport)
    #endif // SCRIPTLIB
    #define DECL_LOCAL
#elif defined(__GNUC__) || defined(__clang__)
    #define SERVER_DECL __attribute__((visibility ("default")))
    #define SCRIPT_DECL __attribute__((visibility ("default")))
    #define DECL_LOCAL __attribute__((visibility ("hidden")))
#else
    #define SERVER_DECL
    #define SCRIPT_DECL
    #define DECL_LOCAL
#endif  // _WIN32
