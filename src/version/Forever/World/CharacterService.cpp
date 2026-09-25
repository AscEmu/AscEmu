/*
Copyright (c) 2014-2026 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "world/Server/WorldSocket.hpp"

#include "Logging/Logger.hpp"
#include "version/Forever/BuildProfile.hpp"
#include "shared/WoWGuid.hpp"
#include "version/Forever/Opcodes.hpp"
#include "version/Forever/Packets/CharacterPackets.hpp"
#include "version/Forever/World/CharacterSelectBootstrap.hpp"
#include "version/Forever/World/ProtocolUtils.hpp"
#include "world/Server/WorldSession.h"
#include "world/Server/DatabaseDefinition.hpp"
#include "world/Objects/Units/Players/PlayerDefines.hpp"
#include "world/Objects/Units/Players/Player.hpp"
#include "world/Server/CharacterErrors.h"
#include "world/Macros/GuildMacros.hpp"
#include "world/Management/ObjectMgr.hpp"
#include "Utilities/Strings.hpp"

#include <algorithm>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace
{
    ByteBuffer buildForeverEmptyAccountItemCollectionData()
    {
        ByteBuffer packet;
        packet << uint32_t(0);
        packet << uint8_t(7);
        packet << uint32_t(0);
        packet.writeBit(0);
        packet.flushBits();
        return packet;
    }

    void ensureForeverCharacterCustomizationTable()
    {
        CharacterDatabase.waitExecute("CREATE TABLE IF NOT EXISTS `character_customizations` (" "`guid` BIGINT UNSIGNED NOT NULL, " "`chrCustomizationOptionID` INT UNSIGNED NOT NULL, " "`chrCustomizationChoiceID` INT UNSIGNED NOT NULL, " "PRIMARY KEY (`guid`, `chrCustomizationOptionID`)" ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
    }

    bool isForeverRaceClassAvailableInDatabase(uint8_t race, uint8_t classId)
    {
        auto result = WorldDatabase.query("SELECT 1 FROM playercreateinfo pi " "WHERE pi.race=%u AND pi.class=%u AND pi.build=(" "SELECT MAX(build) FROM playercreateinfo buildspecific " "WHERE buildspecific.race=pi.race " "AND buildspecific.class=pi.class " "AND buildspecific.build <= %u) LIMIT 1", static_cast<uint32_t>(race), static_cast<uint32_t>(classId), VERSION_STRING);

        return result != nullptr;
    }


}

bool WorldSocket::sendForeverEmptyCharacterList()
{
    using namespace AscEmu::Version::Forever;

    if (!sendForeverPacket(Opcode::SMSG_ENUM_CHARACTERS_RESULT, CharacterSelectBootstrap::EmptyCharacterList.data(), static_cast<uint32_t>(CharacterSelectBootstrap::EmptyCharacterList.size())))
    {
        sLogger.failure("WorldSocket::Forever: failed to send SMSG_ENUM_CHARACTERS_RESULT.");
        return false;
    }

    ByteBuffer collection = buildForeverEmptyAccountItemCollectionData();
    if (!sendForeverPacket(Opcode::SMSG_ACCOUNT_ITEM_COLLECTION_DATA, collection.contents(), static_cast<uint32_t>(collection.size())))
    {
        sLogger.failure("WorldSocket::Forever: failed to send SMSG_ACCOUNT_ITEM_COLLECTION_DATA.");
        return false;
    }

    return true;
}

bool WorldSocket::handleForeverCreateCharacter(const uint8_t* payload, uint32_t payloadSize)
{
    using namespace AscEmu::Version::Forever;

    auto sendResult = [&](CharacterErrorCodes code, uint64_t guid = 0U) -> bool
    {
        const uint32_t foreverResult = AscEmu::Version::Forever::Packets::toCharacterResult(code);
        ByteBuffer wire = AscEmu::Version::Forever::Packets::buildCreateCharacterResponse(foreverResult, m_foreverRealmId, guid);


        return sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CREATE_CHAR, wire.contents(), static_cast<uint32_t>(wire.size()));
    };

    if (m_session == nullptr)
    {
        sLogger.warning("WorldSocket::Forever: CMSG_CREATE_CHARACTER received without WorldSession.");
        return false;
    }

    AscEmu::Version::Forever::Packets::CreateCharacterRequest request;
    if (!AscEmu::Version::Forever::Packets::parseCreateCharacter(payload, payloadSize, request))
    {
        sLogger.warning("WorldSocket::Forever: malformed CMSG_CREATE_CHARACTER payload={} byte(s), hex=[{}].", payloadSize, AscEmu::Version::Forever::bytesToHex(payload, payloadSize));
        return sendResult(E_CHAR_CREATE_FAILED);
    }


    const CharacterErrorCodes nameResult = VerifyName(request.name);
    if (nameResult != E_CHAR_NAME_SUCCESS)
    {
        return sendResult(nameResult);
    }


    if (sObjectMgr.getCachedCharacterInfoByName(request.name) != nullptr)
    {
        return sendResult(E_CHAR_CREATE_NAME_IN_USE);
    }


    if (!isForeverRaceClassAvailableInDatabase(request.race, request.charClass))
    {
        return sendResult(E_CHAR_CREATE_RESTRICTED_RACECLASS);
    }


    const auto bannedNamesQuery = CharacterDatabase.query("SELECT COUNT(*) FROM banned_names WHERE name = '%s'", CharacterDatabase.escapeString(request.name).c_str());
    if (bannedNamesQuery && bannedNamesQuery->fetch()[0].asUint32() > 0U)
    {
        return sendResult(E_CHAR_NAME_PROFANE);
    }


    const auto charactersQuery = CharacterDatabase.query("SELECT COUNT(*) FROM characters WHERE acct = %u", m_session->GetAccountId());

    if (charactersQuery && charactersQuery->fetch()[0].asUint32() >= 60U)
    {
        return sendResult(E_CHAR_CREATE_SERVER_LIMIT);
    }


    CharCreate createInfo{};
    createInfo.name = request.name;
    createInfo._race = request.race;
    createInfo._class = request.charClass;
    createInfo.gender = request.sex;
    createInfo.skin = 0;
    createInfo.face = 0;
    createInfo.hairStyle = 0;
    createInfo.hairColor = 0;
    createInfo.facialHair = 0;
    createInfo.outfitId = 0;

    Player* newPlayer = sObjectMgr.createPlayer(createInfo._class);
    if (newPlayer == nullptr)
    {
        sLogger.warning("WorldSocket::Forever: ObjectMgr::createPlayer returned nullptr.");
        return sendResult(E_CHAR_CREATE_FAILED);
    }

    newPlayer->setSession(m_session);
    if (!newPlayer->create(createInfo))
    {
        sLogger.warning("WorldSocket::Forever: Player::create failed for race={} class={} name='{}'.", request.race, request.charClass, request.name);
        newPlayer->m_isReadyToBeRemoved = true;
        delete newPlayer;
        return sendResult(E_CHAR_CREATE_FAILED);
    }


    newPlayer->unsetBanned();
    newPlayer->saveToDB(true);


    const uint64_t createdGuid = newPlayer->getGuidLow();

    ensureForeverCharacterCustomizationTable();
    CharacterDatabase.waitExecute("DELETE FROM `character_customizations` WHERE `guid`=%llu", static_cast<unsigned long long>(createdGuid));

    for (const auto& customization : request.customizations)
    {
        CharacterDatabase.waitExecute("INSERT INTO `character_customizations` " "(`guid`, `chrCustomizationOptionID`, `chrCustomizationChoiceID`) " "VALUES (%llu, %u, %u)", static_cast<unsigned long long>(createdGuid), customization.optionId, customization.choiceId);
    }


    // Official Forever inserts a newly created character at the top of the
    // visible list and shifts the existing characters down by one.
    //
    // Seed any older characters that still have no order row before shifting,
    // so the new character cannot collide with fallback ordering.
    {
        auto missingExisting = CharacterDatabase.query("SELECT c.guid " "FROM characters c " "LEFT JOIN character_list_order o ON o.acct=c.acct AND o.guid=c.guid " "WHERE c.acct=%u AND c.guid<>%llu AND o.guid IS NULL " "ORDER BY c.guid", m_session->GetAccountId(), static_cast<unsigned long long>(createdGuid));

        if (missingExisting)
        {
            uint32_t nextPosition = 0;

            if (auto maxOrder = CharacterDatabase.query("SELECT MAX(listPosition) " "FROM character_list_order WHERE acct=%u", m_session->GetAccountId()))
            {
                Field* maxFields = maxOrder->fetch();
                if (maxFields[0].isSet())
                    nextPosition = static_cast<uint32_t>(maxFields[0].asUint16()) + 1U;
            }

            do
            {
                const uint64_t guid = missingExisting->fetch()[0].asUint64();

                CharacterDatabase.waitExecute("INSERT IGNORE INTO character_list_order (acct, guid, listPosition) " "VALUES (%u, %llu, %u)", m_session->GetAccountId(), static_cast<unsigned long long>(guid), nextPosition);

                ++nextPosition;
            }
            while (missingExisting->nextRow());
        }
    }

    CharacterDatabase.waitExecute("UPDATE character_list_order SET listPosition=listPosition+1 WHERE acct=%u", m_session->GetAccountId());
    CharacterDatabase.waitExecute("INSERT INTO character_list_order (acct, guid, listPosition) VALUES (%u, %llu, 0) " "ON DUPLICATE KEY UPDATE listPosition=0", m_session->GetAccountId(), static_cast<unsigned long long>(createdGuid));

    // Keep ObjectMgr's runtime character cache byte-for-byte equivalent to the
    // state produced by ObjectMgr::loadCharacters() on a world restart.
    //
    // The previous Forever create path manually constructed CachedCharacterInfo
    // and left fields such as lastLevel/lastZone at their defaults. The DB enum
    // itself is correct, but other glue/account-character paths can consult the
    // ObjectMgr cache. That explains why a full world restart (which reloads the
    // cache from DB) repairs the visible character list.
    auto cacheResult = CharacterDatabase.query("SELECT guid, name, race, class, level, gender, zoneid, timestamp, acct " "FROM characters WHERE guid=%u LIMIT 1", static_cast<uint32_t>(createdGuid));

    if (cacheResult)
    {
        auto playerInfo = std::make_unique<CachedCharacterInfo>(cacheResult->fetch());
        sObjectMgr.addCachedCharacterInfo(std::move(playerInfo));

    }
    else
    {
        sLogger.warning("WorldSocket::Forever: character '{}' was saved with guid={} but could not be re-read for ObjectMgr cache population.", request.name, createdGuid);
    }

    newPlayer->m_isReadyToBeRemoved = true;
    delete newPlayer;


    // The client asks for the character list again immediately after create.
    // The first refresh can still render the pre-create list until another
    // glue tick/refresh occurs, so arm a one-shot follow-up enum after that
    // first post-create CMSG_ENUM_CHARACTERS.
    m_foreverPostCreateEnumRefreshPending = true;
    m_foreverPostCreateEnumRefreshArmed = false;

    return sendResult(E_CHAR_CREATE_SUCCESS, createdGuid);
}

bool WorldSocket::sendForeverCharacterEnumFromDatabase(bool includeCollection)
{
    using namespace AscEmu::Version::Forever;

    if (m_session == nullptr)
    {
        sLogger.warning("WorldSocket::Forever: DB character enum requested without WorldSession.");
        return false;
    }

    const uint32_t accountId = m_session->GetAccountId();

    // Seed missing order rows for existing characters before reading the enum.
    // If character_list_order is empty after upgrading, existing characters
    // keep their current GUID order until the client sends an explicit reorder.
    auto missingOrder = CharacterDatabase.query("SELECT c.guid " "FROM characters c " "LEFT JOIN character_list_order o ON o.acct=c.acct AND o.guid=c.guid " "WHERE c.acct=%u AND o.guid IS NULL " "ORDER BY c.guid", accountId);

    if (missingOrder)
    {
        uint32_t nextPosition = 0;

        if (auto maxOrder = CharacterDatabase.query("SELECT MAX(listPosition) " "FROM character_list_order WHERE acct=%u", accountId))
        {
            Field* maxFields = maxOrder->fetch();
            if (maxFields[0].isSet())
                nextPosition = static_cast<uint32_t>(maxFields[0].asUint16()) + 1U;
        }

        do
        {
            const uint64_t guid = missingOrder->fetch()[0].asUint64();

            CharacterDatabase.waitExecute("INSERT IGNORE INTO character_list_order (acct, guid, listPosition) " "VALUES (%u, %llu, %u)", accountId, static_cast<unsigned long long>(guid), nextPosition);


            ++nextPosition;
        }
        while (missingOrder->nextRow());
    }

    auto result = CharacterDatabase.query("SELECT c.guid, c.level, c.race, c.class, c.gender, c.name, " "c.positionX, c.positionY, c.positionZ, c.mapId, c.zoneId " "FROM characters c " "LEFT JOIN character_list_order o ON o.acct=c.acct AND o.guid=c.guid " "WHERE c.acct=%u " "ORDER BY COALESCE(o.listPosition, 65535), c.guid " "LIMIT 10", accountId);

    if (result == nullptr)
    {
        return sendForeverEmptyCharacterList();
    }

    std::vector<AscEmu::Version::Forever::Packets::CharacterEnumEntry> characters;
    characters.reserve(10);

    do
    {
        Field* fields = result->fetch();

        AscEmu::Version::Forever::Packets::CharacterEnumEntry character;
        character.guid = fields[0].asUint64();
        character.level = fields[1].asUint8();
        character.race = fields[2].asUint8();
        character.charClass = fields[3].asUint8();
        character.gender = fields[4].asUint8();

        std::string dbName = fields[5].asCString();

        // The legacy characters table currently has one name column. Forever's
        // CharacterInfo carries separate 6-bit first/last-name lengths.
        // If a future migration stores "First Last" in this field, split it.
        // Otherwise the complete DB name is the first name and last name is empty.
        const size_t separator = dbName.find(' ');
        if (separator == std::string::npos)
        {
            character.firstName = dbName;
        }
        else
        {
            character.firstName = dbName.substr(0, separator);
            character.lastName = dbName.substr(separator + 1);
        }

        if (character.firstName.size() > 63U)
            character.firstName.resize(63U);
        if (character.lastName.size() > 63U)
            character.lastName.resize(63U);

        character.x = fields[6].asFloat();
        character.y = fields[7].asFloat();
        character.z = fields[8].asFloat();
        character.mapId = fields[9].asInt32();
        character.zoneId = fields[10].asInt32();

        characters.emplace_back(std::move(character));
    }
    while (characters.size() < 10U && result->nextRow());

    ensureForeverCharacterCustomizationTable();

    if (auto customizationResult = CharacterDatabase.query("SELECT cc.guid, cc.chrCustomizationOptionID, cc.chrCustomizationChoiceID " "FROM character_customizations cc " "INNER JOIN characters c ON c.guid=cc.guid " "WHERE c.acct=%u " "ORDER BY cc.guid, cc.chrCustomizationOptionID", accountId))
    {
        do
        {
            Field* customizationFields = customizationResult->fetch();
            const uint64_t guid = customizationFields[0].asUint64();

            auto characterIt = std::find_if(characters.begin(), characters.end(), [guid](const auto& character) { return character.guid == guid; });

            if (characterIt == characters.end())
                continue;

            AscEmu::Version::Forever::Packets::CharacterCustomizationChoice customization;
            customization.optionId = customizationFields[1].asUint32();
            customization.choiceId = customizationFields[2].asUint32();
            characterIt->customizations.emplace_back(customization);
        }
        while (customizationResult->nextRow());
    }

    const uint32_t virtualRealmAddress = ((m_foreverRegionId & 0xFFU) << 24U) | ((m_foreverBattlegroupId & 0xFFU) << 16U) | (m_foreverRealmId & 0xFFFFU);
    const auto& raceClassAvailability = AscEmu::Version::Forever::Packets::getRaceClassAvailability();
    ByteBuffer wire = AscEmu::Version::Forever::Packets::buildCharacterEnumResponse(virtualRealmAddress, m_foreverRealmId, characters, raceClassAvailability);

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_ENUM_CHARACTERS_RESULT, wire.contents(), static_cast<uint32_t>(wire.size())))
        return false;

    // Official Forever sends 0x00460019 immediately after a non-empty
    // character enum. It mirrors the character GUID set and is separate from
    // CharacterInfo/ListPosition.
    ByteBuffer characterListState;
    characterListState << static_cast<uint32_t>(characters.size());

    for (const AscEmu::Version::Forever::Packets::CharacterEnumEntry& character : characters)
    {
        characterListState << uint8_t(0);

        const std::vector<uint8_t> packedGuid = WoWGuid::createModernPlayer(m_foreverRealmId, character.guid).packModern();
        characterListState.append(packedGuid.data(), packedGuid.size());

        characterListState << uint32_t(0);
        characterListState << uint32_t(10);
    }

    if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_CHARACTER_LIST_STATE, characterListState.contents(), static_cast<uint32_t>(characterListState.size())))
        return false;


    if (includeCollection)
    {
        if (!sendForeverPacket(AscEmu::Version::Forever::Opcode::SMSG_ACCOUNT_ITEM_COLLECTION_DATA, CharacterSelectBootstrap::AccountItemCollection460362.data(), static_cast<uint32_t>(CharacterSelectBootstrap::AccountItemCollection460362.size())))
            return false;
    }


    return true;
}
