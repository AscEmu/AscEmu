/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../world/WorldConf.h"

#if VERSION_STRING == Classic
    #include "../world/GameClassic/UpdateFields.h"
#elif VERSION_STRING == TBC
    #include "../world/GameTBC/UpdateFields.h"
#elif VERSION_STRING == WotLK
    #include "../world/GameWotLK/UpdateFields.h"
#elif VERSION_STRING == Cata
    #include "../world/GameCata/UpdateFields.h"
#endif
