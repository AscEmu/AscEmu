/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "WoWGuid.hpp"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgLfgQueueStatus : public ManagedPacket
    {
    public:
        uint32_t dungeon;
        int32_t avgWaitTime;
        int32_t waitTime;
        int32_t waitTimeTanks;
        int32_t waitTimeHealer;
        int32_t waitTimeDps;
        uint8_t tanks;
        uint8_t healers;
        uint8_t dps;
        uint32_t queuedTime;
        uint64_t guid;

        SmsgLfgQueueStatus() : SmsgLfgQueueStatus(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
        {
        }

        SmsgLfgQueueStatus(uint32_t dungeon, int32_t avgWaitTime, int32_t waitTime, int32_t waitTimeTanks, int32_t waitTimeHealer,
            int32_t waitTimeDps, uint8_t tanks, uint8_t healers, uint8_t dps, uint32_t queuedTime, uint64_t guid = 0) :
            ManagedPacket(SMSG_LFG_QUEUE_STATUS, 0),
            dungeon(dungeon),
            avgWaitTime(avgWaitTime),
            waitTime(waitTime),
            waitTimeTanks(waitTimeTanks),
            waitTimeHealer(waitTimeHealer),
            waitTimeDps(waitTimeDps),
            tanks(tanks),
            healers(healers),
            dps(dps),
            queuedTime(queuedTime),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return 8 + 4 + 4 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 4; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid playerGuid = guid;

                packet.writeBit(playerGuid[4]);
                packet.writeBit(playerGuid[3]);
                packet.writeBit(playerGuid[5]);
                packet.writeBit(playerGuid[1]);
                packet.writeBit(playerGuid[2]);
                packet.writeBit(playerGuid[0]);
                packet.writeBit(playerGuid[6]);
                packet.writeBit(playerGuid[7]);

                packet.flushBits();

                packet << uint32_t(dungeon);                                      // Dungeon
                packet.writeByteSeq(playerGuid[0]);
                packet << uint32_t(queuedTime);                                   // Player wait time in queue
                packet.writeByteSeq(playerGuid[4]);
                packet << uint32_t(0);                                            // Join time (not tracked)
                packet << int32_t(waitTimeTanks);                                 // Wait Tanks
                packet << uint8_t(tanks);                                         // Tanks needed
                packet << int32_t(waitTimeHealer);                                // Wait Healers
                packet << uint8_t(healers);                                       // Healers needed
                packet << int32_t(waitTimeDps);                                   // Wait Dps
                packet << uint8_t(dps);                                           // Dps needed
                packet << int32_t(waitTime);                                      // Wait Time
                packet << int32_t(avgWaitTime);                                   // Average Wait time
                packet.writeByteSeq(playerGuid[1]);
                packet << uint32_t(0);                                            // Queue Id (not tracked)
                packet.writeByteSeq(playerGuid[7]);
                packet.writeByteSeq(playerGuid[2]);
                packet << uint32_t(3);                                            // Type
                packet.writeByteSeq(playerGuid[5]);
                packet.writeByteSeq(playerGuid[3]);
                packet.writeByteSeq(playerGuid[6]);

                return true;
            }
            else if (m_protocol.expansion > WoW::Expansion::_TBC)
            {
                packet << uint32_t(dungeon);                                          // Dungeon
                packet << int32_t(avgWaitTime);                                       // Average Wait time
                packet << int32_t(waitTime);                                          // Wait Time
                packet << int32_t(waitTimeTanks);                                     // Wait Tanks
                packet << int32_t(waitTimeHealer);                                    // Wait Healers
                packet << int32_t(waitTimeDps);                                       // Wait Dps
                packet << uint8_t(tanks);                                             // Tanks needed
                packet << uint8_t(healers);                                           // Healers needed
                packet << uint8_t(dps);                                               // Dps needed
                packet << uint32_t(queuedTime);                                       // Player wait time in queue

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
