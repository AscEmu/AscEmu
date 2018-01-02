/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Map/Area/AreaManagementGlobals.hpp"
#include "WorldConf.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Wow build version.
#if VERSION_STRING == Classic
    #define BUILD_VERSION 5875
#elif VERSION_STRING == TBC
    #define BUILD_VERSION 8606
#elif VERSION_STRING == WotLK
    #define BUILD_VERSION 12340
#elif VERSION_STRING == Cata
    #define BUILD_VERSION 15595
#endif

namespace MapManagement
{

} // </ MapManagementNamespace>
