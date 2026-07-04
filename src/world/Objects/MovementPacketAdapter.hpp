/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "AEVersion.hpp"
#include "MovementCodec.hpp"

#if VERSION_STRING == Classic
    using ActiveMovementCodec = MovementCodec<WoW::Expansion::_Classic>;
#elif VERSION_STRING == TBC
    using ActiveMovementCodec = MovementCodec<WoW::Expansion::_TBC>;
#elif VERSION_STRING == WotLK
    using ActiveMovementCodec = MovementCodec<WoW::Expansion::_WotLK>;
#elif VERSION_STRING == Cata
    using ActiveMovementCodec = MovementCodec<WoW::Expansion::_Cata>;
#elif VERSION_STRING == Mop
    using ActiveMovementCodec = MovementCodec<WoW::Expansion::_Mop>;
#endif
