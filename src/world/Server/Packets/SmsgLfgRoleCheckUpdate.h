/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/LFG/LFGMgr.hpp"
#include "Management/ObjectMgr.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgRoleCheckUpdate : public ManagedPacket
    {
    public:
        uint32_t state;
        LfgDungeonSet dungeons;
        LfgRolesMap roles;
        uint64_t leader;

        SmsgLfgRoleCheckUpdate() : SmsgLfgRoleCheckUpdate(0, {}, {}, 0)
        {
        }

        SmsgLfgRoleCheckUpdate(uint32_t state, LfgDungeonSet dungeons, LfgRolesMap roles, uint64_t leader) :
            ManagedPacket(SMSG_LFG_ROLE_CHECK_UPDATE, 0),
            state(state),
            dungeons(std::move(dungeons)),
            roles(std::move(roles)),
            leader(leader)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 4 + 1 + 1 + dungeons.size() * 4 + 1 + roles.size() * (8 + 1 + 4 + 1);
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // Note: guid is intentionally re-used as a mutable cursor across both the bit-mask
                // and byte phases below
                WoWGuid guid = leader;
                auto itLeaderRoles = roles.find(leader);
                uint8_t leaderRoles = itLeaderRoles != roles.end() ? itLeaderRoles->second : 0;
                Player* player = sObjectMgr.getPlayer(guid.getGuidLowPart());

                packet << uint8_t(state);                                       // RoleCheckStatus
                packet << uint8_t(0);                                           // PartyIndex
                packet.writeBits(roles.size(), 21);                             // Members

                if (!roles.empty())
                {
                    // Leader info MUST be sent 1st
                    packet.writeBit(state == LFG_ROLECHECK_FINISHED);           // RoleCheckComplete
                    packet.writeBit(guid[3]);
                    packet.writeBit(guid[0]);
                    packet.writeBit(guid[5]);
                    packet.writeBit(guid[2]);
                    packet.writeBit(guid[7]);
                    packet.writeBit(guid[1]);
                    packet.writeBit(guid[4]);
                    packet.writeBit(guid[6]);

                    for (const auto& rolePair : roles)
                    {
                        if (rolePair.first == leader)
                            continue;

                        guid = rolePair.first;
                        packet.writeBit(state == LFG_ROLECHECK_FINISHED);       // RoleCheckComplete
                        packet.writeBit(guid[3]);
                        packet.writeBit(guid[0]);
                        packet.writeBit(guid[5]);
                        packet.writeBit(guid[2]);
                        packet.writeBit(guid[7]);
                        packet.writeBit(guid[1]);
                        packet.writeBit(guid[4]);
                        packet.writeBit(guid[6]);
                    }
                }

                packet.writeBit(guid[3]);
                packet.writeBit(guid[5]);
                packet.writeBits(dungeons.size(), 22);                          // JoinSlots
                packet.writeBit(guid[0]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);
                packet.writeBit(state == LFG_ROLECHECK_INITIALITING);           // IsBeginning

                packet.flushBits();

                packet.writeByteSeq(guid[0]);

                if (!roles.empty())
                {
                    // Leader info MUST be sent 1st
                    packet << uint8_t(player ? player->getLevel() : 0);         // Level
                    packet.writeByteSeq(guid[3]);
                    packet.writeByteSeq(guid[6]);
                    packet << uint32_t(leaderRoles);                            // RolesDesired
                    packet.writeByteSeq(guid[2]);
                    packet.writeByteSeq(guid[4]);
                    packet.writeByteSeq(guid[0]);
                    packet.writeByteSeq(guid[1]);
                    packet.writeByteSeq(guid[5]);
                    packet.writeByteSeq(guid[7]);

                    for (const auto& rolePair : roles)
                    {
                        if (rolePair.first == leader)
                            continue;

                        guid = rolePair.first;
                        Player* member = sObjectMgr.getPlayer(guid.getGuidLowPart());

                        packet << uint8_t(member ? member->getLevel() : 0);     // Level
                        packet.writeByteSeq(guid[3]);
                        packet.writeByteSeq(guid[6]);
                        packet << uint32_t(rolePair.second);                    // RolesDesired
                        packet.writeByteSeq(guid[2]);
                        packet.writeByteSeq(guid[4]);
                        packet.writeByteSeq(guid[0]);
                        packet.writeByteSeq(guid[1]);
                        packet.writeByteSeq(guid[5]);
                        packet.writeByteSeq(guid[7]);
                    }
                }

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[5]);

                if (!dungeons.empty())
                {
                    for (auto dungeonEntry : dungeons)
                    {
                        auto dungeon = sLFGDungeonStore.lookupEntry(dungeonEntry);
                        packet << uint32_t(dungeon ? dungeon->Entry() : 0);     // Dungeon
                    }
                }

                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << uint32_t(state);                                          // Check result
                packet << uint8_t(state == LFG_ROLECHECK_INITIALITING);
                packet << uint8_t(dungeons.size());                                 // Number of dungeons

                if (m_protocol.expansion < WoW::Expansion::_Cata)
                {
                    if (!dungeons.empty())
                    {
                        for (auto dungeonEntry : dungeons)
                        {
                            auto dungeon = sLFGDungeonStore.lookupEntry(dungeonEntry);
                            packet << uint32_t(dungeon ? dungeon->Entry() : 0);
                        }
                    }
                }

                packet << uint8_t(roles.size());                                    // Players in group
                if (!roles.empty())
                {
                    // Leader info MUST be sent 1st :S
                    uint64_t guid = leader;
                    uint8_t playerRoles = roles.find(guid)->second;
                    packet << uint64_t(guid);                                       // Guid
                    packet << uint8_t(playerRoles > 0);                             // Ready
                    packet << uint32_t(playerRoles);                                // Roles

                    WoWGuid wowGuid;
                    wowGuid.init(guid);

                    Player* player = sObjectMgr.getPlayer(wowGuid.getGuidLowPart());
                    packet << uint8_t(player ? player->getLevel() : 0);             // Level

                    for (const auto rolePair : roles)
                    {
                        if (rolePair.first == leader)
                            continue;

                        WoWGuid guidItr;
                        guidItr.init(rolePair.first);

                        guid = rolePair.first;
                        playerRoles = rolePair.second;
                        packet << uint64_t(guid);                                   // Guid
                        packet << uint8_t(playerRoles > 0);                         // Ready
                        packet << uint32_t(playerRoles);                            // Roles

                        player = sObjectMgr.getPlayer(guidItr.getGuidLowPart());
                        packet << uint8_t(player ? player->getLevel() : 0);         // Level
                    }
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
