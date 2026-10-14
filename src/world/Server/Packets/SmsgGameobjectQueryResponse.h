/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "Objects/GameObject.h"
#include "Objects/GameObjectProperties.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgGameobjectQueryResponse : public ManagedPacket
    {
    public:
        GameObjectProperties info{};
        uint32_t entry = 0;
        WoWGuid guid;
        const char* name;

        SmsgGameobjectQueryResponse() : SmsgGameobjectQueryResponse({}, "")
        {
        }

        SmsgGameobjectQueryResponse(GameObjectProperties info, const char* name) :
            ManagedPacket(SMSG_GAMEOBJECT_QUERY_RESPONSE, 900),
            info(info),
            entry(info.entry),
            name(name)
        {
        }

        SmsgGameobjectQueryResponse(GameObjectProperties const* info, uint32_t entry, WoWGuid const& guid, const char* name) :
            ManagedPacket(SMSG_GAMEOBJECT_QUERY_RESPONSE, 900),
            info(info != nullptr ? *info : GameObjectProperties{}),
            entry(entry),
            guid(guid),
            name(name)
        {
        }

    protected:
        size_t expectedSize() const override { return m_minimum_size; }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
            {
                packet << entry;
                const std::vector<uint8_t> packedGuid = guid.packModern();
                packet.append(packedGuid.data(), packedGuid.size());
                packet.writeBit(info.entry != 0);
                packet.flushBits();

                ByteBuffer stats;
                if (info.entry != 0)
                {
                    stats << static_cast<int32_t>(info.type) << static_cast<int32_t>(info.display_id);
                    stats << std::string(name != nullptr ? name : "") << std::string{} << std::string{} << std::string{};
                    stats << info.category_name << info.cast_bar_text << info.Unkstr;

                    const auto toWireInt32 = [](auto value) { return static_cast<int32_t>(value); };
                    const int32_t data[35] = {
                        toWireInt32(info.raw.parameter_0), toWireInt32(info.raw.parameter_1), toWireInt32(info.raw.parameter_2), toWireInt32(info.raw.parameter_3), toWireInt32(info.raw.parameter_4),
                        toWireInt32(info.raw.parameter_5), toWireInt32(info.raw.parameter_6), toWireInt32(info.raw.parameter_7), toWireInt32(info.raw.parameter_8), toWireInt32(info.raw.parameter_9),
                        toWireInt32(info.raw.parameter_10), toWireInt32(info.raw.parameter_11), toWireInt32(info.raw.parameter_12), toWireInt32(info.raw.parameter_13), toWireInt32(info.raw.parameter_14),
                        toWireInt32(info.raw.parameter_15), toWireInt32(info.raw.parameter_16), toWireInt32(info.raw.parameter_17), toWireInt32(info.raw.parameter_18), toWireInt32(info.raw.parameter_19),
                        toWireInt32(info.raw.parameter_20), toWireInt32(info.raw.parameter_21), toWireInt32(info.raw.parameter_22), toWireInt32(info.raw.parameter_23), toWireInt32(info.raw.parameter_24),
                        toWireInt32(info.raw.parameter_25), toWireInt32(info.raw.parameter_26), toWireInt32(info.raw.parameter_27), toWireInt32(info.raw.parameter_28), toWireInt32(info.raw.parameter_29),
                        toWireInt32(info.raw.parameter_30), toWireInt32(info.raw.parameter_31), toWireInt32(info.raw.parameter_32), 0, 0
                    };
                    for (int32_t value : data)
                        stats << value;

                    stats << info.size;

                    uint8_t questItemCount = 0;
                    for (uint32_t questItem : info.QuestItems)
                        if (questItem != 0)
                            ++questItemCount;

                    stats << questItemCount;
                    for (uint32_t questItem : info.QuestItems)
                        if (questItem != 0)
                            stats << static_cast<int32_t>(questItem);

                    stats << int32_t(0) << int32_t(0); // ContentTuningID, RequiredLevel
                }

                packet << static_cast<uint32_t>(stats.size());
                if (stats.size() != 0)
                    packet.append(stats.contents(), stats.size());

                return true;
            }

            if (m_protocol.expansion <= WoW::Expansion::_Cata)
            {
                packet << info.entry;
                packet << info.type << info.display_id << name << uint8_t(0) << uint8_t(0) << uint8_t(0);
                packet << info.category_name << info.cast_bar_text << info.Unkstr;
                packet << info.raw.parameter_0 << info.raw.parameter_1 << info.raw.parameter_2 << info.raw.parameter_3 << info.raw.parameter_4
                    << info.raw.parameter_5 << info.raw.parameter_6 << info.raw.parameter_7 << info.raw.parameter_8 << info.raw.parameter_9
                    << info.raw.parameter_10 << info.raw.parameter_11 << info.raw.parameter_12 << info.raw.parameter_13 << info.raw.parameter_14
                    << info.raw.parameter_15 << info.raw.parameter_16 << info.raw.parameter_17 << info.raw.parameter_18 << info.raw.parameter_19
                    << info.raw.parameter_20 << info.raw.parameter_21 << info.raw.parameter_22 << info.raw.parameter_23;
                packet << float(info.size);

                if (m_protocol.expansion >= WoW::Expansion::_WotLK)
                {
                    for (uint8_t i = 0; i < 6; ++i)
                        packet << uint32_t(info.QuestItems[i]);
                }
            }
            else
            {
                packet.writeBit(info.entry != 0);
                packet << info.entry;

                size_t pos = packet.wpos();
                packet << uint32_t(0);

                if (info.entry)
                {
                    std::string _name = name;
                    packet << info.type << info.display_id << _name << uint8_t(0) << uint8_t(0) << uint8_t(0);
                    packet << info.category_name << info.cast_bar_text << info.Unkstr;

                    packet << info.raw.parameter_0 << info.raw.parameter_1 << info.raw.parameter_2 << info.raw.parameter_3 << info.raw.parameter_4
                        << info.raw.parameter_5 << info.raw.parameter_6 << info.raw.parameter_7 << info.raw.parameter_8 << info.raw.parameter_9
                        << info.raw.parameter_10 << info.raw.parameter_11 << info.raw.parameter_12 << info.raw.parameter_13 << info.raw.parameter_14
                        << info.raw.parameter_15 << info.raw.parameter_16 << info.raw.parameter_17 << info.raw.parameter_18 << info.raw.parameter_19
                        << info.raw.parameter_20 << info.raw.parameter_21 << info.raw.parameter_22 << info.raw.parameter_23 << info.raw.parameter_24
                        << info.raw.parameter_25 << info.raw.parameter_26 << info.raw.parameter_27 << info.raw.parameter_28 << info.raw.parameter_29
                        << info.raw.parameter_30 << info.raw.parameter_31;

                    packet << float(info.size);
                    packet << uint8_t(6); // quest item count
                    for (uint8_t i = 0; i < 6; ++i)
                        packet << uint32_t(info.QuestItems[i]);

                    packet << info.raw.parameter_32;
                    packet.put(pos, uint32_t(packet.wpos() - (pos + 4)));
                }
            }
            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
