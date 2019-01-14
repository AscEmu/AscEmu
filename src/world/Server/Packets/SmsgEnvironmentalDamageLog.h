/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgEnvironmentalDamageLog : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint8_t type;
        uint32_t damage;
        uint64_t unk;

        SmsgEnvironmentalDamageLog() : SmsgEnvironmentalDamageLog(0, 0, 0, 0)
        {
        }

        SmsgEnvironmentalDamageLog(uint64_t guid, uint8_t type, uint32_t damage, uint64_t unk) :
            ManagedPacket(SMSG_ENVIRONMENTALDAMAGELOG, 0),
            guid(guid),
            type(type),
            damage(damage),
            unk(unk)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return 8 + 1 + 4 + 8;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid << type << damage << unk;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
