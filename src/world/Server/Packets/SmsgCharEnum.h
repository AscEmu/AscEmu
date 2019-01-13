/*
Copyright (c) 2014-2019 AscEmu Team <http://www.ascemu.org>
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
#if VERSION_STRING <= WotLK
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
#elif VERSION_STRING == Cata
            ByteBuffer buffer;

            packet.writeBits(0, 23);
            packet.writeBit(1);
            packet.writeBits(char_count, 17);

            if (char_count)
            {
                for (auto const& data : enum_data)
                {
                    ObjectGuid guid = MAKE_NEW_GUID(data.guid, 0, 0x000);
                    ObjectGuid guildGuid = MAKE_NEW_GUID(data.guildId, 0, HIGHGUID_TYPE_GUILD);

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
                    const uint8_t facialHair = uint8_t(data.bytes & 0xFF);

                    buffer << uint32_t(data.pet_data.family);
                    buffer.WriteByteSeq(guildGuid[2]);
                    buffer << uint8_t(0);
                    buffer << uint8_t(hairStyle);
                    buffer.WriteByteSeq(guildGuid[3]);
                    buffer << uint32_t(data.pet_data.display_id);
                    buffer << uint32_t(data.char_flags);
                    buffer << uint8_t(hairColor);
                    buffer.WriteByteSeq(guid[4]);
                    buffer << uint32_t(data.mapId);
                    buffer.WriteByteSeq(guildGuid[5]);
                    buffer << float(data.z);
                    buffer.WriteByteSeq(guildGuid[6]);
                    buffer << uint32_t(data.pet_data.level);
                    buffer.WriteByteSeq(guid[3]);
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
                    buffer.WriteByteSeq(guid[7]);
                    buffer << uint8_t(data.gender);
                    buffer.append(data.name.c_str(), data.name.length());
                    buffer << uint8_t(face);
                    buffer.WriteByteSeq(guid[0]);
                    buffer.WriteByteSeq(guid[2]);
                    buffer.WriteByteSeq(guildGuid[1]);
                    buffer.WriteByteSeq(guildGuid[7]);
                    buffer << float(data.x);
                    buffer << uint8_t(skin);
                    buffer << uint8_t(data.race);
                    buffer << uint8_t(data.level);
                    buffer.WriteByteSeq(guid[6]);
                    buffer.WriteByteSeq(guildGuid[4]);
                    buffer.WriteByteSeq(guildGuid[0]);
                    buffer.WriteByteSeq(guid[5]);
                    buffer.WriteByteSeq(guid[1]);
                    buffer << uint32_t(data.zoneId);
                }
                packet.flushBits();
                packet.append(buffer);
            }
#elif VERSION_STRING == Mop
            if (char_count)
            {
                ByteBuffer buffer;

                packet.writeBits(0, 21);
                packet.writeBits(char_count, 16);

                for (auto const& data : enum_data)
                {
                    ObjectGuid guid = MAKE_NEW_GUID(data.guid, 0, 0x000);
                    ObjectGuid guildGuid = MAKE_NEW_GUID(data.guildId, 0, HIGHGUID_TYPE_GUILD);

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

                    buffer.WriteByteSeq(guid[1]);

                    buffer << uint8_t(0);

                    const uint8_t skin = uint8_t(data.bytes & 0xFF);
                    const uint8_t face = uint8_t((data.bytes >> 8) & 0xFF);
                    const uint8_t hairStyle = uint8_t((data.bytes >> 16) & 0xFF);
                    const uint8_t hairColor = uint8_t((data.bytes >> 24) & 0xFF);
                    const uint8_t facialHair = uint8_t(data.bytes & 0xFF);
                    buffer << uint8(hairStyle);

                    buffer.WriteByteSeq(guildGuid[2]);
                    buffer.WriteByteSeq(guildGuid[0]);
                    buffer.WriteByteSeq(guildGuid[6]);

                    buffer.append(data.name.c_str(), data.name.length());

                    buffer.WriteByteSeq(guildGuid[3]);

                    buffer << float(data.x);
                    buffer << uint32_t(0);
                    buffer << uint8_t(face);
                    buffer << uint8_t(data.Class);

                    buffer.WriteByteSeq(guildGuid[5]);

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

                    buffer.WriteByteSeq(guid[3]);
                    buffer.WriteByteSeq(guid[5]);

                    buffer << uint32_t(data.pet_data.family);

                    buffer.WriteByteSeq(guildGuid[4]);

                    buffer << uint32_t(data.mapId);
                    buffer << uint8_t(data.race);
                    buffer << uint8_t(skin);

                    buffer.WriteByteSeq(guildGuid[1]);

                    buffer << uint8_t(data.level);

                    buffer.WriteByteSeq(guid[0]);
                    buffer.WriteByteSeq(guid[2]);

                    buffer << uint8_t(hairColor);
                    buffer << uint8_t(data.gender);
                    buffer << uint8_t(facialHair);

                    buffer << uint32_t(data.pet_data.level);

                    buffer.WriteByteSeq(guid[4]);
                    buffer.WriteByteSeq(guid[7]);

                    buffer << float(data.y);
                    buffer << uint32_t(data.pet_data.display_id);
                    buffer << uint32_t(0);

                    buffer.WriteByteSeq(guid[6]);

                    buffer << uint32_t(data.char_flags);
                    buffer << uint32_t(data.zoneId);

                    buffer.WriteByteSeq(guildGuid[7]);

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
#endif

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override
        {
            return true;
        }
    };
}}
