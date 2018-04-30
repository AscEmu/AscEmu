/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class CmsgCastSpell : public ManagedPacket
    {
    public:
        uint32_t spell_id;
        uint8_t cast_count;
        uint8_t flags;

        CmsgCastSpell() : CmsgCastSpell(0, 0, 0)
        {
        }

        CmsgCastSpell(uint32_t spell_id, uint8_t cast_count, uint8_t flags) :
            ManagedPacket(CMSG_CAST_SPELL, 0),
            spell_id(spell_id),
            cast_count(cast_count),
            flags(flags)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= TBC
            packet << spell_id << cast_count;
#elif VERSION_STRING == WotLK
            packet << cast_count << spell_id << flags;
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
#if VERSION_STRING <= TBC
            packet >> spell_id >> cast_count;
#elif VERSION_STRING == WotLK
            packet >> cast_count >> spell_id >> flags;
#endif
            return true;
        }
    };
}}
