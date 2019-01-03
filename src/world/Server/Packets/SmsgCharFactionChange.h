/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCharFactionChange : public ManagedPacket
    {
#if VERSION_STRING > TBC
    public:
        uint8_t result;
        uint64_t guid;
        CharCreate charCreate;

        SmsgCharFactionChange() : SmsgCharFactionChange(0, 0, CharCreate())
        {
        }

        SmsgCharFactionChange(uint8_t result) :
            ManagedPacket(SMSG_CHAR_FACTION_CHANGE, 1),
            result(result),
            guid(0),
            charCreate(CharCreate())
        {
        }

        SmsgCharFactionChange(uint8_t result, uint64_t guid, CharCreate charCreate) :
            ManagedPacket(SMSG_CHAR_FACTION_CHANGE, 9 + charCreate.name.size() + 10),
            result(result),
            guid(guid),
            charCreate(charCreate)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (result)
            {
                packet << result << guid << charCreate.name << charCreate.gender << charCreate.skin
                    << charCreate.face << charCreate.hairStyle << charCreate.hairColor << charCreate.facialHair
                    << charCreate._race;
            }
            else
            {
                packet << result;
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
#endif
    };
}}
