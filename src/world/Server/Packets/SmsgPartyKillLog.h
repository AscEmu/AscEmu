/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class SmsgPartyKillLog : public ManagedPacket
    {
    public:
        uint64_t playerGuid;
        uint64_t killedGuid;

        SmsgPartyKillLog() : SmsgPartyKillLog(0, 0)
        {
        }

        SmsgPartyKillLog(uint64_t playerGuid, uint64_t killedGuid) :
            ManagedPacket(SMSG_PARTYKILLLOG, 0),
            playerGuid(playerGuid),
            killedGuid(killedGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                WoWGuid victimGuid = killedGuid;
                WoWGuid killerGuid = playerGuid;

                packet.writeBit(victimGuid[7]);
                packet.writeBit(victimGuid[2]);
                packet.writeBit(killerGuid[1]);
                packet.writeBit(victimGuid[4]);
                packet.writeBit(killerGuid[2]);
                packet.writeBit(killerGuid[5]);
                packet.writeBit(victimGuid[3]);
                packet.writeBit(victimGuid[1]);
                packet.writeBit(victimGuid[0]);
                packet.writeBit(killerGuid[3]);
                packet.writeBit(killerGuid[0]);
                packet.writeBit(killerGuid[4]);
                packet.writeBit(victimGuid[6]);
                packet.writeBit(killerGuid[7]);
                packet.writeBit(victimGuid[5]);
                packet.writeBit(killerGuid[6]);

                packet.flushBits();

                packet.writeByteSeq(victimGuid[0]);
                packet.writeByteSeq(victimGuid[5]);
                packet.writeByteSeq(killerGuid[0]);
                packet.writeByteSeq(killerGuid[2]);
                packet.writeByteSeq(victimGuid[7]);
                packet.writeByteSeq(victimGuid[6]);
                packet.writeByteSeq(victimGuid[1]);
                packet.writeByteSeq(victimGuid[4]);
                packet.writeByteSeq(killerGuid[4]);
                packet.writeByteSeq(killerGuid[1]);
                packet.writeByteSeq(victimGuid[2]);
                packet.writeByteSeq(killerGuid[6]);
                packet.writeByteSeq(killerGuid[3]);
                packet.writeByteSeq(killerGuid[5]);
                packet.writeByteSeq(killerGuid[7]);
                packet.writeByteSeq(victimGuid[3]);

                return true;
            }
            else if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << playerGuid << killedGuid;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
