/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"
#include "WorldPacket.h"

//\brief: this packet is not correct!
namespace AscEmu { namespace Packets
{
    class CmsgPetSetAction : public ManagedPacket
    {
    public:
        WoWGuid guid;
        uint32_t slot;
        uint16_t spell;
        uint16_t state;

        CmsgPetSetAction() : CmsgPetSetAction(0, 0, 0, 0)
        {
        }

        CmsgPetSetAction(uint64_t guid, uint32_t slot, uint16_t spell, uint16_t state) :
            ManagedPacket(CMSG_PET_SET_ACTION, 8),
            guid(guid),
            slot(slot),
            spell(spell),
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
            packet >> unpacked_guid >> slot >> spell >> state;
            guid.Init(unpacked_guid);
            return true;
        }
    };
}}
