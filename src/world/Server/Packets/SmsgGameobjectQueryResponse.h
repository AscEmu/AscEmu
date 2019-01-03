/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once
#include <cstdint>
#include <utility>

#include "ManagedPacket.h"
#include "Objects/GameObject.h"

namespace AscEmu { namespace Packets
{
    class SmsgGameobjectQueryResponse : public ManagedPacket
    {
    public:
        GameObjectProperties info;
        const char* name;

        SmsgGameobjectQueryResponse() : SmsgGameobjectQueryResponse(info, "")
        {
        }

        SmsgGameobjectQueryResponse(GameObjectProperties info, const char* name) :
            ManagedPacket(SMSG_GAMEOBJECT_QUERY_RESPONSE, 900),
            info(info),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            packet << info.entry << info.type << info.display_id << name << uint8_t(0) << uint8_t(0) << uint8_t(0);
            packet << info.category_name << info.cast_bar_text << info.Unkstr;
            packet << info.raw.parameter_0 << info.raw.parameter_1 << info.raw.parameter_2 << info.raw.parameter_3 << info.raw.parameter_4
                   << info.raw.parameter_5 << info.raw.parameter_6 << info.raw.parameter_7 << info.raw.parameter_8 << info.raw.parameter_9 
                   << info.raw.parameter_10 << info.raw.parameter_11 << info.raw.parameter_12 << info.raw.parameter_13 << info.raw.parameter_14
                   << info.raw.parameter_15 << info.raw.parameter_16 << info.raw.parameter_17 << info.raw.parameter_18 << info.raw.parameter_19
                   << info.raw.parameter_20 << info.raw.parameter_21 << info.raw.parameter_22 << info.raw.parameter_23;
            packet << float(info.size);

#if VERSION_STRING >= WotLK
            for (uint8_t i = 0; i < 6; ++i)
                packet << uint32_t(info.QuestItems[i]);
#endif
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}}
