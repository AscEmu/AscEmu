/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include <cstdint>
#include <vector>

namespace AscEmu::Packets
{
    class SmsgEnumCharactersResult : public ManagedPacket
    {
    public:
        typedef std::vector<CharEnumData> _charEnumData;
        _charEnumData enum_data;

        uint8_t char_count;
        uint8_t unk1;

        SmsgEnumCharactersResult() : SmsgEnumCharactersResult(0, std::vector<CharEnumData>())
        {
        }

        SmsgEnumCharactersResult(uint8_t char_count, _charEnumData enum_data) :
            ManagedPacket(SMSG_ENUM_CHARACTERS_RESULT, 1 + char_count * 200),
            enum_data(enum_data),
            char_count(char_count),
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
            if (m_protocol.expansion <= WoW::Expansion::_WotLK)
            {
                //loop vector char enum count
                packet << char_count;

                for (auto const& data : enum_data)
                {
                    packet << data.guid << data.name << data.race << data.Class << data.gender << data.bytes
                        << uint8_t(data.bytes2 & 0xFF) << data.level << data.zoneId << data.mapId << data.x
                        << data.y << data.z << data.guildId;

                    packet << data.char_flags;

                    if (m_protocol.expansion > WoW::Expansion::_TBC)
                    {
                        packet << data.customization_flag;
                    }

                    packet << unk1;

                    packet << data.pet_data.display_id << data.pet_data.level << data.pet_data.family;

                    uint8_t counter = 20;
                    if (m_protocol.expansion > WoW::Expansion::_TBC)
                        counter = 23;

                    //\todo INVENTORY_SLOT_BAG_END but somehow we have 20 for older versions instead of 23.

                    for (uint8_t i = 0; i < counter; ++i)
                    {
                        packet << data.player_items[i].displayId << data.player_items[i].inventoryType;

                        if (m_protocol.expansion > WoW::Expansion::_Classic)
                        {
                            packet << data.player_items[i].enchantmentId;
                        }
                    }
                }
            }
            else if (m_protocol.expansion == WoW::Expansion::_Cata)
            {
                ByteBuffer buffer;

                packet.writeBits(0, 23);
                packet.writeBit(1);
                packet.writeBits(char_count, 17);

                if (char_count)
                {
                    for (auto const& data : enum_data)
                    {
                        WoWGuid guid(data.guid);
                        WoWGuid guildGuid(data.guildId, 0, HIGHGUID_TYPE_GUILD);

                        packet.writeBit(guid[3]);
                        packet.writeBit(guildGuid[1]);
                        packet.writeBit(guildGuid[7]);
                        packet.writeBit(guildGuid[2]);
                        packet.writeBits(uint32_t(data.name.length()), 7);
                        packet.writeBit(guid[4]);
                        packet.writeBit(guid[7]);
                        packet.writeBit(guildGuid[3]);
                        packet.writeBit(guid[5]);
                        packet.writeBit(guildGuid[6]);
                        packet.writeBit(guid[1]);
                        packet.writeBit(guildGuid[5]);
                        packet.writeBit(guildGuid[4]);
                        packet.writeBit(data.loginFlags & 0x20); // 0x20 = AT_LOGIN_FIRST
                        packet.writeBit(guid[0]);
                        packet.writeBit(guid[2]);
                        packet.writeBit(guid[6]);
                        packet.writeBit(guildGuid[0]);

                        buffer << uint8_t(data.Class);

                        for (uint8_t i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
                        {
                            buffer << uint8_t(data.player_items[i].inventoryType);
                            buffer << uint32_t(data.player_items[i].displayId);
                            buffer << uint32_t(data.player_items[i].enchantmentId);
                        }

                        const uint8_t skin = uint8_t(data.bytes & 0xFF);
                        const uint8_t face = uint8_t((data.bytes >> 8) & 0xFF);
                        const uint8_t hairStyle = uint8_t((data.bytes >> 16) & 0xFF);
                        const uint8_t hairColor = uint8_t((data.bytes >> 24) & 0xFF);
                        const uint8_t facialHair = uint8_t(data.bytes2 & 0xFF);

                        buffer << uint32_t(data.pet_data.family);
                        buffer.writeByteSeq(guildGuid[2]);
                        buffer << uint8_t(0);
                        buffer << uint8_t(hairStyle);
                        buffer.writeByteSeq(guildGuid[3]);
                        buffer << uint32_t(data.pet_data.display_id);
                        buffer << uint32_t(data.char_flags);
                        buffer << uint8_t(hairColor);
                        buffer.writeByteSeq(guid[4]);
                        buffer << uint32_t(data.mapId);
                        buffer.writeByteSeq(guildGuid[5]);
                        buffer << float(data.z);
                        buffer.writeByteSeq(guildGuid[6]);
                        buffer << uint32_t(data.pet_data.level);
                        buffer.writeByteSeq(guid[3]);
                        buffer << float(data.y);

                        switch (data.loginFlags)
                        {
                            case LOGIN_CUSTOMIZE_LOOKS:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_CUSTOMIZE);    //Character recustomization flag
                                break;
                            case LOGIN_CUSTOMIZE_RACE:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_RACE);         //Character recustomization + race flag
                                break;
                            case LOGIN_CUSTOMIZE_FACTION:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_FACTION);      //Character recustomization + race + faction flag
                                break;
                            default:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_NONE);         //Character recustomization no flag set
                        }

                        buffer << uint8_t(facialHair);
                        buffer.writeByteSeq(guid[7]);
                        buffer << uint8_t(data.gender);
                        buffer.append(data.name.c_str(), data.name.length());
                        buffer << uint8_t(face);
                        buffer.writeByteSeq(guid[0]);
                        buffer.writeByteSeq(guid[2]);
                        buffer.writeByteSeq(guildGuid[1]);
                        buffer.writeByteSeq(guildGuid[7]);
                        buffer << float(data.x);
                        buffer << uint8_t(skin);
                        buffer << uint8_t(data.race);
                        buffer << uint8_t(data.level);
                        buffer.writeByteSeq(guid[6]);
                        buffer.writeByteSeq(guildGuid[4]);
                        buffer.writeByteSeq(guildGuid[0]);
                        buffer.writeByteSeq(guid[5]);
                        buffer.writeByteSeq(guid[1]);
                        buffer << uint32_t(data.zoneId);
                    }
                    packet.flushBits();
                    packet.append(buffer);
                }
            }
            else if (m_protocol.expansion == WoW::Expansion::_Mop)
            {
                if (char_count)
                {
                    ByteBuffer buffer;

                    packet.writeBits(0, 21);
                    packet.writeBits(char_count, 16);

                    uint8_t listOrder = 1;
                    for (auto const& data : enum_data)
                    {
                        WoWGuid guid(static_cast<uint32_t>(data.guid), 0, HIGHGUID_TYPE_PLAYER);
                        WoWGuid guildGuid(data.guildId, 0, data.guildId ? HIGHGUID_TYPE_GUILD : 0);

                        packet.writeBit(guildGuid[4]);
                        packet.writeBit(guid[0]);
                        packet.writeBit(guildGuid[3]);
                        packet.writeBit(guid[3]);
                        packet.writeBit(guid[7]);
                        packet.writeBit(0);
                        packet.writeBit(data.loginFlags & 0x20); // 0x20 = AT_LOGIN_FIRST
                        packet.writeBit(guid[6]);
                        packet.writeBit(guildGuid[6]);
                        packet.writeBits(uint32_t(data.name.length()), 6);
                        packet.writeBit(guid[1]);
                        packet.writeBit(guildGuid[1]);
                        packet.writeBit(guildGuid[0]);
                        packet.writeBit(guid[4]);
                        packet.writeBit(guildGuid[7]);
                        packet.writeBit(guid[2]);
                        packet.writeBit(guid[5]);
                        packet.writeBit(guildGuid[2]);
                        packet.writeBit(guildGuid[5]);

                        buffer << uint32_t(0);

                        buffer.writeByteSeq(guid[1]);

                        buffer << uint8_t(listOrder++);

                        const uint8_t skin = uint8_t(data.bytes & 0xFF);
                        const uint8_t face = uint8_t((data.bytes >> 8) & 0xFF);
                        const uint8_t hairStyle = uint8_t((data.bytes >> 16) & 0xFF);
                        const uint8_t hairColor = uint8_t((data.bytes >> 24) & 0xFF);
                        const uint8_t facialHair = uint8_t(data.bytes2 & 0xFF);
                        buffer << uint8_t(hairStyle);

                        buffer.writeByteSeq(guildGuid[2]);
                        buffer.writeByteSeq(guildGuid[0]);
                        buffer.writeByteSeq(guildGuid[6]);

                        buffer.append(data.name.c_str(), data.name.length());

                        buffer.writeByteSeq(guildGuid[3]);

                        buffer << float(data.x);
                        buffer << uint32_t(0);
                        buffer << uint8_t(face);
                        buffer << uint8_t(data.Class);

                        buffer.writeByteSeq(guildGuid[5]);

                        for (uint8_t i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
                        {
                            buffer << uint32_t(data.player_items[i].enchantmentId);
                            buffer << uint8_t(data.player_items[i].inventoryType);
                            buffer << uint32_t(data.player_items[i].displayId);
                        }

                        switch (data.loginFlags)
                        {
                            case LOGIN_CUSTOMIZE_LOOKS:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_CUSTOMIZE);    //Character recustomization flag
                                break;
                            case LOGIN_CUSTOMIZE_RACE:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_RACE);         //Character recustomization + race flag
                                break;
                            case LOGIN_CUSTOMIZE_FACTION:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_FACTION);      //Character recustomization + race + faction flag
                                break;
                            default:
                                buffer << uint32_t(CHAR_CUSTOMIZE_FLAG_NONE);         //Character recustomization no flag set
                        }

                        buffer.writeByteSeq(guid[3]);
                        buffer.writeByteSeq(guid[5]);

                        buffer << uint32_t(data.pet_data.family);

                        buffer.writeByteSeq(guildGuid[4]);

                        buffer << uint32_t(data.mapId);
                        buffer << uint8_t(data.race);
                        buffer << uint8_t(skin);

                        buffer.writeByteSeq(guildGuid[1]);

                        buffer << uint8_t(data.level);

                        buffer.writeByteSeq(guid[0]);
                        buffer.writeByteSeq(guid[2]);

                        buffer << uint8_t(hairColor);
                        buffer << uint8_t(data.gender);
                        buffer << uint8_t(facialHair);

                        buffer << uint32_t(data.pet_data.level);

                        buffer.writeByteSeq(guid[4]);
                        buffer.writeByteSeq(guid[7]);

                        buffer << float(data.y);
                        buffer << uint32_t(data.pet_data.display_id);
                        buffer << uint32_t(0);

                        buffer.writeByteSeq(guid[6]);

                        buffer << uint32_t(data.char_flags);
                        buffer << uint32_t(data.zoneId);

                        buffer.writeByteSeq(guildGuid[7]);

                        buffer << float(data.z);
                    }
                    packet.writeBit(1);
                    packet.flushBits();
                    packet.append(buffer);
                }
                else
                {
                    packet.writeBits(0, 21);
                    packet.writeBits(0, 16);
                    packet.writeBit(1);
                    packet.flushBits();
                }
            }
            else if (m_protocol.isWoD())
            {
                // 6.x guids are 128 bit: type and realm in the high part, the counter in the low part
                constexpr uint64_t highTypePlayer = 2;
                constexpr uint64_t highTypeGuild = 28;
                const uint64_t realmPart = static_cast<uint64_t>(m_protocol.realmId & 0xFFFF) << 42;

                packet.writeBit(1);                             // success
                packet.writeBit(0);                             // list of deleted characters
                packet.flushBits();
                packet << uint32_t(enum_data.size());
                packet << uint32_t(0);                          // faction change restrictions

                uint8_t listPosition = 0;
                for (auto const& data : enum_data)
                {
                    writePackedGuid128(packet, (highTypePlayer << 58) | realmPart, WoWGuid::getLowGuidFromRaw(data.guid));

                    packet << uint8_t(listPosition++);
                    packet << data.race << data.Class << data.gender;
                    packet << uint8_t(data.bytes & 0xFF);               // skin
                    packet << uint8_t((data.bytes >> 8) & 0xFF);        // face
                    packet << uint8_t((data.bytes >> 16) & 0xFF);       // hair style
                    packet << uint8_t((data.bytes >> 24) & 0xFF);       // hair color
                    packet << uint8_t(data.bytes2 & 0xFF);              // facial hair
                    packet << data.level;
                    packet << int32_t(data.zoneId);
                    packet << int32_t(data.mapId);
                    packet << data.x << data.y << data.z;

                    if (data.guildId != 0)
                        writePackedGuid128(packet, (highTypeGuild << 58) | realmPart, data.guildId);
                    else
                        writePackedGuid128(packet, 0, 0);

                    packet << uint32_t(data.char_flags);
                    packet << uint32_t(data.customization_flag);
                    packet << uint32_t(0);                      // flags 3
                    packet << uint32_t(data.pet_data.display_id);
                    packet << uint32_t(data.pet_data.level);
                    packet << uint32_t(data.pet_data.family);

                    packet << uint32_t(0);                      // profession 1
                    packet << uint32_t(0);                      // profession 2

                    for (uint8_t i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
                    {
                        packet << uint32_t(data.player_items[i].displayId);
                        packet << uint32_t(data.player_items[i].enchantmentId);
                        packet << uint8_t(data.player_items[i].inventoryType);
                    }

                    packet << uint32_t(0);                      // last played time
                    packet.writeBits(static_cast<uint32_t>(data.name.length()), 6);
                    packet.writeBit(data.loginFlags & 0x20);    // first login
                    packet.writeBit(0);                         // boost in progress
                    packet.writeBits(0, 5);
                    packet.flushBits();

                    packet.append(data.name.c_str(), data.name.length());
                }
            }

            return true;
        }

        // 128 bit guid: one mask byte per half, then the non zero bytes of the low and the high part
        static void writePackedGuid128(WorldPacket& packet, uint64_t high, uint64_t low)
        {
            uint8_t masks[2] = { 0, 0 };
            uint8_t bytes[16];
            size_t count = 0;

            const uint64_t parts[2] = { low, high };
            for (size_t part = 0; part < 2; ++part)
            {
                for (uint8_t i = 0; i < 8; ++i)
                {
                    const uint8_t value = static_cast<uint8_t>((parts[part] >> (i * 8)) & 0xFF);
                    if (value != 0)
                    {
                        masks[part] |= static_cast<uint8_t>(1 << i);
                        bytes[count++] = value;
                    }
                }
            }

            packet << masks[0] << masks[1];
            packet.append(bytes, count);
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
