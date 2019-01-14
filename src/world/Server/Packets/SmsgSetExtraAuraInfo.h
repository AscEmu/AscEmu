/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include <cstdint>

#include "ManagedPacket.h"

#ifdef AE_TBC
namespace AscEmu { namespace Packets
{
    class SmsgSetExtraAuraInfo : public ManagedPacket
    {
        bool guid_was_allocated;
    public:
        WoWGuid* guid;
        uint8_t aura_slot;
        uint32_t spell_id;
        uint32_t max_duration;
        uint32_t duration;

        SmsgSetExtraAuraInfo() : SmsgSetExtraAuraInfo(nullptr, 0, 0, 0, 0)
        {
        }

        SmsgSetExtraAuraInfo(WoWGuid* guid, uint8_t aura_slot, uint32_t spell_id, uint32_t max_duration, uint32_t duration) :
            ManagedPacket(SMSG_SET_EXTRA_AURA_INFO, 0),
            guid_was_allocated(false),
            guid(guid),
            aura_slot(aura_slot),
            spell_id(spell_id),
            max_duration(max_duration),
            duration(duration)
        {
        }

        ~SmsgSetExtraAuraInfo()
        {
            if (guid_was_allocated)
                delete guid;
        }
    protected:
        bool internalSerialise(WorldPacket& packet) override
        {
            if (!guid)
                return false;

            packet << *guid << aura_slot << spell_id << max_duration << duration;
            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            if (!guid)
            {
                guid = new WoWGuid;
                guid_was_allocated = true;
            }
            packet >> *guid >> aura_slot >> spell_id >> max_duration >> duration;
            return true;
        }
    };
}}
#endif
