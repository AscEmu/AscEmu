/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"

#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgRequestCemeteryListResponse : public ManagedPacket
    {
    public:
        std::vector<uint32_t> graveyards;

        SmsgRequestCemeteryListResponse() :
            ManagedPacket(SMSG_REQUEST_CEMETERY_LIST_RESPONSE, 4)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            constexpr size_t baseReserve = 4;
            constexpr size_t entryReserve = 4;

            return baseReserve + graveyards.size() * entryReserve;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isCata())
            {
                packet.writeBit(false); //unk
                packet.flushBits();
                packet.writeBits(graveyards.size(), 24);
                packet.flushBits();

                for (const auto& id : graveyards)
                    packet << id;

                return true;
            }
            else if (m_protocol.isMop())
            {
                packet.writeBits(graveyards.size(), 22);
                packet.writeBit(false); //unk

                for (const auto& id : graveyards)
                    packet << uint32_t(id);

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override{ return false; }
    };
}
