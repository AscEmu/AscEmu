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
            ManagedPacket(SMSG_REQUEST_CEMETERY_LIST_RESPONSE, 4 + graveyards.size() * 2 * 4)
        {
        }

    protected:
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
                    packet << id;

                return true;
            }

            return false;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override{ return false; }
    };
}
