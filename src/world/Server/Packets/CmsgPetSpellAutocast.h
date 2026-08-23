/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>

namespace AscEmu::Packets
{
    class CmsgPetSpellAutocast : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t spellId;
        uint8_t state;

        CmsgPetSpellAutocast() : CmsgPetSpellAutocast(0, 0, 0)
        {
        }

        CmsgPetSpellAutocast(uint64_t guid, uint32_t spellId, uint8_t state) :
            ManagedPacket(CMSG_PET_SPELL_AUTOCAST, 13),
            guid(guid),
            spellId(spellId),
            state(state)
        {
        }

    protected:
        bool internalDeserialise(WorldPacket& packet) override
        {
            if (m_protocol.expansion < WoW::Expansion::_Mop)
            {
                uint64_t unpacked_guid;
                packet >> unpacked_guid >> spellId >> state;
                guid.init(unpacked_guid);
                return true;
            }
            else if (m_protocol.isMop())
            {
                packet >> spellId;

                guid[0] = packet.readBit();
                guid[4] = packet.readBit();
                guid[2] = packet.readBit();
                guid[6] = packet.readBit();
                guid[1] = packet.readBit();
                guid[5] = packet.readBit();
                guid[3] = packet.readBit();
                guid[7] = packet.readBit();

                state = packet.readBit();

                packet.readByteSeq(guid[5]);
                packet.readByteSeq(guid[0]);
                packet.readByteSeq(guid[4]);
                packet.readByteSeq(guid[1]);
                packet.readByteSeq(guid[7]);
                packet.readByteSeq(guid[2]);
                packet.readByteSeq(guid[3]);
                packet.readByteSeq(guid[6]);
                return true;
            }

            return false;
        }
    };
}
