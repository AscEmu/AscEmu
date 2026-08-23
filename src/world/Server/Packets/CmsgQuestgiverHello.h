/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgQuestgiverHello : public ManagedPacket
    {
    public:
        WoWGuid questGiverGuid;

        CmsgQuestgiverHello() : CmsgQuestgiverHello(0)
        {
        }

        CmsgQuestgiverHello(uint64_t questGiverGuid) :
            ManagedPacket(CMSG_QUESTGIVER_HELLO, 8),
            questGiverGuid(questGiverGuid)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                questGiverGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                questGiverGuid[5] = packet.readBit();
                questGiverGuid[6] = packet.readBit();
                questGiverGuid[7] = packet.readBit();
                questGiverGuid[3] = packet.readBit();
                questGiverGuid[4] = packet.readBit();
                questGiverGuid[2] = packet.readBit();
                questGiverGuid[1] = packet.readBit();
                questGiverGuid[0] = packet.readBit();

                packet.readByteSeq(questGiverGuid[4]);
                packet.readByteSeq(questGiverGuid[1]);
                packet.readByteSeq(questGiverGuid[7]);
                packet.readByteSeq(questGiverGuid[3]);
                packet.readByteSeq(questGiverGuid[6]);
                packet.readByteSeq(questGiverGuid[0]);
                packet.readByteSeq(questGiverGuid[5]);
                packet.readByteSeq(questGiverGuid[2]);
                return true;
            }

            return false;
        }
    };
}
