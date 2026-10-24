/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/Battleground/BattlegroundDefines.hpp"
#include "Network/WorldPacket.hpp"
#include "Server/ClientProtocol.hpp"
#include "Server/World.h"
#include "WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets::Battlegrounds
{
    // 6.x+ queue ticket: requester (packed on 6.x, 128 bit from 7.x), queue slot, type 1 (battlegrounds), join time;
    // 9.x+ closes it with the cross faction bit
    inline void writeTicket(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t receiverMapId, uint64_t requester, uint32_t slot, uint32_t time)
    {
        if (protocol.isWoD())
            packet.appendPackGuid(requester);
        else
            packet << WoWGuid(requester).toGuid128(protocol.realmId, receiverMapId);

        packet << uint32_t(slot);
        packet << uint32_t(1);
        packet << uint32_t(time);
        if (protocol.expansion >= WoW::Expansion::_Shadowlands)
        {
            packet.writeBit(false);
            packet.flushBits();
        }
    }

    inline void readTicket(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t& slot, uint32_t& time)
    {
        if (protocol.isWoD())
        {
            packet.unpackGuid();
        }
        else
        {
            WoWGuid128 requester;
            packet >> requester;
        }

        uint32_t type = 0;
        packet >> slot >> type >> time;
        if (protocol.expansion >= WoW::Expansion::_Shadowlands)
        {
            packet.readBit();
            packet.resetBitPos();
        }
    }

    // players per team of an arena queue, 0 for a battleground
    inline uint8_t teamSize(uint32_t type)
    {
        switch (type)
        {
            case BattlegroundDef::TYPE_ARENA_2V2: return 2;
            case BattlegroundDef::TYPE_ARENA_3V3: return 3;
            case BattlegroundDef::TYPE_ARENA_5V5: return 5;
            default: return 0;
        }
    }

    // the queue id of 6.x+ clients: battlemaster list, queue type, rated flag and team size under the battleground guid type
    inline uint64_t queueId(uint32_t type, bool rated)
    {
        const bool arena = teamSize(type) != 0;
        return uint64_t(arena ? 6 : type)
            | (uint64_t(arena ? 1 : 0) << 16)
            | (uint64_t(rated ? 1 : 0) << 20)
            | (uint64_t(teamSize(type)) << 24)
            | (uint64_t(0x1F10) << 48);
    }

    // status header: ticket, queue id (a list from 8.x on), level range, team size, instance and the rated bits
    inline void writeStatusHeader(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t receiverMapId, uint64_t requester, uint32_t slot, uint32_t time, uint32_t type, uint32_t instanceId, bool rated)
    {
        writeTicket(packet, protocol, receiverMapId, requester, slot, time);

        const bool list = protocol.expansion >= WoW::Expansion::_BfA;
        if (list)
            packet << uint32_t(1);
        else
            packet << uint64_t(queueId(type, rated));

        packet << uint8_t(0);                                                       // range min
        packet << uint8_t(worldConfig.player.playerLevelCap);                       // range max
        packet << uint8_t(teamSize(type) != 0 ? teamSize(type) : (protocol.isWoD() ? 1 : 0));
        packet << uint32_t(instanceId);
        if (list)
            packet << uint64_t(queueId(type, rated));

        packet.writeBit(rated);                                                     // registered match
        packet.writeBit(false);                                                     // tournament rules
        packet.flushBits();
    }
}
