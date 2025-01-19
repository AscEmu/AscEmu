/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "CommonTypes.hpp"

#ifdef SCRIPTLIB

#include <git_version.hpp>

extern "C" SCRIPT_DECL const char* _exp_get_version()
{
    return AE_BUILD_HASH;
}

#endif
