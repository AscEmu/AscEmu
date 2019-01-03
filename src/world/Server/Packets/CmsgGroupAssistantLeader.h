/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid >> isActivated;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
