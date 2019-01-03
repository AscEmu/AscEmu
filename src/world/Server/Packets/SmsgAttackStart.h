/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgAttackStart : public ManagedPacket
    {
    public:
        uint64_t attackerGuid;
        uint64_t victimGuid;

        SmsgAttackStart() : SmsgAttackStart(0, 0)
        {
        }

        SmsgAttackStart(uint64_t attackerGuid, uint64_t victimGuid) :
            ManagedPacket(SMSG_ATTACKSTART, 0),
            attackerGuid(attackerGuid),
            victimGuid(victimGuid)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << attackerGuid << victimGuid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
