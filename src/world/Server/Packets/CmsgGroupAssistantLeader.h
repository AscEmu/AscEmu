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
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> isActivated;
            guid.init(unpackedGuid);
            return true;
        }
    };
}
