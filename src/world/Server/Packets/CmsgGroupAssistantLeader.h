/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGroupAssistantLeader : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint8_t isActivated;

        CmsgGroupAssistantLeader() : CmsgGroupAssistantLeader(0, 0)
        {
        }

        CmsgGroupAssistantLeader(uint64_t guid, uint8_t isActivated) :
            ManagedPacket(CMSG_GROUP_ASSISTANT_LEADER, 9),
            guid(guid),
            isActivated(isActivated)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                packet.readSkip<uint8_t>();

                guid[2] = packet.readBit();
                guid[0] = packet.readBit();
                guid[6] = packet.readBit();
                guid[3] = packet.readBit();
                guid[1] = packet.readBit();

                isActivated = packet.readBit() ? 1 : 0;

                guid[4] = packet.readBit();
                guid[5] = packet.readBit();
                guid[7] = packet.readBit();

                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[4]);

                return true;
            }
            else
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> isActivated;
                guid.init(unpackedGuid);

                return true;
            }
        }
    };
}
