/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

namespace AscEmu { namespace Packets
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
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            uint64_t unpacked_guid;
            packet >> unpacked_guid >> spellId >> state;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
