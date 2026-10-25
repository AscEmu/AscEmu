/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Network/WorldPacket.hpp"
#include "Server/ClientProtocol.hpp"
#include "WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the item bonus block: context and the bonus list ids
    inline void skipItemBonuses(WorldPacket& packet)
    {
        packet.readSkip<uint8_t>();
        uint32_t count = 0;
        packet >> count;
        for (uint32_t i = 0; i < count && !packet.hadReadFailure(); ++i)
            packet.readSkip<uint32_t>();
    }

    // the item instance a 6.x+ client sends: the id is kept, bonuses and modifiers are read past
    inline void readItemInstance(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t& itemId)
    {
        packet >> itemId;

        if (protocol.isWoD() || protocol.isLegion() || protocol.isBfA())
        {
            // 6.x - 8.x: random properties (6.x, 7.x), then the bonus and modifier flags; modifiers come as a mask of values
            if (!protocol.isBfA())
            {
                packet.readSkip<int32_t>();         // random properties seed
                packet.readSkip<int32_t>();         // random properties
            }
            const bool hasBonus = packet.readBit();
            const bool hasModifications = packet.readBit();
            packet.resetBitPos();
            if (hasBonus)
                skipItemBonuses(packet);
            if (hasModifications)
            {
                uint32_t mask = 0;
                packet >> mask;
                for (; mask != 0 && !packet.hadReadFailure(); mask >>= 1)
                    if (mask & 1)
                        packet.readSkip<int32_t>();
            }
            return;
        }

        if (protocol.isMidnight())
        {
            // 12.x: the modifier list first (7 bit count), then the bonus flag
            const uint32_t modifications = packet.readBits(7);
            packet.resetBitPos();
            for (uint32_t i = 0; i < modifications && !packet.hadReadFailure(); ++i)
            {
                packet.readSkip<uint8_t>();         // type
                packet.readSkip<int32_t>();         // value
            }
            const bool hasBonus = packet.readBit();
            packet.resetBitPos();
            if (hasBonus)
                skipItemBonuses(packet);
            return;
        }

        // 9.x - 11.x: the bonus flag, the modifier list (6 bit count), then the bonuses; 11.x turns the modifier around
        const bool hasBonus = packet.readBit();
        packet.resetBitPos();
        const uint32_t modifications = packet.readBits(6);
        packet.resetBitPos();
        for (uint32_t i = 0; i < modifications && !packet.hadReadFailure(); ++i)
        {
            if (protocol.isTheWarWithinOrLater())
            {
                packet.readSkip<uint8_t>();         // type
                packet.readSkip<int32_t>();         // value
            }
            else
            {
                packet.readSkip<int32_t>();         // value
                packet.readSkip<uint8_t>();         // type
            }
        }
        if (hasBonus)
            skipItemBonuses(packet);
    }

    // an item instance without bonuses and modifiers for a 6.x+ client
    inline void writeItemInstance(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t itemId, int32_t randomProperties = 0, int32_t propertySeed = 0)
    {
        packet << itemId;

        if (protocol.isWoD() || protocol.isLegion() || protocol.isBfA())
        {
            if (!protocol.isBfA())
                packet << propertySeed << randomProperties;
            packet.writeBit(false);                 // item bonus
            packet.writeBit(false);                 // modifications
            packet.flushBits();
            return;
        }

        if (protocol.isMidnight())
        {
            packet.writeBits(0, 7);                 // modifications
            packet.flushBits();
            packet.writeBit(false);                 // item bonus
            packet.flushBits();
            return;
        }

        packet.writeBit(false);                     // item bonus
        packet.flushBits();
        packet.writeBits(0, 6);                     // modifications
        packet.flushBits();
    }

    // a guid of a 6.x+ client: packed on 6.x, 128 bit from 7.x
    inline WoWGuid readClientGuid(WorldPacket& packet, WoW::ClientProtocol const& protocol)
    {
        if (protocol.isWoD())
            return WoWGuid(packet.unpackGuid());

        WoWGuid128 guid;
        packet >> guid;
        return WoWGuid::fromGuid128(guid);
    }

    inline void writeClientGuid(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t receiverMapId, uint64_t guid)
    {
        if (protocol.isWoD())
            packet.appendPackGuid(guid);
        else
            packet << WoWGuid(guid).toGuid128(protocol.realmId, receiverMapId);
    }
}
