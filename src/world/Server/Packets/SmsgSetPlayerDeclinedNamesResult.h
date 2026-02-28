/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"
#include "WoWGuid.hpp"

namespace AscEmu::Packets
{
    class SmsgSetPlayerDeclinedNamesResult : public ManagedPacket
    {
    public:
        uint32_t error;
        uint64_t guid;

        SmsgSetPlayerDeclinedNamesResult() : SmsgSetPlayerDeclinedNamesResult(0, 0)
        {
        }

        SmsgSetPlayerDeclinedNamesResult(uint32_t error, uint64_t guid) :
            ManagedPacket(SMSG_SET_PLAYER_DECLINED_NAMES_RESULT, 12),
            error(error),
            guid(guid)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING == Mop
            // MoP: from sgl-panda-core — 1 bit (false=OK), 8 bits GUID, flush, 8 bytes GUID, uint32(error). On error: 1 bit true, 8 bits 0, flush, uint32(1).
            const bool success = (error == 0);
            packet.writeBit(success);
            if (success)
            {
                WoWGuid g(guid);
                packet.writeBit(g[2]);
                packet.writeBit(g[0]);
                packet.writeBit(g[3]);
                packet.writeBit(g[1]);
                packet.writeBit(g[4]);
                packet.writeBit(g[6]);
                packet.writeBit(g[5]);
                packet.writeBit(g[7]);
            }
            else
            {
                packet.writeBits(0, 8);
            }
            packet.flushBits();
            if (success)
            {
                WoWGuid g(guid);
                packet.WriteByteSeq(g[2]);
                packet.WriteByteSeq(g[7]);
                packet.WriteByteSeq(g[1]);
                packet.WriteByteSeq(g[0]);
                packet.WriteByteSeq(g[4]);
                packet.WriteByteSeq(g[3]);
                packet.WriteByteSeq(g[6]);
                packet.WriteByteSeq(g[5]);
            }
            packet << error;
            return true;
#else
            packet << error << guid;
            return true;
#endif
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return false;
        }
    };
}
