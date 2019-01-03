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
    class CmsgPetCancelAura : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t spellId;

        CmsgPetCancelAura() : CmsgPetCancelAura(0, 0)
        {
        }

        CmsgPetCancelAura(uint64_t guid, uint32_t spellId) :
            ManagedPacket(CMSG_PET_CANCEL_AURA, 12),
            guid(guid),
            spellId(spellId)
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
            packet >> unpacked_guid >> spellId;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
