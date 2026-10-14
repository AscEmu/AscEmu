/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#pragma once

#include "ManagedPacket.h"
#include "world/Objects/ItemDefines.hpp"
#include "world/Objects/Units/Players/PlayerDefines.hpp"
#include "shared/WoWGuid.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace AscEmu::Packets
{
    struct ForeverCharacterCustomizationChoice
    {
        uint32_t optionId{0};
        uint32_t choiceId{0};
    };

    struct ForeverCharacterVisualItem
    {
        uint32_t itemId{0};
        uint32_t transmogrifiedItemId{0};
        uint8_t subclass{0};
        uint8_t inventoryType{0};
        uint32_t displayId{0};
        uint32_t displayEnchantId{0};
        int32_t secondaryItemModifiedAppearanceId{0};
        uint8_t sheatheCategory{0};
    };

    struct ForeverCharacterEnumEntry
    {
        uint64_t guid{0};
        uint8_t level{0};
        uint8_t race{0};
        uint8_t charClass{0};
        uint8_t gender{0};
        std::string firstName;
        std::string lastName;
        float x{0.0f};
        float y{0.0f};
        float z{0.0f};
        int32_t mapId{0};
        int32_t zoneId{0};
        std::array<ForeverCharacterVisualItem, 19> visualItems{};
        std::vector<ForeverCharacterCustomizationChoice> customizations;
    };

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

        SmsgEnumCharactersResult(uint32_t virtualRealmAddress, uint32_t realmId, uint32_t superDistrictId, std::vector<ForeverCharacterEnumEntry> characters) :
            ManagedPacket(SMSG_ENUM_CHARACTERS_RESULT, 430),
            char_count(0),
            unk1(0),
            foreverVirtualRealmAddress(virtualRealmAddress),
            foreverRealmId(realmId),
            foreverSuperDistrictId(superDistrictId),
            foreverCharacters(std::move(characters))
        {
        }

    protected:
        size_t expectedSize() const override
        {
            return m_minimum_size;
        }

        bool internalSerialise(WorldPacket& packet) override
        {
            if (m_protocol.isForever())
                return serialiseForever(packet);

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

            return true;
        }

    private:
        uint32_t foreverVirtualRealmAddress{0};
        uint32_t foreverRealmId{0};
        uint32_t foreverSuperDistrictId{0};
        std::vector<ForeverCharacterEnumEntry> foreverCharacters;

        bool serialiseForever(WorldPacket& packet)
        {
            constexpr uint32_t ClassDisableMask69913 = 0x7FFFFA20U;
            constexpr uint32_t CharacterFlags2_69913 = 0x00000004U;
            constexpr uint32_t CharacterFlags4_69913 = 0x00040120U;
            constexpr int32_t CharacterSaveVersion69913 = 76;
            constexpr uint8_t CharacterNameFlags69913 = 0x0CU;

            packet.writeBit(1); // Success
            if (foreverCharacters.empty())
            {
                packet.writeBit(0); // Realmless
                packet.writeBit(0); // IsDeletedCharacters
                packet.writeBit(0); // IgnoreNewPlayerRestrictions
                packet.writeBit(1); // IsRestrictedNewPlayer
                packet.writeBit(0); // IsNewcomerChatCompleted
                packet.writeBit(0); // IsRestrictedTrial
                packet.writeBit(1); // IsAccountLapsedPlayer
                packet.writeBit(1); // ClassDisableMask present
                packet.writeBit(1); // ForceCharacterListSort
            }
            else
            {
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(1);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(0);
                packet.writeBit(1); // ClassDisableMask present
                packet.writeBit(0); // ForceCharacterListSort
            }
            packet.flushBits();

            packet << uint32_t(0); // RegionwideCharacters
            packet << uint32_t(foreverCharacters.size());

            int32_t maxCharacterLevel = foreverCharacters.empty() ? 0 : 1;
            for (const ForeverCharacterEnumEntry& character : foreverCharacters)
                maxCharacterLevel = std::max<int32_t>(maxCharacterLevel, character.level);

            packet << maxCharacterLevel;
            packet << uint32_t(ForeverRaceClassAvailability.size());
            packet << uint32_t(0); // UnlockedConditionalAppearances
            packet << uint32_t(0); // RaceLimitDisables
            packet << uint32_t(0); // WarbandGroups
            packet << ClassDisableMask69913;

            for (const ForeverCharacterEnumEntry& character : foreverCharacters)
            {
                const std::vector<uint8_t> packedGuid = WoWGuid::createModernPlayer(foreverRealmId, character.guid).packModern();
                packet.append(packedGuid.data(), packedGuid.size());
                packet << foreverVirtualRealmAddress;
                packet << uint16_t(0); // 69913 carries visible ordering in account-data type 16
                packet << character.race << character.gender << character.charClass;
                packet << int16_t(0); // SpecID
                packet << uint32_t(character.customizations.size());
                packet << character.level;
                packet << character.mapId << character.zoneId;
                packet << character.x << character.y << character.z;
                packet << (character.guid | (static_cast<uint64_t>(foreverRealmId & 0x0FFFU) << 48U));

                const std::vector<uint8_t> emptyGuildGuid = WoWGuid::createModernEmpty().packModern();
                packet.append(emptyGuildGuid.data(), emptyGuildGuid.size());

                packet << uint32_t(0);
                packet << CharacterFlags2_69913;
                packet << uint32_t(0);
                packet << CharacterFlags4_69913;
                packet << uint8_t(0); // CantLoginReason
                packet << uint32_t(0) << uint32_t(0) << uint32_t(0); // Pet

                for (const ForeverCharacterVisualItem& visualItem : character.visualItems)
                {
                    packet << visualItem.itemId << visualItem.transmogrifiedItemId;
                    packet << visualItem.subclass << visualItem.inventoryType;
                    packet << visualItem.displayId << visualItem.displayEnchantId;
                    packet << visualItem.secondaryItemModifiedAppearanceId << visualItem.sheatheCategory;
                }

                packet << CharacterSaveVersion69913;
                packet << uint64_t(0); // CreateTime
                packet << uint64_t(0); // LastActiveTime
                packet << int32_t(0); // LastLoginVersion
                for (uint8_t i = 0; i < 5U; ++i)
                    packet << int32_t(-1);
                packet << uint32_t(0) << uint32_t(0); // ProfessionIds
                packet << int32_t(0); // TimerunningSeasonID
                packet << uint32_t(0); // OverrideSelectScreenFileDataID
                packet << uint32_t(0); // RealmQueue
                packet << foreverSuperDistrictId; // FOREVER-VERIFIED: Cfg_SuperDistrictID for this realm/character.

                for (const ForeverCharacterCustomizationChoice& customization : character.customizations)
                    packet << customization.optionId << customization.choiceId;

                const uint32_t firstNameLength = static_cast<uint32_t>(std::min<size_t>(character.firstName.size(), 63U));
                const uint32_t lastNameLength = static_cast<uint32_t>(std::min<size_t>(character.lastName.size(), 63U));
                packet.writeBits(firstNameLength, 6);
                packet.writeBits(lastNameLength, 6);
                packet.writeBits(CharacterNameFlags69913, 4);
                packet.flushBits();
                if (firstNameLength != 0U)
                    packet.append(reinterpret_cast<const uint8_t*>(character.firstName.data()), firstNameLength);
                if (lastNameLength != 0U)
                    packet.append(reinterpret_cast<const uint8_t*>(character.lastName.data()), lastNameLength);

                packet.writeBit(0); // BoostInProgress
                packet.writeBit(0); // RpeAvailable
                packet.flushBits();
                packet << uint32_t(0); // RestrictionFlags
                packet << uint32_t(0); // MailSenders count
                packet << uint32_t(0); // MailSenderTypes count
                packet << uint32_t(0); // NoRpeReason
                packet << uint8_t(0) << uint8_t(0) << uint8_t(0) << uint8_t(0xFF) << uint8_t(0) << uint8_t(0); // 69913 restriction tail
            }

            for (const RaceClassAvailability& race : ForeverRaceClassAvailability)
            {
                packet << int8_t(static_cast<uint8_t>(race.race));
                packet << uint32_t(race.classCount);
                for (uint8_t i = 0; i < race.classCount; ++i)
                {
                    packet << int8_t(static_cast<uint8_t>(race.classes[i]));
                    packet << uint32_t(0); // AchievementID
                    packet.writeBit(1); // HasExpansion
                    packet.writeBit(1); // HasUnlockedAchievement
                    packet.writeBit(1); // HasEntitlement
                    packet.flushBits();
                }

                packet.writeBit(1); // HasUnlockedLicense
                packet.writeBit(0); // HasUnlockedAchievement
                packet.writeBit(0); // HasHeritageArmorUnlockAchievement
                packet.writeBit(1); // HasEntitlement
                packet.writeBit(0); // HideRaceOnClient
                packet.writeBit(0); // FactionBalanceDisabled
                packet.writeBit(0); // DoesNotHaveAvailableClasses
                packet.flushBits();
            }

            return true;
        }

        bool internalDeserialise(WorldPacket& /*packet*/) override { return false; }
    };
}
