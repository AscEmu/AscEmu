/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgSpiritHealerConfirm : public ManagedPacket
    {
    public:
        uint64_t guid;

        SmsgSpiritHealerConfirm() : SmsgSpiritHealerConfirm(0)
        {
        }

        SmsgSpiritHealerConfirm(uint64_t guid) :
            ManagedPacket(SMSG_SPIRIT_HEALER_CONFIRM, 8),
            guid(guid)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << guid;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
