/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "../ValuesUpdate.hpp"
#include "../../ProtocolLayout.hpp"

#include <array>
#include <cstdint>

namespace AscEmu::Version::Forever::UpdateFields::Definitions
{
    struct PresenceBit
    {
        bool ValuesUpdatePresence::* member;
        uint8_t objectTypeBit;
        bool ownerOnly;
    };

    inline constexpr std::array<PresenceBit, 9> ValuesPresenceBits{{
        { &ValuesUpdatePresence::objectData, ProtocolLayout::ObjectType::Object, false },
        { &ValuesUpdatePresence::itemData, ProtocolLayout::ObjectType::Item, false },
        { &ValuesUpdatePresence::containerData, ProtocolLayout::ObjectType::Container, false },
        { &ValuesUpdatePresence::unitData, ProtocolLayout::ObjectType::Unit, false },
        { &ValuesUpdatePresence::playerData, ProtocolLayout::ObjectType::Player, false },
        { &ValuesUpdatePresence::activePlayerData, ProtocolLayout::ObjectType::ActivePlayer, true },
        { &ValuesUpdatePresence::gameObjectData, ProtocolLayout::ObjectType::GameObject, false },
        { &ValuesUpdatePresence::dynamicObjectData, ProtocolLayout::ObjectType::DynamicObject, false },
        { &ValuesUpdatePresence::corpseData, ProtocolLayout::ObjectType::Corpse, false }
    }};
}
