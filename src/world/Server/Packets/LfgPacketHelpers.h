/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "Management/LFG/LFG.hpp"
#include "Network/WorldPacket.hpp"
#include "Server/ClientProtocol.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "WoWGuid.hpp"

#include <cstdint>

namespace AscEmu::Packets::Lfg
{
    // 6.x+ queue ticket: requester, queue id, type 2 (dungeon finder), join time; 9.x+ closes it with the cross faction bit
    inline void writeRideTicket(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t receiverMapId, uint64_t requester, uint32_t id, uint32_t time)
    {
        packet << WoWGuid(requester).toGuid128(protocol.realmId, receiverMapId);
        packet << uint32_t(id);
        packet << uint32_t(2);
        packet << uint32_t(time);
        if (protocol.expansion >= WoW::Expansion::_Shadowlands)
        {
            packet.writeBit(false);
            packet.flushBits();
        }
    }

    inline void readRideTicket(WorldPacket& packet, WoW::ClientProtocol const& protocol, WoWGuid& requester, uint32_t& id, uint32_t& time)
    {
        WoWGuid128 guid;
        uint32_t type = 0;
        packet >> guid >> id >> type >> time;
        requester = WoWGuid::fromGuid128(guid);
        if (protocol.expansion >= WoW::Expansion::_Shadowlands)
        {
            packet.readBit();
            packet.resetBitPos();
        }
    }

    // locked dungeons of one player: guid bit, slot count, guid, then slot, reason and the two item levels (9.x+ adds the soft lock)
    inline void writeBlackList(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t receiverMapId, uint64_t guid, LfgLockMap const& lock)
    {
        packet.writeBit(guid != 0);
        packet << uint32_t(lock.size());
        if (guid != 0)
            packet << WoWGuid(guid).toGuid128(protocol.realmId, receiverMapId);

        for (const auto& [slot, reason] : lock)
        {
            packet << uint32_t(slot);
            packet << uint32_t(reason);
            packet << int32_t(0);
            packet << int32_t(0);
            if (protocol.expansion >= WoW::Expansion::_Shadowlands)
                packet << uint32_t(0);
        }
    }

    // roles travel as a byte from 10.x on, as uint32 before
    inline void writeRoles(WorldPacket& packet, WoW::ClientProtocol const& protocol, uint32_t roles)
    {
        if (protocol.expansion >= WoW::Expansion::_Dragonflight)
            packet << uint8_t(roles);
        else
            packet << uint32_t(roles);
    }

    // the client names a dungeon by id and type
    inline uint32_t dungeonEntry(uint32_t dungeonId)
    {
        if (auto const* dungeon = sLFGDungeonStore.lookupEntry(dungeonId & 0x00FFFFFF))
            return dungeon->Entry();

        return dungeonId;
    }

    // join results of 4.x+ clients: the block behind the group full value starts at a version specific base, 5.x has no gap behind it
    inline uint8_t clientJoinResult(WoW::ClientProtocol const& protocol, uint32_t result)
    {
        if (protocol.expansion < WoW::Expansion::_Cata || result == 0 || result == 6)
            return uint8_t(result);

        uint8_t base = 0x1F;
        bool gap = true;
        switch (protocol.expansion)
        {
            case WoW::Expansion::_Cata: base = 0x1C; break;
            case WoW::Expansion::_Mop: base = 0x1E; gap = false; break;
            case WoW::Expansion::_WoD: base = 0x1E; break;
            case WoW::Expansion::_Legion: base = 0x1D; break;
            default: break;
        }

        uint8_t offset = 0;
        switch (result)
        {
            case 1: offset = 15; break;         // role check failed
            case 2: offset = 0; break;          // group full
            case 4: offset = 2; break;          // internal error
            case 5: offset = 3; break;          // requirements not met
            default: offset = uint8_t(result >= 7 && result <= 17 ? result - 3 : 2); break;
        }

        if (!gap && offset >= 2)
            --offset;

        return uint8_t(base + offset);
    }

    // update types of 4.x+ clients, 8.x inserts the suspended queue value
    inline uint8_t clientUpdateType(WoW::ClientProtocol const& protocol, uint32_t type)
    {
        if (protocol.expansion < WoW::Expansion::_Cata)
            return uint8_t(type);

        const bool shifted = protocol.expansion >= WoW::Expansion::_BfA;
        switch (type)
        {
            case LFG_UPDATETYPE_JOIN_PROPOSAL: return 6;
            case LFG_UPDATETYPE_ROLECHECK_FAILED: return 7;
            case LFG_UPDATETYPE_REMOVED_FROM_QUEUE: return 8;
            case LFG_UPDATETYPE_PROPOSAL_FAILED: return 9;
            case LFG_UPDATETYPE_PROPOSAL_DECLINED: return 10;
            case LFG_UPDATETYPE_GROUP_FOUND: return 11;
            case LFG_UPDATETYPE_ADDED_TO_QUEUE: return 13;
            case LFG_UPDATETYPE_PROPOSAL_BEGIN: return shifted ? 15 : 14;
            case LFG_UPDATETYPE_CLEAR_LOCK_LIST: return shifted ? 16 : 15;
            case LFG_UPDATETYPE_GROUP_MEMBER_OFFLINE: return shifted ? 17 : 16;
            case LFG_UPDATETYPE_GROUP_DISBAND: return shifted ? 18 : 17;
            default: return uint8_t(type);
        }
    }
}
