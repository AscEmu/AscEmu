/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

namespace AscEmu::Version::Forever::BroadcastTextId
{
    constexpr uint32_t TypeBits = 6U;
    constexpr uint32_t SourceIdBits = 32U - TypeBits;
    constexpr uint32_t TypeShift = SourceIdBits;

    constexpr uint32_t TypeMask = 0xFC000000U;
    constexpr uint32_t SourceIdMask = 0x03FFFFFFU;
    constexpr uint32_t MaxSourceId = SourceIdMask;

    enum class Type : uint8_t
    {
        Native = 0,
        Gossip = 1,
        CreatureText = 2,
        QuestText = 3,
        ScriptText = 4,
        GameObjectText = 5,
    };

    constexpr bool isValidSourceId(uint32_t sourceId)
    {
        return sourceId <= MaxSourceId;
    }

    constexpr uint32_t encode(Type type, uint32_t sourceId)
    {
        return (static_cast<uint32_t>(type) << TypeShift) | (sourceId & SourceIdMask);
    }

    constexpr Type getType(uint32_t encodedId)
    {
        return static_cast<Type>((encodedId & TypeMask) >> TypeShift);
    }

    constexpr uint32_t getSourceId(uint32_t encodedId)
    {
        return encodedId & SourceIdMask;
    }

    constexpr bool isNative(uint32_t encodedId)
    {
        return getType(encodedId) == Type::Native;
    }
}
