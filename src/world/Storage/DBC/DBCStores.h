/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#ifdef AE_MOP
#include "../world/GameMop/Storage/DBCStores.h"
#endif

#ifdef AE_CATA
    #include "../world/GameCata/Storage/DBCStores.h"
#endif

#ifdef AE_WOTLK
    #include "../world/GameWotLK/Storage/DBCStores.h"
#endif

#ifdef AE_TBC
    #include "../world/GameTBC/Storage/DBCStores.h"
#endif

#ifdef AE_CLASSIC
    #include "../world/GameClassic/Storage/DBCStores.h"
#endif
