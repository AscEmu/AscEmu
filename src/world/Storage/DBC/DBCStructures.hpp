/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#ifdef AE_CATA
    #include "../world/GameCata/Storage/DBCStructures.h"
#endif

#ifdef AE_WOTLK
#include "../world/GameWotLK/Storage/DBCStructures.h"
#endif

#ifdef AE_TBC
    #include "../world/GameTBC/Storage/DBCStructures.h"
#endif

#ifdef AE_CLASSIC
    #include "../world/GameClassic/Storage/DBCStructures.h"
#endif
