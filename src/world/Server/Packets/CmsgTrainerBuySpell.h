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
    class CmsgTrainerBuySpell : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t trainerId = 0;
        uint32_t spellId;

        CmsgTrainerBuySpell() : CmsgTrainerBuySpell(0, 0)
        {
        }

        CmsgTrainerBuySpell(uint64_t guid, uint32_t spellId) :
            ManagedPacket(CMSG_TRAINER_BUY_SPELL, 12),
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
            uint64_t unpackedGuid;
            packet >> unpackedGuid;
#if VERSION_STRING > WotLK
            packet >> trainerId;
#endif
            packet >> spellId;
            guid.Init(unpackedGuid);
            return true;
        }
    };
}}
