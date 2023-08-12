/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#ifdef SCRIPTLIB

#include <git_version.h>

extern "C" SCRIPT_DECL const char* _exp_get_version()
{
    return BUILD_HASH_STR;
}

#endif
