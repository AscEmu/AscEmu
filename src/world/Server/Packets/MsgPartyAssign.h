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
    class MsgPartyAssign : public ManagedPacket
    {
    public:
        uint8_t promoteType;
        uint8_t isActivated;
        WoWGuid guid;

        MsgPartyAssign() : MsgPartyAssign(0, 0, 0)
        {
        }

        MsgPartyAssign(uint8_t promoteType, uint8_t isActivated, uint64_t guid) :
            ManagedPacket(MSG_PARTY_ASSIGNMENT, 10),
            promoteType(promoteType),
            isActivated(isActivated),
            guid(guid)
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
            packet >> promoteType >> isActivated >> unpackedGuid;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
