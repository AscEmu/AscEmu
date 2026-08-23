/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetitionBuy : public ManagedPacket
    {
    public:
        WoWGuid creatureGuid;
        std::string name;
        uint32_t signerCount = 0;
        uint32_t arenaIndex = 0;

        CmsgPetitionBuy() : CmsgPetitionBuy("")
        {
        }

        CmsgPetitionBuy(std::string name) :
            ManagedPacket(CMSG_PETITION_BUY, 0),
            name(name)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpackedGuid;
                packet >> unpackedGuid;
                packet.readSkip<uint32_t>();
                packet.readSkip<uint64_t>();
                packet >> name;
                packet.readSkip<std::string>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint16_t>();
                packet.readSkip<uint32_t>();
                packet.readSkip<uint32_t>();
                packet >> signerCount;
                for (uint8_t s = 0; s < 10; ++s)
                    packet.readSkip<std::string>();

                packet >> arenaIndex;
                packet.readSkip<uint32_t>();

                creatureGuid.init(unpackedGuid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                // Mop's client no longer offers arena charters through this opcode -
                // signerCount/arenaIndex are not sent and stay at their defaults
                creatureGuid[5] = packet.readBit();
                creatureGuid[2] = packet.readBit();
                creatureGuid[3] = packet.readBit();

                const uint32_t nameLength = packet.readBits(7);

                creatureGuid[4] = packet.readBit();
                creatureGuid[1] = packet.readBit();
                creatureGuid[7] = packet.readBit();
                creatureGuid[0] = packet.readBit();
                creatureGuid[6] = packet.readBit();

                name = packet.readString(nameLength);

                packet.readByteSeq(creatureGuid[1]);
                packet.readByteSeq(creatureGuid[7]);
                packet.readByteSeq(creatureGuid[4]);
                packet.readByteSeq(creatureGuid[6]);
                packet.readByteSeq(creatureGuid[0]);
                packet.readByteSeq(creatureGuid[5]);
                packet.readByteSeq(creatureGuid[2]);
                packet.readByteSeq(creatureGuid[3]);
                return true;
            }

            return false;
        }
    };
}
