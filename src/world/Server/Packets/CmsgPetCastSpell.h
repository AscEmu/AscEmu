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
    class CmsgPetCastSpell : public ManagedPacket
    {
    public:
        uint64_t petGuid;
        uint8_t castCount;
        uint32_t spellId;
        uint8_t castFlags;

        CmsgPetCastSpell() : CmsgPetCastSpell(0, 0, 0, 0)
        {
        }

        CmsgPetCastSpell(uint64_t petGuid, uint8_t castCount, uint32_t spellId, uint8_t castFlags) :
            ManagedPacket(CMSG_PET_CAST_SPELL, 12),
            petGuid(petGuid),
            castCount(castCount),
            spellId(spellId),
            castFlags(castFlags)
        {
        }

    protected:
        bool internalSerialise(WorldPacket& /*packet*/) override
        {
            return false;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            packet >> petGuid >> castCount >> spellId >> castFlags;
            return true;
        }
    };
}}
