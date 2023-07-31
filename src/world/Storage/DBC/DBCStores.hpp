/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "WorldConf.h"

#ifdef AE_MOP
#include <GameMop/Storage/DBCStores.hpp>
#endif

#ifdef AE_CATA
    #include <GameCata/Storage/DBCStores.hpp>
#endif

#ifdef AE_WOTLK
    #include <GameWotLK/Storage/DBCStores.hpp>
#endif

#ifdef AE_TBC
    #include <GameTBC/Storage/DBCStores.hpp>
#endif

#ifdef AE_CLASSIC
    #include <GameClassic/Storage/DBCStores.hpp>
#endif
