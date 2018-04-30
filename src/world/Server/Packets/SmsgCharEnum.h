/*
Copyright (c) 2014-2018 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>

#include "ManagedPacket.h"

namespace AscEmu { namespace Packets
{
    class SmsgCharEnum : public ManagedPacket
    {
    public:
        typedef std::vector<CharEnumData> _charEnumData;
        _charEnumData enum_data;

        uint8_t char_count;
        uint8_t unk1;

        SmsgCharEnum() : SmsgCharEnum(0, std::vector<CharEnumData>())
        {
        }

        SmsgCharEnum(uint8_t char_count, _charEnumData enum_data) :
            ManagedPacket(SMSG_CHAR_ENUM, 1 + char_count * 200),
            char_count(char_count),
            enum_data(enum_data),
            unk1(0)
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            //loop vector char enum count
            packet << char_count;

            for (auto const& data : enum_data)
            {
                packet << data.guid << data.name << data.race << data.Class << data.gender << data.bytes
                    << uint8_t(data.bytes2 & 0xFF) << data.level << data.zoneId << data.mapId << data.x 
                    << data.y << data.z << data.guildId;

                packet << data.char_flags;
                
#if VERSION_STRING > TBC
                packet << data.customization_flag;
#endif

                packet << unk1;

                packet << data.pet_data.display_id << data.pet_data.level << data.pet_data.family;

                //\todo INVENTORY_SLOT_BAG_END but somehow we have 20 for older versions instead of 23.
#if VERSION_STRING > TBC
                for (uint8_t i = 0; i < 23; ++i)
#else
                for (uint8_t i = 0; i < 20; ++i)
#endif
                {
                    packet << data.player_items[i].displayId << data.player_items[i].inventoryType;

#if VERSION_STRING > Classic
                    packet << data.player_items[i].enchantmentId;
#endif
                }
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& packet) override
        {
            return true;
        }
    };
}}
