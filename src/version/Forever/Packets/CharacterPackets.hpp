#pragma once

#include "Network/ByteBuffer.hpp"
#include "world/Server/CharacterErrors.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace AscEmu::Version::Forever::Packets
{
    struct CharacterCustomizationChoice
    {
        uint32_t optionId{0};
        uint32_t choiceId{0};
    };


    struct CharacterEnumEntry
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
        std::vector<CharacterCustomizationChoice> customizations;
    };

    struct ClassAvailability
    {
        uint8_t classId{0};
    };

    struct RaceClassAvailability
    {
        uint8_t raceId{0};
        std::vector<ClassAvailability> classes;
    };

    struct CreateCharacterRequest
    {
        uint8_t race{0};
        uint8_t charClass{0};
        uint8_t sex{0};
        uint32_t customizationCount{0};
        uint32_t timerunningSeasonId{0};
        uint32_t templateSet{0};
        std::string name;
        std::vector<CharacterCustomizationChoice> customizations;
    };

    struct CheckCharacterNameRequest
    {
        uint32_t sequenceIndex{0};
        std::string name;
    };

    bool parseCreateCharacter(const uint8_t* payload, size_t payloadSize, CreateCharacterRequest& request);
    bool parseCheckCharacterName(const uint8_t* payload, size_t payloadSize, CheckCharacterNameRequest& request);

    uint32_t toCharacterResult(CharacterErrorCodes code);
    uint32_t toDeleteCharacterResult(CharacterErrorCodes code);
    const std::vector<RaceClassAvailability>& getRaceClassAvailability();
    ByteBuffer buildCreateCharacterResponse(uint32_t result, uint32_t realmId, uint64_t characterGuid);
    ByteBuffer buildCharacterEnumResponse(uint32_t virtualRealmAddress, uint32_t realmId, const std::vector<CharacterEnumEntry>& characters, const std::vector<RaceClassAvailability>& raceClassAvailability);
}
