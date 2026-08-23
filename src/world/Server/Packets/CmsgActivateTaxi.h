/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgActivateTaxi : public ManagedPacket
    {
    public:
        WoWGuid guid;
        std::vector<uint32_t> nodes;

        CmsgActivateTaxi() : CmsgActivateTaxi(0, {0, 0})
        {
            nodes.resize(2);
        }

        CmsgActivateTaxi(uint64_t guid, std::vector<uint32_t> nodes) :
            ManagedPacket(CMSG_ACTIVATE_TAXI, 8 + 4 + 4),
            guid(guid),
            nodes(nodes)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.isMop())
            {
                nodes.resize(2);

                packet >> nodes[1] >> nodes[0];

                guid[4] = packet.readBit();
                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[2] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();
                guid[7] = packet.readBit();
                guid[3] = packet.readBit();

                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[7]);

                return true;
            }
            else if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                nodes.resize(2);

                uint64_t unpackedGuid;
                packet >> unpackedGuid >> nodes[0] >> nodes[1];
                guid = WoWGuid(unpackedGuid);

                return true;
            }

            return false;
        }
    };
}
