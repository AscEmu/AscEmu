/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Management/LFG/LFG.hpp"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgPartyInfo : public ManagedPacket
    {
    public:
        LfgLockPartyMap lockMap;

        SmsgLfgPartyInfo() : SmsgLfgPartyInfo(LfgLockPartyMap{})
        {
        }

        SmsgLfgPartyInfo(LfgLockPartyMap lockMap) :
            ManagedPacket(SMSG_LFG_PARTY_INFO, 0),
            lockMap(std::move(lockMap))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            uint32_t size = 0;
            for (const auto& lock : lockMap)
                size += 8 + 4 + uint32_t(lock.second.size()) * (4 + 4);

            return 1 + size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                // BuildPartyLockDungeonBlock
                packet.writeBits(lockMap.size(), 22);
                for (const auto& lock : lockMap)
                {
                    WoWGuid playerGuid = lock.first;
                    constexpr bool hasGuid = true;

                    packet.writeBit(hasGuid);
                    packet.writeBit(playerGuid[3]);
                    packet.writeBit(playerGuid[6]);
                    packet.writeBit(playerGuid[0]);
                    packet.writeBit(playerGuid[5]);
                    packet.writeBit(playerGuid[2]);
                    packet.writeBit(playerGuid[7]);
                    packet.writeBit(playerGuid[4]);
                    packet.writeBit(playerGuid[1]);
                    packet.writeBits(lock.second.size(), 20);    // Size of lock dungeons

                    packet.flushBits();

                    // BuildPlayerLockDungeonBlock
                    for (const auto& playerLock : lock.second)
                    {
                        packet << uint32_t(0);                   // Current itemLevel (not tracked)
                        packet << uint32_t(playerLock.first);    // Dungeon entry (id + type)
                        packet << uint32_t(0);                   // Required itemLevel (not tracked)
                        packet << uint32_t(playerLock.second);   // Lock status
                    }

                    packet.writeByteSeq(playerGuid[0]);
                    packet.writeByteSeq(playerGuid[3]);
                    packet.writeByteSeq(playerGuid[1]);
                    packet.writeByteSeq(playerGuid[4]);
                    packet.writeByteSeq(playerGuid[6]);
                    packet.writeByteSeq(playerGuid[2]);
                    packet.writeByteSeq(playerGuid[5]);
                    packet.writeByteSeq(playerGuid[7]);
                }
                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                // BuildPartyLockDungeonBlock
                packet << uint8_t(lockMap.size());
                for (const auto& lock : lockMap)
                {
                    packet << uint64_t(lock.first);

                    // BuildPlayerLockDungeonBlock
                    packet << uint32_t(lock.second.size());
                    for (const auto& playerLock : lock.second)
                    {
                        packet << uint32_t(playerLock.first);
                        packet << uint32_t(playerLock.second);
                    }
                }
                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
