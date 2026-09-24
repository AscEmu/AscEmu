#include "version/Forever/Packets/CharacterPackets.hpp"
#include "shared/WoWGuid.hpp"

#include <algorithm>
#include <cstring>

namespace AscEmu::Version::Forever::Packets
{
    namespace
    {
        uint32_t readUInt32LE(const uint8_t* data)
        {
            uint32_t value = 0;
            std::memcpy(&value, data, sizeof(value));
            return value;
        }
    }

    bool parseCreateCharacter(const uint8_t* payload, size_t payloadSize, CreateCharacterRequest& request)
    {
        constexpr size_t FixedPrefix = 18U;
        if (payload == nullptr || payloadSize < FixedPrefix)
            return false;

        request.race = payload[3];
        request.charClass = payload[4];
        request.sex = payload[5];
        request.customizationCount = readUInt32LE(payload + 6);
        request.timerunningSeasonId = readUInt32LE(payload + 10);
        request.templateSet = readUInt32LE(payload + 14);

        if (request.customizationCount > 250U)
            return false;

        const size_t customizationBytes = static_cast<size_t>(request.customizationCount) * 8U;
        if (payloadSize < FixedPrefix + customizationBytes)
            return false;

        const size_t nameLength = payloadSize - FixedPrefix - customizationBytes;
        if (nameLength == 0U || nameLength > 63U)
            return false;

        request.name.assign(reinterpret_cast<const char*>(payload + FixedPrefix), nameLength);

        request.customizations.clear();
        request.customizations.reserve(request.customizationCount);

        const uint8_t* customizationData = payload + FixedPrefix + nameLength;
        for (uint32_t i = 0; i < request.customizationCount; ++i)
        {
            CharacterCustomizationChoice customization;
            customization.optionId = readUInt32LE(customizationData + static_cast<size_t>(i) * 8U);
            customization.choiceId = readUInt32LE(customizationData + static_cast<size_t>(i) * 8U + 4U);
            request.customizations.emplace_back(customization);
        }

        std::sort(request.customizations.begin(), request.customizations.end(), [](const CharacterCustomizationChoice& left, const CharacterCustomizationChoice& right) { return left.optionId < right.optionId; });

        return true;
    }

    bool parseCheckCharacterName(const uint8_t* payload, size_t payloadSize, CheckCharacterNameRequest& request)
    {
        if (payload == nullptr || payloadSize < 6U)
            return false;

        ByteBuffer packet(payloadSize);
        packet.append(payload, payloadSize);
        packet >> request.sequenceIndex;

        const uint32_t nameLength = packet.readBits(6);
        if (nameLength == 0U || nameLength > 63U || packet.rpos() + nameLength > packet.size())
            return false;

        request.name = packet.readString(nameLength);
        return true;
    }

    uint32_t toCharacterResult(CharacterErrorCodes code)
    {
        switch (code)
        {
            case E_CHAR_NAME_SUCCESS: return 0U;
            case E_CHAR_CREATE_SUCCESS: return 24U;
            case E_CHAR_CREATE_ERROR: return 25U;
            case E_CHAR_CREATE_FAILED: return 26U;
            case E_CHAR_CREATE_NAME_IN_USE: return 27U;
            case E_CHAR_CREATE_DISABLED: return 28U;
            case E_CHAR_CREATE_PVP_TEAMS_VIOLATION: return 29U;
            case E_CHAR_CREATE_SERVER_LIMIT: return 30U;
            case E_CHAR_CREATE_ACCOUNT_LIMIT: return 31U;
            case E_CHAR_CREATE_SERVER_QUEUE: return 32U;
            case E_CHAR_CREATE_ONLY_EXISTING: return 33U;
            case E_CHAR_CREATE_EXPANSION: return 34U;
            case E_CHAR_CREATE_EXPANSION_CLASS: return 35U;
            case E_CHAR_CREATE_CHARACTER_IN_GUILD: return 36U;
            case E_CHAR_CREATE_RESTRICTED_RACECLASS: return 37U;
            case E_CHAR_CREATE_CHARACTER_CHOOSE_RACE: return 38U;
            case E_CHAR_CREATE_CHARACTER_ARENA_LEADER: return 39U;
            case E_CHAR_CREATE_CHARACTER_DELETE_MAIL: return 41U;
            case E_CHAR_CREATE_CHARACTER_SWAP_FACTION: return 42U;
            case E_CHAR_CREATE_CHARACTER_RACE_ONLY: return 43U;
            case E_CHAR_CREATE_CHARACTER_GOLD_LIMIT: return 44U;
            case E_CHAR_CREATE_FORCE_LOGIN: return 45U;
            case E_CHAR_CREATE_TRIAL: return 46U;
            case E_CHAR_CREATE_UNIQUE_CLASS_LIMIT: return 56U;
            case E_CHAR_CREATE_LEVEL_REQUIREMENT: return 57U;
            case E_CHAR_NAME_FAILURE: return 97U;
            case E_CHAR_NAME_NO_NAME: return 98U;
            case E_CHAR_NAME_TOO_SHORT: return 99U;
            case E_CHAR_NAME_TOO_LONG: return 100U;
            case E_CHAR_NAME_INVALID_CHARACTER: return 101U;
            case E_CHAR_NAME_MIXED_LANGUAGES: return 102U;
            case E_CHAR_NAME_PROFANE: return 103U;
            case E_CHAR_NAME_RESERVED: return 104U;
            case E_CHAR_NAME_INVALID_APOSTROPHE: return 105U;
            case E_CHAR_NAME_MULTIPLE_APOSTROPHES: return 106U;
            case E_CHAR_NAME_THREE_CONSECUTIVE: return 107U;
            case E_CHAR_NAME_INVALID_SPACE: return 108U;
            case E_CHAR_NAME_CONSECUTIVE_SPACES: return 109U;
            case E_CHAR_NAME_RUSSIAN_CONSECUTIVE_SILENT_CHARACTERS: return 110U;
            case E_CHAR_NAME_RUSSIAN_SILENT_CHARACTER_AT_BEGINNING_OR_END: return 111U;
            case E_CHAR_NAME_DECLENSION_DOESNT_MATCH_BASE_NAME: return 112U;
            default: return 25U;
        }
    }

    uint32_t toDeleteCharacterResult(CharacterErrorCodes code)
    {
        switch (code)
        {
            case E_CHAR_DELETE_SUCCESS: return 67U;
            case E_CHAR_DELETE_FAILED_GUILD_LEADER: return 70U;
            case E_CHAR_DELETE_FAILED_ARENA_CAPTAIN: return 71U;
            default: return 68U;
        }
    }

    const std::vector<RaceClassAvailability>& getRaceClassAvailability()
    {
        static const std::vector<RaceClassAvailability> availability =
        {
            { 95, { { 3 }, { 4 }, { 1 }, { 8 }, { 11 } } },
            { 96, { { 3 }, { 4 }, { 7 }, { 1 }, { 11 } } },
            { 1, { { 1 }, { 2 }, { 4 }, { 5 }, { 8 }, { 9 }, { 3 } } },
            { 2, { { 1 }, { 3 }, { 4 }, { 7 }, { 9 }, { 8 } } },
            { 3, { { 1 }, { 2 }, { 3 }, { 5 }, { 4 }, { 7 } } },
            { 4, { { 1 }, { 3 }, { 4 }, { 5 }, { 11 } } },
            { 5, { { 1 }, { 4 }, { 5 }, { 8 }, { 9 }, { 2 } } },
            { 6, { { 1 }, { 3 }, { 7 }, { 11 } } },
            { 7, { { 1 }, { 4 }, { 8 }, { 9 }, { 5 } } },
            { 8, { { 1 }, { 4 }, { 3 }, { 5 }, { 7 }, { 8 }, { 9 } } }
        };
        return availability;
    }

    ByteBuffer buildCreateCharacterResponse(uint32_t result, uint32_t realmId, uint64_t characterGuid)
    {
        ByteBuffer packet;
        packet << result;

        const auto packedGuid = WoWGuid::createModernPlayer(realmId, characterGuid).packModern();
        packet.append(packedGuid.data(), packedGuid.size());
        return packet;
    }

    ByteBuffer buildCharacterEnumResponse(uint32_t virtualRealmAddress, uint32_t realmId, const std::vector<CharacterEnumEntry>& characters, const std::vector<RaceClassAvailability>& raceClassAvailability)
    {
        constexpr uint32_t ClassDisableMask69913 = 0x7FFFFA20U;
        constexpr uint32_t CharacterFlags2_69913 = 0x00000004U;
        constexpr uint32_t CharacterFlags4_69913 = 0x00040120U;
        constexpr int32_t CharacterSaveVersion69913 = 76;
        constexpr uint32_t CharacterInfoUnknown69913 = 2U;
        constexpr uint8_t CharacterNameFlags69913 = 0x0CU;

        ByteBuffer packet;

        packet.writeBit(1); // Success
        packet.writeBit(1); // 69913 glue flag
        packet.writeBit(0);
        packet.writeBit(0);
        packet.writeBit(1); // 69913 glue flag
        packet.writeBit(0);
        packet.writeBit(0);
        packet.writeBit(0);
        packet.writeBit(1); // ClassDisableMask present
        packet.writeBit(0); // ForceCharacterListSort
        packet.flushBits();

        packet << uint32_t(0); // RegionwideCharacters
        packet << uint32_t(characters.size());

        int32_t maxCharacterLevel = 1;
        for (const CharacterEnumEntry& character : characters)
            maxCharacterLevel = std::max<int32_t>(maxCharacterLevel, character.level);

        packet << maxCharacterLevel;
        packet << uint32_t(raceClassAvailability.size());
        packet << uint32_t(0); // UnlockedConditionalAppearances
        packet << uint32_t(0); // RaceLimitDisables
        packet << uint32_t(0); // WarbandGroups
        packet << ClassDisableMask69913;

        for (const CharacterEnumEntry& character : characters)
        {
            const std::vector<uint8_t> packedGuid = WoWGuid::createModernPlayer(realmId, character.guid).packModern();
            packet.append(packedGuid.data(), packedGuid.size());
            packet << virtualRealmAddress;
            packet << uint16_t(0); // 69913 carries visible ordering in account-data type 16
            packet << character.race << character.gender << character.charClass;
            packet << int16_t(0); // SpecID
            packet << uint32_t(character.customizations.size());
            packet << character.level;
            packet << character.mapId << character.zoneId;
            packet << character.x << character.y << character.z;
            packet << (character.guid | (static_cast<uint64_t>(realmId & 0x0FFFU) << 48U));

            const std::vector<uint8_t> emptyGuildGuid = WoWGuid::createModernEmpty().packModern();
            packet.append(emptyGuildGuid.data(), emptyGuildGuid.size());

            packet << uint32_t(0);
            packet << CharacterFlags2_69913;
            packet << uint32_t(0);
            packet << CharacterFlags4_69913;
            packet << uint8_t(0); // CantLoginReason
            packet << uint32_t(0) << uint32_t(0) << uint32_t(0); // Pet

            for (uint8_t slot = 0; slot < 19U; ++slot)
            {
                packet << uint32_t(0) << uint32_t(0);
                packet << uint8_t(0) << uint8_t(0);
                packet << uint32_t(0) << uint32_t(0);
                packet << int32_t(0) << uint8_t(0);
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
            packet << CharacterInfoUnknown69913;

            for (const CharacterCustomizationChoice& customization : character.customizations)
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

        for (const RaceClassAvailability& race : raceClassAvailability)
        {
            packet << int8_t(race.raceId);
            packet << uint32_t(race.classes.size());
            for (const ClassAvailability& charClass : race.classes)
            {
                packet << int8_t(charClass.classId);
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

        return packet;
    }
}
