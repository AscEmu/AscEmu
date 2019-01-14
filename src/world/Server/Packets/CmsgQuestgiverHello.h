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

        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
            questGiverGuid.Init(unpackedGuid);
            return true;
        }
    };
}}
