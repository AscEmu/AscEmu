/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgSummonResponse : public ManagedPacket
    {
    public:
        WoWGuid summonGuid;
        uint8_t isClickOn;

        CmsgSummonResponse() : CmsgSummonResponse(0, 0)
        {
        }

        CmsgSummonResponse(uint64_t summonGuid, uint8_t isClickOn) :
            ManagedPacket(CMSG_SUMMON_RESPONSE, 8 + 1),
            summonGuid(summonGuid),
            isClickOn(isClickOn)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> isClickOn;
                summonGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                summonGuid[1] = packet.readBit();
                summonGuid[3] = packet.readBit();
                summonGuid[5] = packet.readBit();
                summonGuid[2] = packet.readBit();
                isClickOn = packet.readBit();
                summonGuid[7] = packet.readBit();
                summonGuid[0] = packet.readBit();
                summonGuid[4] = packet.readBit();
                summonGuid[6] = packet.readBit();

                packet.readByteSeq(summonGuid[0]);
                packet.readByteSeq(summonGuid[1]);
                packet.readByteSeq(summonGuid[6]);
                packet.readByteSeq(summonGuid[3]);
                packet.readByteSeq(summonGuid[5]);
                packet.readByteSeq(summonGuid[4]);
                packet.readByteSeq(summonGuid[2]);
                packet.readByteSeq(summonGuid[7]);
                return true;
            }

            return false;
        }
    };
}
