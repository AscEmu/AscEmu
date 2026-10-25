/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "BattlePetPacketData.hpp"

#include <cstdint>

namespace AscEmu::Packets
{
    // the name of a summoned pet another player owns
    class CmsgBattlePetQueryName : public ManagedPacket
    {
    public:
        uint32_t petId = 0;
        uint64_t unitGuid = 0;

        CmsgBattlePetQueryName() : ManagedPacket(CMSG_BATTLE_PET_QUERY_NAME, 2)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion >= WoW::Expansion::_WoD)
            {
                petId = BattlePetWire::readGuid(packet);
                unitGuid = BattlePetWire::readObjectGuid(packet);
                return true;
            }

            if (m_protocol.isMop())
            {
                WoWGuid pet;
                WoWGuid unit;
                unit[2] = packet.readBit();
                pet[6] = packet.readBit();
                pet[3] = packet.readBit();
                unit[3] = packet.readBit();
                pet[7] = packet.readBit();
                unit[4] = packet.readBit();
                unit[1] = packet.readBit();
                unit[0] = packet.readBit();
                pet[0] = packet.readBit();
                unit[7] = packet.readBit();
                unit[5] = packet.readBit();
                unit[6] = packet.readBit();
                pet[1] = packet.readBit();
                pet[2] = packet.readBit();
                pet[5] = packet.readBit();
                pet[4] = packet.readBit();

                packet.readByteSeq(unit[5]);
                packet.readByteSeq(pet[1]);
                packet.readByteSeq(unit[0]);
                packet.readByteSeq(pet[4]);
                packet.readByteSeq(unit[3]);
                packet.readByteSeq(pet[3]);
                packet.readByteSeq(unit[1]);
                packet.readByteSeq(unit[6]);
                packet.readByteSeq(pet[6]);
                packet.readByteSeq(pet[0]);
                packet.readByteSeq(pet[2]);
                packet.readByteSeq(unit[7]);
                packet.readByteSeq(unit[2]);
                packet.readByteSeq(pet[7]);
                packet.readByteSeq(unit[4]);
                packet.readByteSeq(pet[5]);

                petId = static_cast<uint32_t>(pet.getRawGuid());
                unitGuid = unit.getRawGuid();
                return true;
            }

            return false;
        }
    };
}
