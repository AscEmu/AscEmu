/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include "Server/WorldSocket.h"
#if VERSION_STRING <= TBC
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgClearExtraAuraInfo : public ManagedPacket
    {
    public:
        uint64_t guid;
        uint32_t spell_id;

        SmsgClearExtraAuraInfo() : SmsgClearExtraAuraInfo(0, 0)
        {
        }

        SmsgClearExtraAuraInfo(uint64_t guid, uint32_t spell_id) :
            ManagedPacket(SMSG_CLEAR_EXTRA_AURA_INFO, 0),
            guid(guid),
            spell_id(spell_id)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            FastGUIDPack(packet, guid);
            packet << spell_id;
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return true;
        }
    };
}}
#endif
