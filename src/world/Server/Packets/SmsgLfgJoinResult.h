/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "LfgPacketHelpers.h"
#include "Management/LFG/LFG.hpp"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgJoinResult : public ManagedPacket
    {
    public:
        uint32_t result;
        uint32_t state;
        LfgLockPartyMap lockmap;
        WoWGuid guid;
        // 6.x+: queue ticket
        uint32_t ticketId = 0;
        uint32_t ticketTime = 0;

        SmsgLfgJoinResult() : SmsgLfgJoinResult(0, 0, {}, 0)
        {
        }

        SmsgLfgJoinResult(uint32_t result, uint32_t state, LfgLockPartyMap lockmap, uint64_t guid = 0) :
            ManagedPacket(SMSG_LFG_JOIN_RESULT, 0),
            result(result),
            state(state),
            lockmap(std::move(lockmap)),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            uint32_t size = 0;
            for (const auto& lock : lockmap)
                size += 8 + 4 + uint32_t(lock.second.size()) * (4 + 4);

            return 4 + 4 + size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                const auto expansion = m_protocol.expansion;
                Lfg::writeRideTicket(packet, m_protocol, m_receiverMapId, guid.getRawGuid(), ticketId, ticketTime);
                if (expansion >= WoW::Expansion::_TheWarWithin)
                    packet << int32_t(Lfg::clientJoinResult(m_protocol, result));
                else
                    packet << uint8_t(Lfg::clientJoinResult(m_protocol, result));
                packet << uint8_t(state);                                       // result detail
                packet << uint32_t(lockmap.size());
                if (expansion >= WoW::Expansion::_BfA)
                    packet << uint32_t(0);                                      // names of players missing requirements

                for (const auto& lock : lockmap)
                {
                    // 6.x and 8.x name the player without the guid bit and without the soft lock
                    if (expansion == WoW::Expansion::_WoD || expansion == WoW::Expansion::_BfA)
                    {
                        packet << WoWGuid(lock.first).toGuid128(m_protocol.realmId, m_receiverMapId);
                        packet << uint32_t(lock.second.size());
                        for (const auto& playerLock : lock.second)
                        {
                            packet << uint32_t(playerLock.first);
                            packet << uint32_t(playerLock.second);
                            packet << int32_t(0);
                            packet << int32_t(0);
                        }
                    }
                    else
                    {
                        Lfg::writeBlackList(packet, m_protocol, m_receiverMapId, lock.first, lock.second);
                    }
                }
                return true;
            }
            if (m_protocol.isMop())
            {
                packet.writeBit(guid[7]);
                packet.writeBit(guid[6]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.writeBits(lockmap.size(), 22);            // BlackList

                for (const auto& lock : lockmap)
                {
                    WoWGuid playerGuid = lock.first;
                    packet.writeBit(playerGuid[3]);
                    packet.writeBits(lock.second.size(), 20);    // Slots
                    packet.writeBit(playerGuid[6]);
                    packet.writeBit(playerGuid[1]);
                    packet.writeBit(playerGuid[4]);
                    packet.writeBit(playerGuid[7]);
                    packet.writeBit(playerGuid[2]);
                    packet.writeBit(playerGuid[0]);
                    packet.writeBit(playerGuid[5]);
                }

                packet.writeBit(guid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[4]);
                packet.writeBit(guid[2]);

                packet.flushBits();

                packet << uint8_t(Lfg::clientJoinResult(m_protocol, result));                       // Result

                for (const auto& lock : lockmap)
                {
                    WoWGuid playerGuid = lock.first;
                    packet.writeByteSeq(playerGuid[4]);

                    for (const auto& playerLock : lock.second)
                    {
                        packet << uint32_t(0);                   // SubReason2 (required itemLevel - not tracked)
                        packet << uint32_t(0);                   // SubReason1 (current itemLevel - not tracked)
                        packet << uint32_t(playerLock.second);   // Reason
                        packet << uint32_t(playerLock.first);    // Slot
                    }

                    packet.writeByteSeq(playerGuid[1]);
                    packet.writeByteSeq(playerGuid[0]);
                    packet.writeByteSeq(playerGuid[5]);
                    packet.writeByteSeq(playerGuid[7]);
                    packet.writeByteSeq(playerGuid[3]);
                    packet.writeByteSeq(playerGuid[6]);
                    packet.writeByteSeq(playerGuid[2]);
                }

                packet << uint8_t(state);                        // ResultDetail
                packet.writeByteSeq(guid[2]);
                packet << uint32_t(time(nullptr));               // UnixTime
                packet << uint32_t(0);                           // Id (LFG queue id - not tracked)
                packet << uint32_t(3);                           // Type
                packet.writeByteSeq(guid[6]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[3]);

                return true;
            }
            else if (m_protocol.isCata())
            {
                packet << uint32_t(3);                            // Type

                packet << uint8_t(Lfg::clientJoinResult(m_protocol, result));                        // Check Result
                packet << uint32_t(0);                            // Queue Id - not tracked
                packet << uint8_t(state);                         // Check Value
                packet << uint32_t(time(nullptr));                // Join date

                packet.writeBit(guid[2]);
                packet.writeBit(guid[7]);
                packet.writeBit(guid[3]);
                packet.writeBit(guid[0]);
                packet.writeBits(lockmap.size(), 24);

                for (const auto& lock : lockmap)
                {
                    WoWGuid playerGuid = lock.first;
                    packet.writeBit(playerGuid[7]);
                    packet.writeBit(playerGuid[5]);
                    packet.writeBit(playerGuid[3]);
                    packet.writeBit(playerGuid[6]);
                    packet.writeBit(playerGuid[0]);
                    packet.writeBit(playerGuid[2]);
                    packet.writeBit(playerGuid[4]);
                    packet.writeBit(playerGuid[1]);
                    packet.writeBits(lock.second.size(), 22);
                }

                packet.writeBit(guid[4]);
                packet.writeBit(guid[5]);
                packet.writeBit(guid[1]);
                packet.writeBit(guid[6]);
                packet.flushBits();

                for (const auto& lock : lockmap)
                {
                    WoWGuid playerGuid = lock.first;

                    for (const auto& playerLock : lock.second)
                    {
                        packet << uint32_t(playerLock.second);   // Lock status
                        packet << uint32_t(0);                   // Current itemLevel - not tracked
                        packet << uint32_t(0);                   // Required itemLevel - not tracked
                        packet << uint32_t(playerLock.first);    // Dungeon entry
                    }

                    packet.writeByteSeq(playerGuid[2]);
                    packet.writeByteSeq(playerGuid[5]);
                    packet.writeByteSeq(playerGuid[1]);
                    packet.writeByteSeq(playerGuid[0]);
                    packet.writeByteSeq(playerGuid[4]);
                    packet.writeByteSeq(playerGuid[3]);
                    packet.writeByteSeq(playerGuid[6]);
                    packet.writeByteSeq(playerGuid[7]);
                }

                packet.writeByteSeq(guid[1]);
                packet.writeByteSeq(guid[4]);
                packet.writeByteSeq(guid[3]);
                packet.writeByteSeq(guid[5]);
                packet.writeByteSeq(guid[0]);
                packet.writeByteSeq(guid[7]);
                packet.writeByteSeq(guid[2]);
                packet.writeByteSeq(guid[6]);

                return true;
            }
            else if (m_protocol.expansion >= WoW::Expansion::_WotLK)
            {
                packet << uint32_t(result);      // Check Result
                packet << uint32_t(state);        // Check Value

                if (!lockmap.empty())
                {
                    // BuildPartyLockDungeonBlock
                    packet << uint8_t(lockmap.size());
                    for (const auto& lock : lockmap)
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
                }
                return true;
            }
            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
