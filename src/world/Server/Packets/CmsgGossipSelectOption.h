/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgGossipSelectOption : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t gossip_id;
        uint32_t option;
        std::string input;

        CmsgGossipSelectOption() : CmsgGossipSelectOption(0, 0, 0, "")
        {
        }

        CmsgGossipSelectOption(uint64_t guid, uint32_t gossip_id, uint32_t option, std::string input) :
            ManagedPacket(CMSG_GOSSIP_SELECT_OPTION, 0),
            guid(guid),
            gossip_id(gossip_id),
            option(option),
            input(input)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid >> gossip_id >> option;
                guid.init(unpackedGuid);

                if (packet.rpos() != packet.wpos())
                    packet >> input;
            }
            else // Mop
            {
                uint32_t inputLength = 0;
                packet >> option;
                packet >> gossip_id;

                guid[3] = packet.readBit();
                guid[0] = packet.readBit();
                guid[1] = packet.readBit();
                guid[4] = packet.readBit();
                guid[7] = packet.readBit();
                guid[5] = packet.readBit();
                guid[6] = packet.readBit();

                inputLength = packet.readBits(8);

                guid[2] = packet.readBit();

                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[6]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[5]);

                if (inputLength > 0)
                    input = packet.readString(inputLength);

                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[1]);
            }
            return true;
        }
    };
}
