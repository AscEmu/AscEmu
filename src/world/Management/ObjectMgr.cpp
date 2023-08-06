/*
Copyright (c) 2014-2023 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include <utility>
#include "Storage/DBC/DBCStores.hpp"
#include "Management/QuestLogEntry.hpp"
#include "Objects/Container.hpp"
#include "Objects/Units/Stats.h"
#include "Management/ArenaTeam.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Objects/Units/Players/PlayerClasses.hpp"
#include "Map/Maps/InstanceDefines.hpp"
#include "Map/Management/MapMgr.hpp"
#include "Map/Maps/MapScriptInterface.h"
#include "Spell/SpellMgr.hpp"
#include "Objects/Units/Creatures/Pet.h"
#include "Spell/Definitions/SpellEffects.hpp"
#include "Management/TaxiMgr.h"
#include "Management/LFG/LFGMgr.hpp"
#include "Movement/MovementManager.h"
#include "Objects/Units/Creatures/Summons/Summon.hpp"
#include "Server/DatabaseDefinition.hpp"
#include "Utilities/Strings.hpp"

#if VERSION_STRING < Cata
#include "Management/Guild/Guild.hpp"
#include "Server/World.h"
#endif

ObjectMgr& ObjectMgr::getInstance()
{
    static ObjectMgr mInstance;
    return mInstance;
}

void ObjectMgr::initialize()
{
    loadCreatureDisplayInfo();
}

void ObjectMgr::finalize()
{
    sLogger.info("ObjectMgr : Deleting Corpses...");
    unloadCorpseCollector();

    sLogger.info("ObjectMgr : Clearing Vendors...");
    m_vendors.clear();

    sLogger.info("ObjectMgr : Clearing TrainerSpellSets...");
    m_trainerSpellSet.clear();

    sLogger.info("ObjectMgr : Clearing Trainers...");
    m_trainers.clear();

    sLogger.info("ObjectMgr : Clearing Level Information...");
    for (const auto levelInfoPair : m_levelInfo)
        levelInfoPair.second->clear();

    m_levelInfo.clear();

    sLogger.info("ObjectMgr : Clearing timed emote Cache...");
    m_timedEmotes.clear();

    sLogger.info("ObjectMgr : Clearing Charters...");
    for (auto& charter : m_charters)
        charter.clear();

    sLogger.info("ObjectMgr : Clearing Reputation Tables...");
    m_reputationFaction.clear();
    m_reputationCreature.clear();
    m_reputationInstance.clear();

    sLogger.info("ObjectMgr : Deleting Groups...");
    for (const auto groupPair : m_groups)
    {
        auto group = groupPair.second;
        if (group != nullptr)
        {
            for (uint32_t i = 0; i < group->GetSubGroupCount(); ++i)
            {
                SubGroup* pSubGroup = group->GetSubGroup(i);
                if (pSubGroup != nullptr)
                    pSubGroup->Disband();
            }
        }
    }

    sLogger.info("ObjectMgr : Clearing Player Information...");
    m_cachedCharacterInfo.clear();

    sLogger.info("ObjectMgr : Clearing Boss Information...");
    m_dungeonEncounterStore.clear();

    sLogger.info("ObjectMgr : Clearing Arena Teams...");
    m_arenaTeams.clear();

#ifdef FT_VEHICLES
    sLogger.info("ObjectMgr : Cleaning up vehicle accessories...");
    m_vehicleAccessoryStore.clear();
    m_vehicleSeatAddonStore.clear();
#endif

    sLogger.info("ObjectMgr : Cleaning up worldstate templates...");
    m_worldstateTemplates.clear();

    m_creatureDisplayInfoData.clear();

    sLogger.info("ObjectMgr : Clearing up event scripts...");
    m_eventScriptMaps.clear();
    m_spellEffectMaps.clear();
}
//////////////////////////////////////////////////////////////////////////////////////////
// Arena Team
void ObjectMgr::loadArenaTeams()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM arenateams");
    if (result != nullptr)
    {
        if (result->GetFieldCount() != 22)
        {
            sLogger.failure("arenateams table format is invalid. Please update your database.");
            return;
        }
        do
        {
            const std::shared_ptr<ArenaTeam> team = std::make_shared<ArenaTeam>(result->Fetch());
            addArenaTeam(team);
            if (team->m_id > static_cast<uint32_t>(m_hiArenaTeamId.load()))
                m_hiArenaTeamId = static_cast<uint32_t>(team->m_id);

        } while (result->NextRow());
        delete result;
    }

    updateArenaTeamRankings();
}

void ObjectMgr::addArenaTeam(std::shared_ptr<ArenaTeam> _arenaTeam)
{
    std::lock_guard guard(m_arenaTeamLock);
    m_arenaTeams[_arenaTeam->m_id] = _arenaTeam;
    m_arenaTeamMap[_arenaTeam->m_type].insert(std::make_pair(_arenaTeam->m_id, _arenaTeam));
}

void ObjectMgr::removeArenaTeam(std::shared_ptr<ArenaTeam> _arenaTeam)
{
    std::lock_guard guard(m_arenaTeamLock);
    m_arenaTeams.erase(_arenaTeam->m_id);
    m_arenaTeamMap[_arenaTeam->m_type].erase(_arenaTeam->m_id);
}

std::shared_ptr<ArenaTeam> ObjectMgr::getArenaTeamByName(std::string& _name, uint32_t /*type*/)
{
    std::lock_guard guard(m_arenaTeamLock);
    for (auto& arenaTeam : m_arenaTeams)
        if (arenaTeam.second->m_name == _name)
            return arenaTeam.second;

    return nullptr;
}

std::shared_ptr<ArenaTeam> ObjectMgr::getArenaTeamById(uint32_t _id)
{
    std::lock_guard guard(m_arenaTeamLock);
    const auto arenaTeam = m_arenaTeams.find(_id);
    return arenaTeam == m_arenaTeams.end() ? nullptr : arenaTeam->second;
}

std::shared_ptr<ArenaTeam> ObjectMgr::getArenaTeamByGuid(uint32_t _guid, uint32_t _type)
{
    std::lock_guard guard(m_arenaTeamLock);
    for (auto& arenaTeam : m_arenaTeamMap[_type])
    {
        if (arenaTeam.second->isMember(_guid))
            return arenaTeam.second;
    }
    return nullptr;
}

class ArenaSorter
{
public:

    bool operator()(std::shared_ptr<ArenaTeam> const& _arenaTeamA, std::shared_ptr<ArenaTeam> const& _arenaTeamB) const
    {
        return (_arenaTeamA->m_stats.rating > _arenaTeamB->m_stats.rating);
    }

    bool operator()(std::shared_ptr<ArenaTeam>& _arenaTeamA, std::shared_ptr<ArenaTeam>& _arenaTeamB) const
    {
        return (_arenaTeamA->m_stats.rating > _arenaTeamB->m_stats.rating);
    }
};

void ObjectMgr::updateArenaTeamRankings()
{
    std::lock_guard guard(m_arenaTeamLock);
    for (auto& arenaTeams : m_arenaTeamMap)
    {
        std::vector<std::shared_ptr<ArenaTeam>> ranking;
        ranking.reserve(arenaTeams.size());

        for (auto& arenaTeamPair : arenaTeams)
            ranking.push_back(arenaTeamPair.second);

        std::ranges::sort(ranking, ArenaSorter());
        uint32_t rank = 1;

        for (const auto& arenaTeam : ranking)
        {
            if (arenaTeam->m_stats.ranking != rank)
            {
                arenaTeam->m_stats.ranking = rank;
                arenaTeam->saveToDB();
            }

            ++rank;
        }
    }
}

void ObjectMgr::updateArenaTeamWeekly()
{
    std::lock_guard guard(m_arenaTeamLock);
    for (auto& arenaTeams : m_arenaTeamMap)
    {
        for (const auto& arenaTeamPair : arenaTeams)
        {
            if (const std::shared_ptr<ArenaTeam> arenaTeam = arenaTeamPair.second)
            {
                arenaTeam->m_stats.played_week = 0;
                arenaTeam->m_stats.won_week = 0;

                for (uint32_t j = 0; j < arenaTeam->m_memberCount; ++j)
                {
                    arenaTeam->m_members[j].Played_ThisWeek = 0;
                    arenaTeam->m_members[j].Won_ThisWeek = 0;
                }

                arenaTeam->saveToDB();
            }
        }
    }
}

void ObjectMgr::resetArenaTeamRatings()
{
    std::lock_guard guard(m_arenaTeamLock);
    for (auto& arenaTeams : m_arenaTeamMap)
    {
        for (auto& arenaTeamPair : arenaTeams)
        {
            if (const std::shared_ptr<ArenaTeam> arenaTeam = arenaTeamPair.second)
            {
                arenaTeam->m_stats.played_season = 0;
                arenaTeam->m_stats.played_week = 0;
                arenaTeam->m_stats.won_season = 0;
                arenaTeam->m_stats.won_week = 0;
                arenaTeam->m_stats.rating = 1500;

                for (uint32_t j = 0; j < arenaTeam->m_memberCount; ++j)
                {
                    arenaTeam->m_members[j].Played_ThisSeason = 0;
                    arenaTeam->m_members[j].Played_ThisWeek = 0;
                    arenaTeam->m_members[j].Won_ThisSeason = 0;
                    arenaTeam->m_members[j].Won_ThisWeek = 0;
                    arenaTeam->m_members[j].PersonalRating = 1500;
                }
                arenaTeam->saveToDB();
            }
        }
    }

    updateArenaTeamRankings();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Charter
void ObjectMgr::loadCharters()
{
    m_hiCharterId = 0;

    if (QueryResult* result = CharacterDatabase.Query("SELECT * FROM charters"))
    {
        do
        {
            auto charter = std::make_shared<Charter>(result->Fetch());
            m_charters[charter->getCharterType()].insert(std::make_pair(charter->getId(), charter));
            if (charter->getId() > static_cast<int64_t>(m_hiCharterId.load()))
                m_hiCharterId = charter->getId();

        } while (result->NextRow());

        delete result;
    }
    sLogger.info("ObjectMgr : %u charters loaded.", static_cast<uint32_t>(m_charters[0].size()));
}

void ObjectMgr::removeCharter(const std::shared_ptr<Charter>& _charter)
{
    if (_charter)
    {
        if (_charter->getCharterType() >= NUM_CHARTER_TYPES)
        {
            sLogger.debug("ObjectMgr : Charter %u cannot be destroyed as type %u is not a valid type.", _charter->getId(), static_cast<uint32_t>(_charter->getCharterType()));
            return;
        }

        std::lock_guard guard(m_charterLock);
        m_charters[_charter->getCharterType()].erase(_charter->getId());
    }
}

std::shared_ptr<Charter> ObjectMgr::createCharter(uint32_t _leaderGuid, CharterTypes _type)
{
    uint32_t charterId = ++m_hiCharterId;
    auto charter = std::make_shared<Charter>(charterId, _leaderGuid, _type);

    std::lock_guard guard(m_charterLock);
    m_charters[charter->getCharterType()].insert(std::make_pair(charter->getId(), charter));

    return charter;
}

std::shared_ptr<Charter> ObjectMgr::getCharterByName(const std::string& _charterName, const CharterTypes _type)
{
    std::lock_guard guard(m_charterLock);

    for (auto& charterPair : m_charters[_type])
        if (charterPair.second->getGuildName() == _charterName)
            return charterPair.second;

    return nullptr;
}

std::shared_ptr<Charter> ObjectMgr::getCharter(const uint32_t _charterId, const CharterTypes _type)
{
    std::lock_guard guard(m_charterLock);
    const auto charterPair = m_charters[_type].find(_charterId);
    return charterPair == m_charters[_type].end() ? nullptr : charterPair->second;
}

std::shared_ptr<Charter> ObjectMgr::getCharterByGuid(const uint64_t _playerGuid, const CharterTypes _type)
{
    std::lock_guard guard(m_charterLock);
    for (auto& charterPair : m_charters[_type])
    {
        if (_playerGuid == charterPair.second->getLeaderGuid())
            return charterPair.second;

        for (const uint32_t playerGuid : charterPair.second->getSignatures())
            if (playerGuid == _playerGuid)
                return charterPair.second;
    }

    return nullptr;
}

std::shared_ptr<Charter> ObjectMgr::getCharterByItemGuid(const uint64_t _itemGuid)
{
    std::lock_guard guard(m_charterLock);
    for (auto& charterType : m_charters)
    {
        for (auto& charterPair : charterType)
            if (charterPair.second->getItemGuid() == _itemGuid)
                return charterPair.second;
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// CachedCharacterInfo
void ObjectMgr::loadCharacters()
{
    QueryResult* result = CharacterDatabase.Query("SELECT guid, name, race, class, level, gender, zoneid, timestamp, acct FROM characters");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            const auto cachedCharacterInfo = std::make_shared<CachedCharacterInfo>();
            cachedCharacterInfo->guid = fields[0].GetUInt32();

            std::string characterNameDB = fields[1].GetString();
            AscEmu::Util::Strings::capitalize(characterNameDB);

            cachedCharacterInfo->name = characterNameDB;
            cachedCharacterInfo->race = fields[2].GetUInt8();
            cachedCharacterInfo->cl = fields[3].GetUInt8();
            cachedCharacterInfo->lastLevel = fields[4].GetUInt32();
            cachedCharacterInfo->gender = fields[5].GetUInt8();
            cachedCharacterInfo->lastZone = fields[6].GetUInt32();
            cachedCharacterInfo->lastOnline = fields[7].GetUInt32();
            cachedCharacterInfo->acct = fields[8].GetUInt32();
            cachedCharacterInfo->m_Group = nullptr;
            cachedCharacterInfo->subGroup = 0;
            cachedCharacterInfo->m_guild = 0;
            cachedCharacterInfo->guildRank = GUILD_RANK_NONE;
            cachedCharacterInfo->team = getSideByRace(cachedCharacterInfo->race);

            m_cachedCharacterInfo[cachedCharacterInfo->guid] = cachedCharacterInfo;

        } while (result->NextRow());
        delete result;
    }
    sLogger.info("ObjectMgr : %u players loaded.", static_cast<uint32_t>(m_cachedCharacterInfo.size()));
}

void ObjectMgr::addCachedCharacterInfo(const std::shared_ptr<CachedCharacterInfo>& _characterInfo)
{
    std::lock_guard guard(m_cachedCharacterLock);
    m_cachedCharacterInfo[_characterInfo->guid] = _characterInfo;
}

std::shared_ptr<CachedCharacterInfo> ObjectMgr::getCachedCharacterInfo(uint32_t _playerGuid)
{
    std::lock_guard guard(m_cachedCharacterLock);

    const auto characterPair = m_cachedCharacterInfo.find(_playerGuid);
    if (characterPair != m_cachedCharacterInfo.end())
        return characterPair->second;

    return nullptr;
}

std::shared_ptr<CachedCharacterInfo> ObjectMgr::getCachedCharacterInfoByName(std::string _playerName)
{
    std::string searchName = std::string(std::move(_playerName));
    AscEmu::Util::Strings::toLowerCase(searchName);

    std::lock_guard guard(m_cachedCharacterLock);

    for (const auto characterPair : m_cachedCharacterInfo)
    {
        std::string characterName = characterPair.second->name;
        AscEmu::Util::Strings::toLowerCase(characterName);
        if (characterName == searchName)
            return characterPair.second;
    }

    return nullptr;
}

void ObjectMgr::updateCachedCharacterInfoName(const std::shared_ptr<CachedCharacterInfo>& _characterInfo, const std::string& _newName)
{
    std::lock_guard guard(m_cachedCharacterLock);

    for (const auto& characterPair : m_cachedCharacterInfo)
        if (_characterInfo == characterPair.second)
            characterPair.second->name = _newName;
}

void ObjectMgr::deleteCachedCharacterInfo(const uint32_t _playerGuid)
{
    std::lock_guard guard(m_cachedCharacterLock);

    const auto characterPair = m_cachedCharacterInfo.find(_playerGuid);
    if (characterPair == m_cachedCharacterInfo.end())
        return;

    const auto characterInfo = characterPair->second;
    if (characterInfo->m_Group)
        characterInfo->m_Group->RemovePlayer(characterInfo);

    m_cachedCharacterInfo.erase(characterPair);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Corpse
void ObjectMgr::loadCorpsesForInstance(WorldMap* _worldMap) const
{
    if (QueryResult* result = CharacterDatabase.Query("SELECT * FROM corpses WHERE instanceid = %u", _worldMap->getInstanceId()))
    {
        do
        {
            Field* fields = result->Fetch();
            const auto corpse = std::make_shared<Corpse>(HIGHGUID_TYPE_CORPSE, fields[0].GetUInt32());
            corpse->SetPosition(fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            corpse->setZoneId(fields[5].GetUInt32());
            corpse->SetMapId(fields[6].GetUInt32());
            corpse->SetInstanceID(fields[7].GetUInt32());
            corpse->setCorpseDataFromDbString(fields[8].GetString());

            if (corpse->getDisplayId() == 0)
                continue;

            corpse->PushToWorld(_worldMap);
        } while (result->NextRow());

        delete result;
    }
}

std::shared_ptr<Corpse> ObjectMgr::loadCorpseByGuid(const uint32_t _corpseGuid) const
{
    if (QueryResult* result = CharacterDatabase.Query("SELECT * FROM corpses WHERE guid =%u ", _corpseGuid))
    {
        Field* field = result->Fetch();
        const auto corpse = std::make_shared<Corpse>(HIGHGUID_TYPE_CORPSE, field[0].GetUInt32());
        corpse->SetPosition(field[1].GetFloat(), field[2].GetFloat(), field[3].GetFloat(), field[4].GetFloat());
        corpse->setZoneId(field[5].GetUInt32());
        corpse->SetMapId(field[6].GetUInt32());
        corpse->setCorpseDataFromDbString(field[7].GetString());

        if (corpse->getDisplayId() == 0)
            return nullptr;

        corpse->setLoadedFromDB(true);
        corpse->SetInstanceID(field[8].GetUInt32());
        corpse->AddToWorld();

        delete result;
        return corpse;
    }

    return nullptr;
}

std::shared_ptr<Corpse> ObjectMgr::createCorpse()
{
    uint32_t corpseGuid = ++m_hiCorpseGuid;
    return std::make_shared<Corpse>(HIGHGUID_TYPE_CORPSE, corpseGuid);
}

void ObjectMgr::addCorpse(const std::shared_ptr<Corpse>& _corpse)
{
    std::lock_guard guard(m_corpseLock);
    m_corpses[_corpse->getGuidLow()] = _corpse;
}

void ObjectMgr::removeCorpse(const std::shared_ptr<Corpse>& _corpse)
{
    std::lock_guard guard(m_corpseLock);
    m_corpses.erase(_corpse->getGuidLow());
}

std::shared_ptr<Corpse> ObjectMgr::getCorpseByGuid(uint32_t _corpseGuid)
{
    std::lock_guard guard(m_corpseLock);
    const auto corpsePair = m_corpses.find(_corpseGuid);
    return corpsePair != m_corpses.end() ? corpsePair->second : nullptr;
}

std::shared_ptr<Corpse> ObjectMgr::getCorpseByOwner(const uint32_t _playerGuid)
{
    std::lock_guard guard(m_corpseLock);
    for (const auto& corpsePair : m_corpses)
    {
        WoWGuid wowGuid;
        wowGuid.Init(corpsePair.second->getOwnerGuid());

        if (wowGuid.getGuidLowPart() == _playerGuid)
            return corpsePair.second;
    }

    return nullptr;
}

void ObjectMgr::unloadCorpseCollector()
{
    std::lock_guard guard(m_corpseLock);
    for (const auto& corpsePair : m_corpses)
    {
        const auto corpse = corpsePair.second;
        if (corpse->IsInWorld())
            corpse->RemoveFromWorld(false);
    }
    m_corpses.clear();
}

void ObjectMgr::addCorpseDespawnTime(const std::shared_ptr<Corpse>& _corpse)
{
    if (_corpse->IsInWorld())
        _corpse->getWorldMap()->addCorpseDespawn(_corpse->getGuid(), 600000);
}

void ObjectMgr::delinkCorpseForPlayer(const Player* _player)
{
    if (const auto corpse = getCorpseByOwner(_player->getGuidLow()))
    {
        corpse->delink();
        addCorpseDespawnTime(corpse);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Vendors
void ObjectMgr::loadVendors()
{
    m_vendors.clear();

    QueryResult* result = sMySQLStore.getWorldDBQuery("SELECT * FROM vendors");
    if (result != nullptr)
    {
        std::shared_ptr<std::vector<CreatureItem>> items;

        if (result->GetFieldCount() < 6 + 1)
        {
            sLogger.failure("Invalid format in vendors (%u/6) columns, not enough data to proceed.", result->GetFieldCount());
            delete result;
            return;
        }

        if (result->GetFieldCount() > 6 + 1)
        {
            sLogger.failure("Invalid format in vendors (%u/6) columns, loading anyway because we have enough data", result->GetFieldCount());
        }


        DBC::Structures::ItemExtendedCostEntry const* item_extended_cost = nullptr;

        do
        {
            Field* fields = result->Fetch();

            auto itr = m_vendors.find(fields[0].GetUInt32());
            if (itr == m_vendors.end())
            {
                items = std::make_shared<std::vector<CreatureItem>>();
                m_vendors[fields[0].GetUInt32()] = items;
            }
            else
            {
                items = itr->second;
            }

            CreatureItem itm;
            itm.itemid = fields[1].GetUInt32();
            itm.amount = fields[2].GetUInt32();
            itm.available_amount = fields[3].GetUInt32();
            itm.max_amount = fields[3].GetUInt32();
            itm.incrtime = fields[4].GetUInt32();
            if (fields[5].GetUInt32() > 0)
            {
                item_extended_cost = sItemExtendedCostStore.LookupEntry(fields[5].GetUInt32());
                if (item_extended_cost == nullptr)
                    sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "LoadVendors : Extendedcost for item %u references nonexistent EC %u", fields[1].GetUInt32(), fields[5].GetUInt32());
            }

            itm.extended_cost = item_extended_cost;
            items->push_back(itm);
        } while (result->NextRow());

        delete result;
    }
    sLogger.info("ObjectMgr : %u vendors loaded.", static_cast<uint32_t>(m_vendors.size()));
}

std::shared_ptr<std::vector<CreatureItem>> ObjectMgr::getVendorList(uint32_t _entry)
{
    return m_vendors[_entry];
}

void ObjectMgr::setVendorList(uint32_t _entry, std::shared_ptr<std::vector<CreatureItem>> _list)
{
    m_vendors[_entry] = _list;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Achievement
#if VERSION_STRING > TBC
void ObjectMgr::loadAchievementCriteriaList()
{
    for (uint32_t rowId = 0; rowId < sAchievementCriteriaStore.GetNumRows(); ++rowId)
    {
        auto criteria = sAchievementCriteriaStore.LookupEntry(rowId);
        if (!criteria)
            continue;

#if VERSION_STRING > WotLK
        auto achievement = sAchievementStore.LookupEntry(criteria->referredAchievement);
        if (achievement && achievement->flags & ACHIEVEMENT_FLAG_GUILD)
            m_GuildAchievementCriteriasByType[criteria->requiredType].push_back(criteria);
        else
#endif
            m_AchievementCriteriasByType[criteria->requiredType].push_back(criteria);
    }
}

void ObjectMgr::loadAchievementRewards()
{
    m_achievementRewards.clear();

    QueryResult* result = WorldDatabase.Query("SELECT entry, gender, title_A, title_H, item, sender, subject, text FROM achievement_reward");

    if (!result)
    {
        sLogger.info("Loaded 0 achievement rewards. DB table `achievement_reward` is empty.");
        return;
    }

    uint32_t count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32_t entry = fields[0].GetUInt32();

        if (!sAchievementStore.LookupEntry(entry))
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : Achievement reward entry %u has wrong achievement, ignore", entry);
            continue;
        }

        AchievementReward reward;
        reward.gender = fields[1].GetUInt8();
        reward.titel_A = fields[2].GetUInt32();
        reward.titel_H = fields[3].GetUInt32();
        reward.itemId = fields[4].GetUInt32();
        reward.sender = fields[5].GetUInt32();
        reward.subject = fields[6].GetString() ? fields[6].GetString() : "";
        reward.text = fields[7].GetString() ? fields[7].GetString() : "";

        if (reward.gender > 2)
            sLogger.debug("ObjectMgr : achievement reward %u has wrong gender %u.", entry, static_cast<uint32_t>(reward.gender));

        bool dup = false;
        AchievementRewardsMapBounds bounds = m_achievementRewards.equal_range(entry);
        for (AchievementRewardsMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
        {
            if (iter->second.gender == 2 || reward.gender == 2)
            {
                dup = true;
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : Achievement reward %u must have single GENDER_NONE (%u), ignore duplicate case", 2, entry);
                break;
            }
        }

        if (dup)
            continue;

        // must be title or mail at least
        if (!reward.titel_A && !reward.titel_H && !reward.sender)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have title or item reward data, ignore.", entry);
            continue;
        }

        if (reward.titel_A)
        {
            auto const* char_title_entry = sCharTitlesStore.LookupEntry(reward.titel_A);
            if (!char_title_entry)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid title id (%u) in `title_A`, set to 0", entry, reward.titel_A);
                reward.titel_A = 0;
            }
        }

        if (reward.titel_H)
        {
            auto const* char_title_entry = sCharTitlesStore.LookupEntry(reward.titel_H);
            if (!char_title_entry)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid title id (%u) in `title_A`, set to 0", entry, reward.titel_H);
                reward.titel_H = 0;
            }
        }

        //check mail data before item for report including wrong item case
        if (reward.sender)
        {
            if (!sMySQLStore.getCreatureProperties(reward.sender))
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid creature entry %u as sender, mail reward skipped.", entry, reward.sender);
                reward.sender = 0;
            }
        }
        else
        {
            if (reward.itemId)
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have sender data but have item reward, item will not rewarded", entry);

            if (!reward.subject.empty())
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have sender data but have mail subject.", entry);

            if (!reward.text.empty())
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have sender data but have mail text.", entry);
        }

        if (reward.itemId == 0)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid item id %u, reward mail will be without item.", entry, reward.itemId);
        }

        m_achievementRewards.insert(AchievementRewardsMap::value_type(entry, reward));
        ++count;

    } while (result->NextRow());

    delete result;

    sLogger.info("ObjectMgr : Loaded %u achievement rewards", count);
}

void ObjectMgr::loadCompletedAchievements()
{
    QueryResult* result = CharacterDatabase.Query("SELECT achievement FROM character_achievement GROUP BY achievement");

    if (!result)
    {
        sLogger.failure("Query failed: SELECT achievement FROM character_achievement");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        m_allCompletedAchievements.insert(fields[0].GetUInt32());
    } while (result->NextRow());
    delete result;
}

AchievementReward const* ObjectMgr::getAchievementReward(uint32_t _entry, uint8_t _gender)
{
    AchievementRewardsMapBounds bounds = m_achievementRewards.equal_range(_entry);
    for (AchievementRewardsMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
    {
        if (iter->second.gender == 2 || iter->second.gender == _gender)
            return &iter->second;
    }
    return nullptr;
}

AchievementCriteriaEntryList const& ObjectMgr::getAchievementCriteriaByType(AchievementCriteriaTypes _type)
{
    return m_AchievementCriteriasByType[_type];
}

void ObjectMgr::addCompletedAchievement(uint32_t _achievementId)
{
    m_allCompletedAchievements.insert(_achievementId);
}

bool ObjectMgr::isInCompletedAchievements(uint32_t _achievementId)
{
    auto const achievementItr = m_allCompletedAchievements.find(_achievementId);
    if (achievementItr != m_allCompletedAchievements.end())
        return true;
    return false;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// Reputation Mods
void ObjectMgr::loadReputationModifiers()
{
    loadReputationModifierTable("reputation_creature_onkill", m_reputationCreature);
    loadReputationModifierTable("reputation_faction_onkill", m_reputationFaction);
    loadInstanceReputationModifiers();
}

void ObjectMgr::loadReputationModifierTable(const char* _tableName, ReputationModMap& _reputationModMap)
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM %s", _tableName);
    if (result)
    {
        do
        {
            auto reputationMod = std::make_shared<ReputationMod>();
            reputationMod->faction[TEAM_ALLIANCE] = result->Fetch()[1].GetUInt32();
            reputationMod->faction[TEAM_HORDE] = result->Fetch()[2].GetUInt32();
            reputationMod->value = result->Fetch()[3].GetInt32();
            reputationMod->replimit = result->Fetch()[4].GetUInt32();

            auto itr = _reputationModMap.find(result->Fetch()[0].GetUInt32());
            if (itr == _reputationModMap.end())
            {
                auto modifier = std::make_shared<ReputationModifier>();
                modifier->entry = result->Fetch()[0].GetUInt32();

                modifier->mods.push_back(reputationMod);

                _reputationModMap.insert(std::pair(result->Fetch()[0].GetUInt32(), modifier));
            }
            else
            {
                itr->second->mods.push_back(reputationMod);
            }

        } while (result->NextRow());
        delete result;
    }
    sLogger.info("ObjectMgr : %u reputation modifiers on %s.", static_cast<uint32_t>(_reputationModMap.size()), _tableName);
}

void ObjectMgr::loadInstanceReputationModifiers()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM reputation_instance_onkill");
    if (!result)
        return;
    do
    {
        Field* field = result->Fetch();

        auto reputationMod = std::make_shared<InstanceReputationMod>();
        reputationMod->mapid = field[0].GetUInt32();
        reputationMod->mob_rep_reward = field[1].GetInt32();
        reputationMod->mob_rep_limit = field[2].GetUInt32();
        reputationMod->boss_rep_reward = field[3].GetInt32();
        reputationMod->boss_rep_limit = field[4].GetUInt32();
        reputationMod->faction[TEAM_ALLIANCE] = field[5].GetUInt32();
        reputationMod->faction[TEAM_HORDE] = field[6].GetUInt32();

        auto itr = m_reputationInstance.find(reputationMod->mapid);
        if (itr == m_reputationInstance.end())
        {
            auto modifier = std::make_shared<InstanceReputationModifier>();
            modifier->mapid = reputationMod->mapid;
            modifier->mods.push_back(reputationMod);
            m_reputationInstance.insert(std::make_pair(modifier->mapid, modifier));
        }
        else
        {
            itr->second->mods.push_back(reputationMod);
        }

    } while (result->NextRow());
    delete result;

    sLogger.info("ObjectMgr : %u instance reputation modifiers loaded.", static_cast<uint32_t>(m_reputationInstance.size()));
}

std::shared_ptr<ReputationModifier> ObjectMgr::getReputationModifier(uint32_t _entry, uint32_t _factionId)
{
    auto reputationPair = m_reputationCreature.find(_entry);
    if (reputationPair != m_reputationCreature.end())
        return reputationPair->second;

    reputationPair = m_reputationFaction.find(_factionId);
    if (reputationPair != m_reputationFaction.end())
        return reputationPair->second;

    return nullptr;
}

bool ObjectMgr::handleInstanceReputationModifiers(Player* _player, Unit* _unitVictim)
{
    const uint32_t team = _player->getTeam();

    if (!_unitVictim->isCreature())
        return false;

    const auto itr = m_reputationInstance.find(_unitVictim->GetMapId());
    if (itr == m_reputationInstance.end())
        return false;

    bool isBoss = false;
    if (dynamic_cast<Creature*>(_unitVictim)->GetCreatureProperties()->isBoss)
        isBoss = true;

    int32_t repLimit;
    int32_t value;

    for (const auto& instanceRepMod : itr->second->mods)
    {
        if (!instanceRepMod->faction[team])
            continue;

        if (isBoss)
        {
            value = instanceRepMod->boss_rep_reward;
            repLimit = instanceRepMod->boss_rep_limit;
        }
        else
        {
            value = instanceRepMod->mob_rep_reward;
            repLimit = instanceRepMod->mob_rep_limit;
        }

        if (!value || (repLimit && _player->getFactionStanding(instanceRepMod->faction[team]) >= repLimit))
            continue;

        value = Util::float2int32(value * worldConfig.getFloatRate(RATE_KILLREPUTATION));
        _player->modFactionStanding(instanceRepMod->faction[team], value);
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Group
void ObjectMgr::loadGroups()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM `groups`");
    if (result)
    {
        if (result->GetFieldCount() != 52)
        {
            sLogger.failure("groups table format is invalid. Please update your database.");
            return;
        }
        do
        {
            const auto group = std::make_shared<Group>(false);
            group->LoadFromDB(result->Fetch());
        } while (result->NextRow());
        delete result;
    }

    sLogger.info("ObjectMgr : %u groups loaded.", static_cast<uint32_t>(this->m_groups.size()));
}

void ObjectMgr::loadGroupInstances()
{
    CharacterDatabase.Execute("DELETE FROM group_instance WHERE guid NOT IN (SELECT guid FROM `groups`)");

    QueryResult* result = CharacterDatabase.Query("SELECT gi.guid, i.map, gi.instance, gi.permanent, i.difficulty, i.resettime, (SELECT COUNT(1) FROM character_instance ci LEFT JOIN `groups` g ON ci.guid = g.group1member1 WHERE ci.instance = gi.instance AND ci.permanent = 1 LIMIT 1) FROM group_instance gi LEFT JOIN instance i ON gi.instance = i.id ORDER BY guid");
    if (!result)
    {
        sLogger.info("Loaded 0 group-instance saves. DB table `group_instance` is empty!");
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();
        std::shared_ptr<Group> group = sObjectMgr.getGroupById(fields[0].GetUInt32());

        DBC::Structures::MapEntry const* mapEntry = sMapStore.LookupEntry(fields[1].GetUInt16());
        if (!mapEntry || !mapEntry->isDungeon())
        {
            sLogger.failure("Incorrect entry in group_instance table : no dungeon map %d", fields[1].GetUInt16());
            continue;
        }

        uint32_t diff = fields[4].GetUInt8();
        if (diff >= static_cast<uint32_t>(mapEntry->isRaid() ? InstanceDifficulty::Difficulties::MAX_RAID_DIFFICULTY : InstanceDifficulty::Difficulties::MAX_DUNGEON_DIFFICULTY))
        {
            sLogger.failure("Wrong dungeon difficulty use in group_instance table: %d", diff + 1);
            diff = 0;                                   // default for both difficaly types
        }

        InstanceSaved* save = sInstanceMgr.addInstanceSave(mapEntry->id, fields[2].GetUInt32(), InstanceDifficulty::Difficulties(diff), time_t(fields[5].GetUInt64()), fields[6].GetUInt64() == 0, true);
        group->bindToInstance(save, fields[3].GetBool(), true);
        ++count;
    } while (result->NextRow());
    delete result;

    sLogger.info("Loaded %u group-instance saves", count);
}

uint32_t ObjectMgr::generateGroupId()
{
    uint32_t groupId = ++m_hiGroupId;
    return groupId;
}

void ObjectMgr::addGroup(std::shared_ptr<Group> _group)
{
    std::lock_guard guard(m_groupLock);
    m_groups.insert(std::make_pair(_group->GetID(), _group));
}

void ObjectMgr::removeGroup(std::shared_ptr<Group> _group)
{
    std::lock_guard guard(m_groupLock);
    m_groups.erase(_group->GetID());
}

std::shared_ptr<Group> ObjectMgr::getGroupByLeader(Player* pPlayer)
{
    std::lock_guard guard(m_groupLock);

    for (auto& m_group : m_groups)
        if (m_group.second->GetLeader() == pPlayer->getPlayerInfo())
            return m_group.second;

    return nullptr;
}

std::shared_ptr<Group> ObjectMgr::getGroupById(uint32_t _id)
{
    std::lock_guard guard(m_groupLock);

    const auto itr = m_groups.find(_id);
    if (itr != m_groups.end())
        return itr->second;

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Player
Player* ObjectMgr::createPlayer(uint8_t _class)
{
    const uint32_t guid = ++m_hiPlayerGuid;
    return createPlayerByGuid(_class, guid);
}

Player* ObjectMgr::createPlayerByGuid(uint8_t _class, uint32_t _guid)
{
    Player* player;

    switch (_class)
    {
        case WARRIOR:
            player = new Warrior(_guid);
            break;
        case PALADIN:
            player = new Paladin(_guid);
            break;
        case HUNTER:
            player = new Hunter(_guid);
            break;
        case ROGUE:
            player = new Rogue(_guid);
            break;
        case PRIEST:
            player = new Priest(_guid);
            break;
#if VERSION_STRING > TBC
        case DEATHKNIGHT:
            player = new DeathKnight(_guid);
            break;
#endif
        case SHAMAN:
            player = new Shaman(_guid);
            break;
        case MAGE:
            player = new Mage(_guid);
            break;
        case WARLOCK:
            player = new Warlock(_guid);
            break;
#if VERSION_STRING > Cata
        case MONK:
            player = new Monk(_guid);
            break;
#endif
        case DRUID:
            player = new Druid(_guid);
            break;
        default:
            player = nullptr;
            break;
    }

    return player;
}

Player* ObjectMgr::getPlayer(const char* _name, bool _caseSensitive)
{
    std::lock_guard guard(m_playerLock);

    for (const auto player : m_players)
    {
        std::string searchName = _name;
        std::string availableName = player.second->getName();

        if (!_caseSensitive)
        {
            AscEmu::Util::Strings::toLowerCase(searchName);
            AscEmu::Util::Strings::toLowerCase(availableName);
        }

        if (availableName == searchName)
            return player.second;
    }

    return nullptr;
}

Player* ObjectMgr::getPlayer(uint32_t guid)
{
    std::lock_guard guard(m_playerLock);

    const auto playerPair = m_players.find(guid);
    return playerPair != m_players.end() ? playerPair->second : nullptr;
}

void ObjectMgr::addPlayer(Player* _player)
{
    std::lock_guard guard(m_playerLock);
    m_players[_player->getGuidLow()] = _player;
}

void ObjectMgr::removePlayer(Player* _player)
{
    std::lock_guard guard(m_playerLock);
    m_players.erase(_player->getGuidLow());
}

void ObjectMgr::resetDailies()
{
    std::lock_guard guard(m_playerLock);

    for (const auto playerPair : m_players)
        if (Player* player = playerPair.second)
            player->resetFinishedDailies();
}

std::unordered_map<uint32_t, Player*> ObjectMgr::getPlayerStorage()
{
    return m_players;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Vehicle
#ifdef FT_VEHICLES
void ObjectMgr::loadVehicleAccessories()
{
    m_vehicleAccessoryStore.clear();

    QueryResult* result = WorldDatabase.Query("SELECT entry, accessory_entry, seat_id , minion, summontype, summontimer FROM vehicle_accessories;");
    if (result != nullptr)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32_t entry = fields[0].GetUInt32();
            uint32_t accessory = fields[1].GetUInt32();
            int8_t seatId = fields[2].GetInt8();
            bool isMinion = fields[3].GetBool();
            uint8_t summonType = fields[4].GetUInt8();
            uint32_t summonTimer = fields[5].GetUInt32();

            if (!sMySQLStore.getCreatureProperties(entry))
            {
                sLogger.failure("Table `vehicle_accessories`: creature template entry %u does not exist.", entry);
                continue;
            }

            if (!sMySQLStore.getCreatureProperties(accessory))
            {
                sLogger.failure("Table `vehicle_accessories`: Accessory %u does not exist.", accessory);
                continue;
            }

            const auto spellClickInfoStore = sMySQLStore.getSpellClickSpellsStore();
            if (!spellClickInfoStore->contains(entry))
            {
                sLogger.failure("Table `vehicle_accessories`: creature template entry %u has no data in npc_spellclick_spells", entry);
                continue;
            }

            m_vehicleAccessoryStore[entry].push_back(VehicleAccessory(accessory, seatId, isMinion, summonType, summonTimer));

        } while (result->NextRow());

        delete result;
    }
}

VehicleAccessoryList const* ObjectMgr::getVehicleAccessories(uint32_t _entry)
{
    const auto vehicleAccessoriesPair = m_vehicleAccessoryStore.find(_entry);
    if (vehicleAccessoriesPair != m_vehicleAccessoryStore.end())
        return &vehicleAccessoriesPair->second;

    return nullptr;
}

void ObjectMgr::loadVehicleSeatAddon()
{
    m_vehicleSeatAddonStore.clear();

    QueryResult* result = WorldDatabase.Query("SELECT SeatEntry, SeatOrientation, ExitParamX , ExitParamY, ExitParamZ, ExitParamO, ExitParamValue FROM vehicle_seat_addon;");
    if (result != nullptr)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32_t seatID = fields[0].GetUInt32();
            float orientation = fields[1].GetFloat();
            float exitX = fields[2].GetFloat();
            float exitY = fields[3].GetFloat();
            float exitZ = fields[4].GetFloat();
            float exitO = fields[5].GetFloat();
            uint8_t exitParam = fields[6].GetUInt8();

            m_vehicleSeatAddonStore[seatID] = VehicleSeatAddon(orientation, { exitX, exitY, exitZ, exitO }, exitParam);

        } while (result->NextRow());

        delete result;
    }
}

VehicleSeatAddon const* ObjectMgr::getVehicleSeatAddon(uint32_t _seatId) const
{
    const auto vehicleSeatAddonPair = m_vehicleSeatAddonStore.find(_seatId);
    if (vehicleSeatAddonPair != m_vehicleSeatAddonStore.end())
        return &vehicleSeatAddonPair->second;

    return nullptr;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// EventScripts
void ObjectMgr::loadEventScripts()
{
    sLogger.info("ObjectMgr : Loading Event Scripts...");

    bool success = false;
    const char* eventScriptsQuery = "SELECT `event_id`, `function`, `script_type`, `data_1`, `data_2`, `data_3`, `data_4`, `data_5`, `x`, `y`, `z`, `o`, `delay`, `next_event` FROM `event_scripts` WHERE `event_id` > 0 ORDER BY `event_id`";
    const auto result = WorldDatabase.Query(&success, eventScriptsQuery);

    if (!success)
    {
        sLogger.debug("LoadEventScripts : Failed on Loading Queries from event_scripts.");
        return;
    }

    if (!result)
    {
        sLogger.debug("LoadEventScripts : Loaded 0 event_scripts. DB table `event_scripts` is empty.");
        delete result;
        return;
    }

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t event_id = fields[0].GetUInt32();
        SimpleEventScript eventscript;
        eventscript.eventId = event_id;
        eventscript.function = static_cast<ScriptCommands>(fields[1].GetUInt8());
        eventscript.scripttype = static_cast<EasyScriptTypes>(fields[2].GetUInt8());
        eventscript.data_1 = fields[3].GetUInt32();
        eventscript.data_2 = fields[4].GetUInt32();
        eventscript.data_3 = fields[5].GetUInt32();
        eventscript.data_4 = fields[6].GetUInt32();
        eventscript.data_5 = fields[7].GetUInt32();
        eventscript.x = fields[8].GetUInt32();
        eventscript.y = fields[9].GetUInt32();
        eventscript.z = fields[10].GetUInt32();
        eventscript.o = fields[11].GetUInt32();
        eventscript.delay = fields[12].GetUInt32();
        eventscript.nextevent = fields[13].GetUInt32();

        SimpleEventScript* SimpleEventScript = &m_eventScriptMaps.insert(EventScriptMaps::value_type(event_id, eventscript))->second;

        // for search by spellid ( data_1 is spell id )
        if (eventscript.data_1 && eventscript.scripttype == EasyScriptTypes::SCRIPT_TYPE_SPELL_EFFECT)
            m_spellEffectMaps.insert(SpellEffectMaps::value_type(eventscript.data_1, SimpleEventScript));

        ++count;

    } while (result->NextRow());

    delete result;

    sLogger.info("ObjectMgr : Loaded event_scripts for %u events...", count);
}

EventScriptBounds ObjectMgr::getEventScripts(uint32_t _eventId) const
{
    return EventScriptBounds(m_eventScriptMaps.lower_bound(_eventId), m_eventScriptMaps.upper_bound(_eventId));
}

SpellEffectMapBounds ObjectMgr::getSpellEffectBounds(uint32_t _data1) const
{
    return SpellEffectMapBounds(m_spellEffectMaps.lower_bound(_data1), m_spellEffectMaps.upper_bound(_data1));
}

bool ObjectMgr::checkForScripts(Player* _player, uint32_t _eventId)
{
    const EventScriptBounds eventScript = sObjectMgr.getEventScripts(_eventId);
    if (eventScript.first == eventScript.second)
        return false;

    for (auto itr = eventScript.first; itr != eventScript.second; ++itr)
        sEventMgr.AddEvent(this, &ObjectMgr::eventScriptsUpdate, _player, itr->second.eventId, EVENT_EVENT_SCRIPTS, itr->second.delay, 1, 0);

    return true;
}

bool ObjectMgr::checkForDummySpellScripts(Player* _player, uint32_t _data1)
{
    const SpellEffectMapBounds eventScript = sObjectMgr.getSpellEffectBounds(_data1);
    if (eventScript.first == eventScript.second)
        return false;

    for (auto itr = eventScript.first; itr != eventScript.second; ++itr)
        sEventMgr.AddEvent(this, &ObjectMgr::eventScriptsUpdate, _player, itr->second->eventId, EVENT_EVENT_SCRIPTS, itr->second->delay, 1, 0);

    return true;
}

void ObjectMgr::eventScriptsUpdate(Player* _player, uint32_t _nextEvent)
{
    const EventScriptBounds eventScript = sObjectMgr.getEventScripts(_nextEvent);

    for (auto itr = eventScript.first; itr != eventScript.second; ++itr)
    {
        if (itr->second.scripttype == EasyScriptTypes::SCRIPT_TYPE_SPELL_EFFECT || itr->second.scripttype == EasyScriptTypes::SCRIPT_TYPE_DUMMY)
        {
            switch (itr->second.function)
            {
                case ScriptCommands::SCRIPT_COMMAND_RESPAWN_GAMEOBJECT:
                {
                    Object* target = _player->getWorldMap()->getInterface()->getGameObjectNearestCoords(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), itr->second.data_1);
                    if (target == nullptr)
                        return;

                    dynamic_cast<GameObject*>(target)->despawn(1000, itr->second.data_2);
                } break;
                case ScriptCommands::SCRIPT_COMMAND_KILL_CREDIT:
                {
                    if (auto* questLog = _player->getQuestLogByQuestId(itr->second.data_2))
                    {
                        if (questLog->getQuestProperties()->required_mob_or_go[itr->second.data_5] >= 0)
                        {
                            const uint32_t requiredMob = questLog->getQuestProperties()->required_mob_or_go[itr->second.data_5];
                            const auto index = static_cast<uint8_t>(itr->second.data_5);
                            if (questLog->getMobCountByIndex(index) < requiredMob)
                            {
                                questLog->setMobCountForIndex(index, questLog->getMobCountByIndex(index) + 1);
                                questLog->sendUpdateAddKill(index);
                                questLog->updatePlayerFields();
                            }
                        }
                    }
                } break;
                default:
                    break;
            }
        }

        if (itr->second.scripttype == EasyScriptTypes::SCRIPT_TYPE_GAMEOBJECT || itr->second.scripttype == EasyScriptTypes::SCRIPT_TYPE_DUMMY)
        {
            switch (itr->second.function)
            {
                case ScriptCommands::SCRIPT_COMMAND_ACTIVATE_OBJECT:
                {
                    GameObject* gameObject;
                    MapScriptInterface* mapScript = _player->getWorldMap()->getInterface();
                    if ((itr->second.x || itr->second.y || itr->second.z) == 0)
                        gameObject = mapScript->getGameObjectNearestCoords(_player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), itr->second.data_1);
                    else
                        gameObject = mapScript->getGameObjectNearestCoords(static_cast<float>(itr->second.x), static_cast<float>(itr->second.y), static_cast<float>(itr->second.z), itr->second.data_1);

                    if (gameObject == nullptr)
                        return;

                    if (gameObject->getState() != GO_STATE_OPEN)
                        gameObject->setState(GO_STATE_OPEN);
                    else
                        gameObject->setState(GO_STATE_CLOSED);

                } break;
                default:
                    break;
            }
        }

        if (itr->second.nextevent != 0)
            sObjectMgr.checkForScripts(_player, itr->second.nextevent);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
// Misc
void ObjectMgr::generateDatabaseGossipMenu(Object* _object, uint32_t _gossipMenuId, Player* _player, uint32_t _forcedTextId /*= 0*/)
{
    uint32_t textId = 2;

    if (_forcedTextId == 0)
    {
        auto gossipMenuTextStore = sMySQLStore.getGossipMenuInitTextId();
        for (auto& initItr : *gossipMenuTextStore)
        {
            if (initItr.first == _gossipMenuId)
            {
                textId = initItr.second.textId;
                break;
            }
        }
    }
    else
    {
        textId = _forcedTextId;
    }

    GossipMenu menu(_object->getGuid(), textId, _player->getSession()->language, _gossipMenuId);

    sQuestMgr.FillQuestMenu(dynamic_cast<Creature*>(_object), _player, menu);

    typedef MySQLDataStore::GossipMenuItemsContainer::iterator GossipMenuItemsIterator;
    std::pair<GossipMenuItemsIterator, GossipMenuItemsIterator> gossipEqualRange = sMySQLStore._gossipMenuItemsStores.equal_range(_gossipMenuId);
    for (GossipMenuItemsIterator itr = gossipEqualRange.first; itr != gossipEqualRange.second; ++itr)
    {
        // check requirements
        // 0 = none
        // 1 = has(active)Quest
        // 2 = has(finished)Quest
        // 3 = canGainXP
        // 4 = canNotGainXP

        if (itr->first == _gossipMenuId)
        {
            auto& gossipMenuItem = itr->second;
            if (gossipMenuItem.requirementType == 1 && !_player->hasQuestInQuestLog(gossipMenuItem.requirementData))
                continue;

            if (gossipMenuItem.requirementType == 3)
            {
                if (_player->canGainXp())
                    menu.addItem(gossipMenuItem.icon, gossipMenuItem.menuOptionText, gossipMenuItem.itemOrder, "", gossipMenuItem.onChooseData, _player->getSession()->LocalizedGossipOption(gossipMenuItem.onChooseData2));

                continue;
            }

            if (gossipMenuItem.requirementType == 4)
            {
                if (!_player->canGainXp())
                    menu.addItem(gossipMenuItem.icon, gossipMenuItem.menuOptionText, gossipMenuItem.itemOrder, "", gossipMenuItem.onChooseData, _player->getSession()->LocalizedGossipOption(gossipMenuItem.onChooseData2));

                continue;
            }

            menu.addItem(gossipMenuItem.icon, gossipMenuItem.menuOptionText, gossipMenuItem.itemOrder);
        }
    }

    menu.sendGossipPacket(_player);
}

void ObjectMgr::generateDatabaseGossipOptionAndSubMenu(Object* _object, Player* _player, uint32_t _gossipItemId, uint32_t _gossipMenuId)
{
    sLogger.debug("GossipId: %u  gossipItemId: %u", _gossipMenuId, _gossipItemId);

    // bool openSubMenu = true;

    typedef MySQLDataStore::GossipMenuItemsContainer::iterator GossipMenuItemsIterator;
    std::pair<GossipMenuItemsIterator, GossipMenuItemsIterator> gossipEqualRange = sMySQLStore._gossipMenuItemsStores.equal_range(_gossipMenuId);
    for (GossipMenuItemsIterator itr = gossipEqualRange.first; itr != gossipEqualRange.second; ++itr)
    {
        if (itr->second.itemOrder == _gossipItemId)
        {
            // onChooseAction
            // 0 = None
            // 1 = sendPoiById (on_choose_data = poiId)
            // 2 = castSpell (on_choose_data = spellId)
            // 3 = sendTaxi (on_choose_data = taxiId, on_choose_data2 = modelId)
            // 4 = required standing (on_choose_data = factionId, on_choose_data2 = standing, on_choose_data3 = broadcastTextId)
            // 5 = close window
            // 6 = toggleXPGain

            // onChooseData
            // depending on Action...
            switch (itr->second.onChooseAction)
            {
            case 1:
            {
                generateDatabaseGossipMenu(_object, itr->second.nextGossipMenu, _player, itr->second.nextGossipMenuText);

                if (itr->second.onChooseData != 0)
                    _player->sendPoiById(itr->second.onChooseData);

            } break;
            case 2:
            {
                if (itr->second.onChooseData != 0)
                {
                    _player->castSpell(_player, sSpellMgr.getSpellInfo(itr->second.onChooseData), true);
                    GossipMenu::senGossipComplete(_player);
                }

            } break;
            case 3:
            {
                if (itr->second.onChooseData != 0)
                {
                    if (_object->isCreature())
                        _player->getSession()->sendTaxiMenu(_object->ToCreature());

                    GossipMenu::senGossipComplete(_player);
                }

            } break;
            case 4:
            {
                if (itr->second.onChooseData != 0)
                {
                    if (_player->getFactionStanding(itr->second.onChooseData) >= static_cast<int32_t>(itr->second.onChooseData2))
                        _player->castSpell(_player, sSpellMgr.getSpellInfo(itr->second.onChooseData3), true);
                    else
                        _player->broadcastMessage(_player->getSession()->LocalizedWorldSrv(itr->second.onChooseData4));

                    GossipMenu::senGossipComplete(_player);
                }

            } break;
            case 5:
            {
                GossipMenu::senGossipComplete(_player);

            } break;
            case 6:
            {
                if (_player->hasEnoughCoinage(itr->second.onChooseData))
                {
                    _player->modCoinage(-static_cast<int32_t>(itr->second.onChooseData));
                    _player->toggleXpGain();
                    GossipMenu::senGossipComplete(_player);
                }
            } break;
            default: // action 0
            {
                generateDatabaseGossipMenu(_object, itr->second.nextGossipMenu, _player, itr->second.nextGossipMenuText);
            } break;
            }
        }
    }
}

void ObjectMgr::loadTrainerSpellSets()
{
    auto* const spellSetResult = sMySQLStore.getWorldDBQuery("SELECT * FROM trainer_properties_spellset WHERE min_build <= %u AND max_build >= %u;", VERSION_STRING, VERSION_STRING);
    if (spellSetResult != nullptr)
    {
        std::shared_ptr<std::vector<TrainerSpell>> trainerSpells;

        do
        {
            Field* fields = spellSetResult->Fetch();

            auto spellSetPair = m_trainerSpellSet.find(fields[0].GetUInt32());

            if (spellSetPair == m_trainerSpellSet.end())
            {
                trainerSpells = std::make_shared<std::vector<TrainerSpell>>();
                m_trainerSpellSet[fields[0].GetUInt32()] = trainerSpells;
            }
            else
            {
                trainerSpells = spellSetPair->second;
            }

            auto* const fields2 = spellSetResult->Fetch();

            auto castSpellID = fields2[3].GetUInt32();
            auto learnSpellID = fields2[4].GetUInt32();

            TrainerSpell ts;
            auto abrt = false;
            if (castSpellID != 0)
            {
                ts.castSpell = sSpellMgr.getSpellInfo(castSpellID);
                if (ts.castSpell != nullptr)
                {
                    // Check that the castable spell has learn spell effect
                    for (uint8_t i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    {
                        if (ts.castSpell->getEffect(i) == SPELL_EFFECT_LEARN_SPELL)
                        {
                            ts.castRealSpell = sSpellMgr.getSpellInfo(ts.castSpell->getEffectTriggerSpell(i));
                            if (ts.castRealSpell == nullptr)
                            {
                                sLogger.failure("LoadTrainers : TrainerSpellSet %u contains cast spell %u that is non-teaching", fields[0].GetUInt32(), castSpellID);
                                abrt = true;
                            }

                            break;
                        }
                    }
                }

                if (abrt)
                    continue;
            }

            if (learnSpellID != 0)
                ts.learnSpell = sSpellMgr.getSpellInfo(learnSpellID);

            if (ts.castSpell == nullptr && ts.learnSpell == nullptr)
                continue;

            if (ts.castSpell != nullptr && ts.castRealSpell == nullptr)
                continue;

            ts.cost = fields2[5].GetUInt32();
            ts.requiredSpell[0] = fields2[6].GetUInt32();
            ts.requiredSkillLine = fields2[7].GetUInt16();
            ts.requiredSkillLineValue = fields2[8].GetUInt32();
            ts.requiredLevel = fields2[9].GetUInt32();
            ts.deleteSpell = fields2[10].GetUInt32();
            ts.isStatic = fields2[11].GetUInt32();

            // Check if spell teaches a primary profession skill
            if (ts.requiredSkillLine == 0 && ts.castRealSpell != nullptr)
                ts.isPrimaryProfession = ts.castRealSpell->isPrimaryProfession();

            // Add all required spells
            const auto spellInfo = ts.castRealSpell != nullptr ? ts.castSpell : ts.learnSpell;
            const auto requiredSpells = sSpellMgr.getSpellsRequiredForSpellBounds(spellInfo->getId());
            for (auto itr = requiredSpells.first; itr != requiredSpells.second; ++itr)
            {
                for (uint8_t i = 0; i < 3; ++i)
                {
                    if (ts.requiredSpell[i] == itr->second)
                        break;

                    if (ts.requiredSpell[i] != 0)
                        continue;

                    ts.requiredSpell[i] = itr->second;
                    break;
                }
            }

            trainerSpells->push_back(ts);
        } while (spellSetResult->NextRow());

        sLogger.info("LoadTrainers : %u TrainerSpellSet loaded", static_cast<uint32_t>(m_trainerSpellSet.size()));
    }
}

std::shared_ptr<std::vector<TrainerSpell>> ObjectMgr::getTrainerSpellSetById(uint32_t _id)
{
    auto itr = m_trainerSpellSet.find(_id);
    if (itr == m_trainerSpellSet.end())
        return {};

    return itr->second;
}

void ObjectMgr::loadTrainers()
{
    std::string normalTalkMessage = "DMSG";

    if (auto* const trainerResult = sMySQLStore.getWorldDBQuery("SELECT * FROM trainer_properties WHERE build <= %u;", VERSION_STRING))
    {
        do
        {
            auto* const fields = trainerResult->Fetch();
            const auto entry = fields[0].GetUInt32();

            std::shared_ptr<Trainer> trainer = std::make_shared<Trainer>();
            trainer->RequiredSkill = fields[2].GetUInt16();
            trainer->RequiredSkillLine = fields[3].GetUInt32();
            trainer->RequiredClass = fields[4].GetUInt32();
            trainer->RequiredRace = fields[5].GetUInt32();
            trainer->RequiredRepFaction = fields[6].GetUInt32();
            trainer->RequiredRepValue = fields[7].GetUInt32();
            trainer->TrainerType = fields[8].GetUInt32();
            trainer->Can_Train_Gossip_TextId = fields[10].GetUInt32();
            trainer->Cannot_Train_GossipTextId = fields[11].GetUInt32();
            trainer->spellset_id = fields[12].GetUInt32();
            trainer->can_train_max_level = fields[13].GetUInt32();
            trainer->can_train_min_skill_value = fields[14].GetUInt32();
            trainer->can_train_max_skill_value = fields[15].GetUInt32();

            if (!trainer->Can_Train_Gossip_TextId)
                trainer->Can_Train_Gossip_TextId = 1;
            if (!trainer->Cannot_Train_GossipTextId)
                trainer->Cannot_Train_GossipTextId = 1;

            std::string temp = fields[9].GetString();
            if (temp.length())
                trainer->UIMessage = temp;
            else
                trainer->UIMessage = normalTalkMessage;

            trainer->SpellCount = static_cast<uint32_t>(getTrainerSpellSetById(trainer->spellset_id)->size());

            // and now we insert it to our lookup table
            if (trainer->SpellCount == 0)
            {
                if (trainer->UIMessage != normalTalkMessage)
                    trainer->UIMessage.clear();

                continue;
            }

            m_trainers.insert(std::pair(entry, trainer));
        } while (trainerResult->NextRow());

        delete trainerResult;
        sLogger.info("ObjectMgr : %u trainers loaded.", static_cast<uint32_t>(m_trainers.size()));
    }
}

std::shared_ptr<Trainer> ObjectMgr::getTrainer(uint32_t _entry)
{
    const auto iter = m_trainers.find(_entry);
    if (iter == m_trainers.end())
        return nullptr;

    return iter->second;
}

void ObjectMgr::loadCreatureDisplayInfo()
{
    for (uint32_t i = 0; i < sCreatureDisplayInfoStore.GetNumRows(); ++i)
    {
        const auto* const displayInfoEntry = sCreatureDisplayInfoStore.LookupEntry(i);
        if (displayInfoEntry == nullptr)
            continue;

        CreatureDisplayInfoData data;
        data.id = displayInfoEntry->ID;
        data.modelId = displayInfoEntry->ModelID;
        data.extendedDisplayInfoId = displayInfoEntry->ExtendedDisplayInfoID;
        data.creatureModelScale = displayInfoEntry->CreatureModelScale;
        data.modelInfo = sCreatureModelDataStore.LookupEntry(data.modelId);
        if (data.modelInfo != nullptr)
        {
            if (strstr(data.modelInfo->ModelName, "InvisibleStalker"))
                data.isModelInvisibleStalker = true;
        }

        m_creatureDisplayInfoData.insert(std::make_pair(displayInfoEntry->ID, data));
    }
}

CreatureDisplayInfoData const* ObjectMgr::getCreatureDisplayInfoData(uint32_t _displayId) const
{
    const auto itr = m_creatureDisplayInfoData.find(_displayId);
    if (itr == m_creatureDisplayInfoData.cend())
        return nullptr;

    return &itr->second;
}

GameObject* ObjectMgr::createGameObjectByGuid(uint32_t _id, uint32_t _guid)
{
    GameObjectProperties const* gameobjectProperties = sMySQLStore.getGameObjectProperties(_id);
    if (gameobjectProperties == nullptr)
        return nullptr;

    GameObject* gameObject;

    const uint64_t createdGuid = uint64_t((uint64_t(HIGHGUID_TYPE_GAMEOBJECT) << 32) | _guid);

    switch (gameobjectProperties->type)
    {
        case GAMEOBJECT_TYPE_DOOR:
            gameObject = new GameObject_Door(createdGuid);
            break;
        case GAMEOBJECT_TYPE_BUTTON:
            gameObject = new GameObject_Button(createdGuid);
            break;
        case GAMEOBJECT_TYPE_QUESTGIVER:
            gameObject = new GameObject_QuestGiver(createdGuid);
            break;
        case GAMEOBJECT_TYPE_CHEST:
            gameObject = new GameObject_Chest(createdGuid);
            break;
        case GAMEOBJECT_TYPE_TRAP:
            gameObject = new GameObject_Trap(createdGuid);
            break;
        case GAMEOBJECT_TYPE_CHAIR:
            gameObject = new GameObject_Chair(createdGuid);
            break;
        case GAMEOBJECT_TYPE_SPELL_FOCUS:
            gameObject = new GameObject_SpellFocus(createdGuid);
            break;
        case GAMEOBJECT_TYPE_GOOBER:
            gameObject = new GameObject_Goober(createdGuid);
            break;
        case GAMEOBJECT_TYPE_TRANSPORT:
            gameObject = new GameObject_Transport(createdGuid);
            break;
        case GAMEOBJECT_TYPE_CAMERA:
            gameObject = new GameObject_Camera(createdGuid);
            break;
        case GAMEOBJECT_TYPE_FISHINGNODE:
            gameObject = new GameObject_FishingNode(createdGuid);
            break;
        case GAMEOBJECT_TYPE_RITUAL:
            gameObject = new GameObject_Ritual(createdGuid);
            break;
        case GAMEOBJECT_TYPE_SPELLCASTER:
            gameObject = new GameObject_SpellCaster(createdGuid);
            break;
        case GAMEOBJECT_TYPE_MEETINGSTONE:
            gameObject = new GameObject_Meetingstone(createdGuid);
            break;
        case GAMEOBJECT_TYPE_FLAGSTAND:
            gameObject = new GameObject_FlagStand(createdGuid);
            break;
        case GAMEOBJECT_TYPE_FISHINGHOLE:
            gameObject = new GameObject_FishingHole(createdGuid);
            break;
        case GAMEOBJECT_TYPE_FLAGDROP:
            gameObject = new GameObject_FlagDrop(createdGuid);
            break;
        case GAMEOBJECT_TYPE_BARBER_CHAIR:
            gameObject = new GameObject_BarberChair(createdGuid);
            break;
        case GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING:
            gameObject = new GameObject_Destructible(createdGuid);
            break;
        default:
            gameObject = new GameObject(createdGuid);
            break;
    }

    gameObject->SetGameObjectProperties(gameobjectProperties);

    return gameObject;
}

void ObjectMgr::loadInstanceEncounters()
{
    const auto startTime = Util::TimeNow();

    //                                                 0         1            2                3               4       5
    QueryResult* result = WorldDatabase.Query("SELECT entry, creditType, creditEntry, lastEncounterDungeon, comment, mapid FROM instance_encounters");
    if (result == nullptr)
    {
        sLogger.debug(">> Loaded 0 instance encounters, table is empty!");
        return;
    }

#if VERSION_STRING >= WotLK
    std::map<uint32_t, DBC::Structures::DungeonEncounterEntry const*> dungeonLastBosses;
#endif

    uint32_t count = 0;
    do
    {
        Field* fields = result->Fetch();
        auto entry = fields[0].GetUInt32();
        auto creditType = fields[1].GetUInt8();
        auto creditEntry = fields[2].GetUInt32();
        auto lastEncounterDungeon = fields[3].GetUInt16();
        auto dungeonEncounterName = fields[4].GetString();

#if VERSION_STRING <= TBC
        auto mapId = fields[5].GetUInt32();
#else
        const auto dungeonEncounter = sDungeonEncounterStore.LookupEntry(entry);
        if (dungeonEncounter == nullptr)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `instance_encounters` has an invalid encounter id %u, skipped!", entry);
            continue;
        }

#if VERSION_STRING == WotLK
        dungeonEncounterName = dungeonEncounter->encounterName[sWorld.getDbcLocaleLanguageId()];
#else
        dungeonEncounterName = dungeonEncounter->encounterName[0];
#endif
#endif

        if (lastEncounterDungeon && sLfgMgr.GetLFGDungeon(lastEncounterDungeon) == 0)
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `instance_encounters` has an encounter %u (%s) marked as final for invalid dungeon id %u, skipped!", entry, dungeonEncounterName, lastEncounterDungeon);
            continue;
        }

#if VERSION_STRING >= WotLK
        if (lastEncounterDungeon)
        {
            const auto itr = dungeonLastBosses.find(lastEncounterDungeon);
            if (itr != dungeonLastBosses.end())
            {
#if VERSION_STRING == WotLK
                const auto itrEncounterName = itr->second->encounterName[sWorld.getDbcLocaleLanguageId()];
#else
                const auto itrEncounterName = itr->second->encounterName;
#endif
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `instance_encounters` specified encounter %u (%s) as last encounter but %u (%s) is already marked as one, skipped!", entry, dungeonEncounterName, itr->second->id, itrEncounterName);
                continue;
            }

            dungeonLastBosses[lastEncounterDungeon] = dungeonEncounter;
        }
#endif

        switch (creditType)
        {
        case ENCOUNTER_CREDIT_KILL_CREATURE:
        {
            const auto creatureprop = sMySQLStore.getCreatureProperties(creditEntry);
            if (creatureprop == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `instance_encounters` has an invalid creature (entry %u) linked to the encounter %u (%s), skipped!", creditEntry, entry, dungeonEncounterName);
                continue;
            }
            const_cast<CreatureProperties*>(creatureprop)->extra_a9_flags |= 0x10000000; // Flagged Dungeon Boss
            break;
        }
        case ENCOUNTER_CREDIT_CAST_SPELL:
        {
            if (sSpellMgr.getSpellInfo(creditEntry) == nullptr)
            {
                sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `instance_encounters` has an invalid spell (entry %u) linked to the encounter %u (%s), skipped!", creditEntry, entry, dungeonEncounterName);
                continue;
            }
            break;
        }
        default:
        {
            sLogger.debugFlag(AscEmu::Logging::LF_DB_TABLES, "Table `instance_encounters` has an invalid credit type (%u) for encounter %u (%s), skipped!", creditType, entry, dungeonEncounterName);
            continue;
        }
        }

#if VERSION_STRING <= TBC
        DungeonEncounterList& encounters = m_dungeonEncounterStore[mapId];
        encounters.push_back(std::make_shared<DungeonEncounter>(EncounterCreditType(creditType), creditEntry));
#else
        DungeonEncounterList& encounters = m_dungeonEncounterStore[static_cast<int32_t>(static_cast<uint16_t>(dungeonEncounter->mapId) | (static_cast<uint32_t>(dungeonEncounter->difficulty) << 16))];
        encounters.push_back(std::make_shared<DungeonEncounter>(dungeonEncounter, EncounterCreditType(creditType), creditEntry, lastEncounterDungeon));
#endif
        ++count;
    } while (result->NextRow());

    sLogger.info("ObjectMgr : Loaded %u instance encounters in %u ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

DungeonEncounterList const* ObjectMgr::getDungeonEncounterList(uint32_t _mapId, uint8_t _difficulty)
{
#if VERSION_STRING >= WotLK
    std::unordered_map<uint32_t, DungeonEncounterList>::const_iterator itr = m_dungeonEncounterStore.find(uint32_t(uint16_t(_mapId) | (uint32_t(_difficulty) << 16)));
#else
    std::unordered_map<uint32_t, DungeonEncounterList>::const_iterator itr = m_dungeonEncounterStore.find(_mapId);
#endif
    if (itr != m_dungeonEncounterStore.end())
        return &itr->second;
    return nullptr;
}

void ObjectMgr::loadCreatureMovementOverrides()
{
    const auto startTime = Util::TimeNow();
    uint32_t count = 0;

    m_creatureMovementOverrides.clear();

    QueryResult* result = WorldDatabase.Query("SELECT SpawnId, Ground, Swim, Flight, Rooted, Chase, Random from creature_movement_override");
    if (!result)
    {
        sLogger.info("loadCreatureMovementOverrides : Loaded 0 creature movement overrides. DB table `creature_movement_override` is empty!");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        uint32_t spawnId = fields[0].GetUInt32();

        QueryResult* spawnResult = WorldDatabase.Query("SELECT * FROM creature_spawns WHERE id = %u", spawnId);
        if (spawnResult == nullptr)
        {
            sLogger.failure("Creature (SpawnId: %u) does not exist but has a record in `creature_movement_override`", spawnId);
            delete spawnResult;
            continue;
        }

        CreatureMovementData& movement = m_creatureMovementOverrides[spawnId];
        movement.Ground = static_cast<CreatureGroundMovementType>(fields[1].GetUInt8());
        movement.Swim = fields[2].GetBool();
        movement.Flight = static_cast<CreatureFlightMovementType>(fields[3].GetUInt8());
        movement.Rooted = fields[4].GetBool();
        movement.Chase = static_cast<CreatureChaseMovementType>(fields[5].GetUInt8());
        movement.Random = static_cast<CreatureRandomMovementType>(fields[6].GetUInt8());

        checkCreatureMovement(spawnId, movement);
        ++count;

    } while (result->NextRow());

    delete result;

    sLogger.info("ObjectMgr :  Loaded %u movement overrides in %u ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

void ObjectMgr::checkCreatureMovement(uint32_t /*id*/, CreatureMovementData& _creatureMovement)
{
    if (_creatureMovement.Ground >= CreatureGroundMovementType::Max)
        _creatureMovement.Ground = CreatureGroundMovementType::Run;

    if (_creatureMovement.Flight >= CreatureFlightMovementType::Max)
        _creatureMovement.Flight = CreatureFlightMovementType::None;

    if (_creatureMovement.Chase >= CreatureChaseMovementType::Max)
        _creatureMovement.Chase = CreatureChaseMovementType::Run;

    if (_creatureMovement.Random >= CreatureRandomMovementType::Max)
        _creatureMovement.Random = CreatureRandomMovementType::Walk;
}

CreatureMovementData const* ObjectMgr::getCreatureMovementOverride(uint32_t _spawnId) const
{
    const auto itr = m_creatureMovementOverrides.find(_spawnId);
    if (itr != m_creatureMovementOverrides.end())
        return &itr->second;
    return nullptr;
}

void ObjectMgr::loadWorldStateTemplates()
{
    QueryResult* result = WorldDatabase.QueryNA("SELECT DISTINCT map FROM worldstate_templates ORDER BY map;");
    if (result == nullptr)
        return;

    do
    {
        Field* field = result->Fetch();
        uint32_t mapId = field[0].GetUInt32();

        m_worldstateTemplates.insert(std::make_pair(mapId, std::make_shared<WorldStateMap>()));

    } while (result->NextRow());

    delete result;

    result = WorldDatabase.QueryNA("SELECT map, zone, field, value FROM worldstate_templates;");
    if (result == nullptr)
        return;

    do
    {
        Field* field = result->Fetch();
        WorldState worldState;

        uint32_t mapId = field[0].GetUInt32();
        uint32_t zone = field[1].GetUInt32();
        worldState.field = field[2].GetUInt32();
        worldState.value = field[3].GetUInt32();

        auto itr = m_worldstateTemplates.find(mapId);
        if (itr == m_worldstateTemplates.end())
            continue;

        itr->second->insert(std::make_pair(zone, worldState));

    } while (result->NextRow());

    delete result;
}

std::shared_ptr<WorldStateMap> ObjectMgr::getWorldStatesForMap(uint32_t _map) const
{
    const auto itr = m_worldstateTemplates.find(_map);
    if (itr == m_worldstateTemplates.end())
        return nullptr;
    return itr->second;
}

void ObjectMgr::loadCreatureTimedEmotes()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM creature_timed_emotes order by rowid asc");
    if (!result)
        return;

    uint32_t count = 0;
    do
    {
        Field* field = result->Fetch();
        auto timedEmotes = std::make_shared<SpawnTimedEmotes>();
        timedEmotes->type = field[2].GetUInt8();
        timedEmotes->value = field[3].GetUInt32();
        timedEmotes->msg = field[4].GetString();
        timedEmotes->msg_type = field[5].GetUInt8();
        timedEmotes->msg_lang = field[6].GetUInt8();
        timedEmotes->expire_after = field[7].GetUInt32();

        uint32_t spawnId = field[0].GetUInt32();

        auto timedEmotePair = m_timedEmotes.find(spawnId);
        if (timedEmotePair == m_timedEmotes.end())
        {
            std::shared_ptr<TimedEmoteList> emoteList = std::make_shared<TimedEmoteList>();
            emoteList->push_back(timedEmotes);
            m_timedEmotes[spawnId] = emoteList;
        }
        else
        {
            timedEmotePair->second->push_back(timedEmotes);
        }

        ++count;
    } while (result->NextRow());

    sLogger.info("ObjectMgr : %u timed emotes cached.", count);
    delete result;
}

std::shared_ptr<TimedEmoteList> ObjectMgr::getTimedEmoteList(uint32_t _spawnId)
{
    const auto timedEmotesPair = m_timedEmotes.find(_spawnId);
    if (timedEmotesPair != m_timedEmotes.end())
        return timedEmotesPair->second;

     return nullptr;
}

void ObjectMgr::generateLevelUpInfo()
{
    struct MissingLevelData
    {
        uint32_t _level;
        uint8_t _race;
        uint8_t _class;
    };

    std::vector<MissingLevelData> _missingHealthLevelData;
    std::vector<MissingLevelData> _missingStatLevelData;

    uint32_t levelstat_counter = 0;
    uint32_t class_levelstat_counter = 0;
    for (uint8_t playerClass = WARRIOR; playerClass < MAX_PLAYER_CLASSES; ++playerClass)
    {
        for (uint8_t playerRace = RACE_HUMAN; playerRace < DBC_NUM_RACES; ++playerRace)
        {
            if (!isClassRaceCombinationPossible(playerClass, playerRace))
            {
                if (sMySQLStore.getPlayerLevelstats(1, playerRace, playerClass))
                {
                    sLogger.info("ObjectMgr : Invalid class/race combination! %u class and %u race.", uint32_t(playerClass), uint32_t(playerRace));
                    sLogger.info("ObjectMgr : But class/race values for level 1 in db!");
                }
                continue;
            }

            auto levelMap = std::make_shared<LevelMap>();

            for (uint32_t level = 1; level <= worldConfig.player.playerLevelCap; ++level)
            {
                auto levelInfo = std::make_shared<LevelInfo>();

                if (auto* playerClassLevelstats = sMySQLStore.getPlayerClassLevelStats(level, playerClass))
                {
                    levelInfo->HP = playerClassLevelstats->health;
                    levelInfo->Mana = playerClassLevelstats->mana;
                    ++class_levelstat_counter;
                }
                else
                {
                    levelInfo->HP = 0;
                    levelInfo->Mana = 0;
                    _missingHealthLevelData.push_back({ level, playerRace, playerClass });
                }

                if (auto* playerLevelstats = sMySQLStore.getPlayerLevelstats(level, playerRace, playerClass))
                {
                    levelInfo->Stat[0] = playerLevelstats->strength;
                    levelInfo->Stat[1] = playerLevelstats->agility;
                    levelInfo->Stat[2] = playerLevelstats->stamina;
                    levelInfo->Stat[3] = playerLevelstats->intellect;
                    levelInfo->Stat[4] = playerLevelstats->spirit;
                    ++levelstat_counter;
                }
                else
                {
                    for (unsigned int& id : levelInfo->Stat)
                        id = 0;

                    _missingStatLevelData.push_back({ level, playerRace, playerClass });
                }

                levelMap->insert(LevelMap::value_type(level, levelInfo));
            }

            m_levelInfo.insert(LevelInfoMap::value_type(std::make_pair(playerRace, playerClass), levelMap));
        }
    }

    sLogger.info("ObjectMgr : %u levelstats and %u classlevelstats applied from db.", levelstat_counter, class_levelstat_counter);

    // generate missing data
    uint32_t hp_counter = 0;
    for (auto missingHP : _missingHealthLevelData)
    {
        uint32_t TotalHealthGain = 0;
        uint32_t TotalManaGain = 0;

        // use legacy gaining
        switch (missingHP._class)
        {
            case WARRIOR:
                if (missingHP._level < 13) TotalHealthGain += 19;
                else if (missingHP._level < 36) TotalHealthGain += missingHP._level + 6;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 206;
                else TotalHealthGain += 2 * missingHP._level - 30;
                break;
            case HUNTER:
                if (missingHP._level < 13) TotalHealthGain += 17;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 161;
                else TotalHealthGain += missingHP._level + 4;

                if (missingHP._level < 11) TotalManaGain += 29;
                else if (missingHP._level < 27) TotalManaGain += missingHP._level + 18;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 150;
                else TotalManaGain += 45;
                break;
            case ROGUE:
                if (missingHP._level < 15) TotalHealthGain += 17;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 191;
                else TotalHealthGain += missingHP._level + 2;
                break;
            case DRUID:
                if (missingHP._level < 17) TotalHealthGain += 17;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 176;
                else TotalHealthGain += missingHP._level;

                if (missingHP._level < 26) TotalManaGain += missingHP._level + 20;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 150;
                else TotalManaGain += 45;
                break;
            case MAGE:
                if (missingHP._level < 23) TotalHealthGain += 15;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 190;
                else TotalHealthGain += missingHP._level - 8;

                if (missingHP._level < 28) TotalManaGain += missingHP._level + 23;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 115;
                else TotalManaGain += 51;
                break;
            case SHAMAN:
                if (missingHP._level < 16) TotalHealthGain += 17;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 157;
                else TotalHealthGain += missingHP._level + 1;

                if (missingHP._level < 22) TotalManaGain += missingHP._level + 19;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 175;
                else TotalManaGain += 49;
                break;
            case WARLOCK:
                if (missingHP._level < 17) TotalHealthGain += 17;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 192;
                else TotalHealthGain += missingHP._level - 2;

                if (missingHP._level < 30) TotalManaGain += missingHP._level + 21;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 121;
                else TotalManaGain += 51;
                break;
            case PALADIN:
                if (missingHP._level < 14) TotalHealthGain += 18;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 167;
                else TotalHealthGain += missingHP._level + 4;

                if (missingHP._level < 30) TotalManaGain += missingHP._level + 17;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 131;
                else TotalManaGain += 42;
                break;
            case PRIEST:
                if (missingHP._level < 21) TotalHealthGain += 15;
                else if (missingHP._level > 60) TotalHealthGain += missingHP._level + 157;
                else TotalHealthGain += missingHP._level - 6;

                if (missingHP._level < 22) TotalManaGain += missingHP._level + 22;
                else if (missingHP._level < 32) TotalManaGain += missingHP._level + 37;
                else if (missingHP._level > 60) TotalManaGain += missingHP._level + 207;
                else TotalManaGain += 54;
                break;
            case DEATHKNIGHT:
                TotalHealthGain += 92;
                break;
            default:
                TotalHealthGain += 15;
                TotalManaGain += 45;
                break;
        }

        if (auto level_info = sObjectMgr.getLevelInfo(missingHP._race, missingHP._class, missingHP._level))
        {
            level_info->HP = level_info->HP + TotalHealthGain;
            level_info->Mana = level_info->Mana + TotalManaGain;
            ++hp_counter;
        }
    }

    uint32_t stat_counter = 0;
    for (auto missingStat : _missingStatLevelData)
    {
        if (auto level_info = sObjectMgr.getLevelInfo(missingStat._race, missingStat._class, missingStat._level))
        {
            for (uint8_t id = 0; id < 5; ++id)
            {
                uint32_t val = GainStat(static_cast<uint16_t>(missingStat._level), missingStat._class, id);
                level_info->Stat[id] = level_info->Stat[id] + val;
            }

            ++stat_counter;
        }
    }

    sLogger.info("ObjectMgr : %u level up information generated.", (stat_counter + hp_counter));
}

std::shared_ptr<LevelInfo> ObjectMgr::getLevelInfo(uint32_t _race, uint32_t _class, uint32_t _level)
{
    for (const auto& levelInfoPair : m_levelInfo)
    {
        if (levelInfoPair.first.first == _race && levelInfoPair.first.second == _class)
        {
            if (_level > worldConfig.player.playerLevelCap)
                _level = worldConfig.player.playerLevelCap;

            const auto levelInfoMap = levelInfoPair.second->find(_level);
            if (levelInfoMap == levelInfoPair.second->end())
            {
                sLogger.info("GetLevelInfo : No level information found for level %u!", _level);
                return nullptr;
            }

            return levelInfoMap->second;
        }
    }

    return nullptr;
}

std::shared_ptr<Pet> ObjectMgr::createPet(uint32_t _entry)
{
    const uint32_t guid = ++m_hiPetGuid;
    return std::make_shared<Pet>(WoWGuid(guid, _entry, HIGHGUID_TYPE_PET));
}

void ObjectMgr::loadPetSpellCooldowns()
{
    for (uint32_t i = 0; i < sCreatureSpellDataStore.GetNumRows(); ++i)
    {
        const auto cretureSpellData = sCreatureSpellDataStore.LookupEntry(i);

        for (uint8_t j = 0; j < 3; ++j)
        {
            if (cretureSpellData == nullptr)
                continue;

            uint32_t spellId = cretureSpellData->Spells[j];
            uint32_t cooldown = cretureSpellData->Cooldowns[j] * 10;

            if (spellId != 0)
            {
                auto petCooldownPair = m_petSpellCooldowns.find(spellId);
                if (petCooldownPair == m_petSpellCooldowns.end())
                {
                    if (cooldown)
                        m_petSpellCooldowns.insert(std::make_pair(spellId, cooldown));
                }
            }
        }
    }
}

uint32_t ObjectMgr::getPetSpellCooldown(uint32_t _spellId)
{
    const auto petCooldownPair = m_petSpellCooldowns.find(_spellId);
    if (petCooldownPair != m_petSpellCooldowns.end())
        return petCooldownPair->second;

    if (const auto spellInfo = sSpellMgr.getSpellInfo(_spellId))
    {
        if (spellInfo->getRecoveryTime() > spellInfo->getCategoryRecoveryTime())
            return spellInfo->getRecoveryTime();
        return spellInfo->getCategoryRecoveryTime();
    }

    return 0;
}

Item* ObjectMgr::loadItem(uint32_t _lowGuid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM playeritems WHERE guid = %u", _lowGuid);
    Item* item = nullptr;
    if (result)
    {
        if (const auto itemProperties = sMySQLStore.getItemProperties(result->Fetch()[2].GetUInt32()))
        {
            if (itemProperties->InventoryType == INVTYPE_BAG)
            {
                Container* container = new Container(HIGHGUID_TYPE_CONTAINER, _lowGuid);
                container->loadFromDB(result->Fetch());
                item = container;
            }
            else
            {
                item = new Item;
                item->init(HIGHGUID_TYPE_ITEM, _lowGuid);
                item->loadFromDB(result->Fetch(), nullptr, false);
            }
        }

        delete result;
    }

    return item;
}

Item* ObjectMgr::createItem(uint32_t _entry, Player* _playerOwner)
{
    ItemProperties const* itemProperties = sMySQLStore.getItemProperties(_entry);
    if (itemProperties == nullptr)
        return nullptr;

    if (itemProperties->InventoryType == INVTYPE_BAG)
    {
        Container* container = new Container(HIGHGUID_TYPE_CONTAINER, generateLowGuid(HIGHGUID_TYPE_CONTAINER));
        container->create(_entry, _playerOwner);
        container->setStackCount(1);
        return container;
    }

    Item* item = new Item;
    item->init(HIGHGUID_TYPE_ITEM, generateLowGuid(HIGHGUID_TYPE_ITEM));
    item->create(_entry, _playerOwner);
    item->setStackCount(1);

#if VERSION_STRING > TBC
    if (_playerOwner != nullptr)
    {
        const uint32_t* playedTime = _playerOwner->getPlayedTime();
        item->setCreatePlayedTime(playedTime[1]);
    }
#endif

    return item;
}

void ObjectMgr::setHighestGuids()
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(guid) FROM characters");
    if (result)
    {
        m_hiPlayerGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM playeritems");
    if (result)
    {
        m_hiItemGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM corpses");
    if (result)
    {
        m_hiCorpseGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = sMySQLStore.getWorldDBQuery("SELECT MAX(id) FROM creature_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0", VERSION_STRING, VERSION_STRING);
    if (result)
    {
        do
        {
            m_hiCreatureSpawnId = result->Fetch()[0].GetUInt32();
        } while (result->NextRow());

        delete result;
    }

    result = sMySQLStore.getWorldDBQuery("SELECT MAX(id) FROM gameobject_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0", VERSION_STRING, VERSION_STRING);
    if (result)
    {
        do
        {
            m_hiGameObjectSpawnId = result->Fetch()[0].GetUInt32();
        } while (result->NextRow());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(group_id) FROM `groups`");
    if (result)
    {
        m_hiGroupId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(charterid) FROM charters");
    if (result)
    {
        m_hiCharterId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guildid) FROM guilds");
    if (result)
    {
        m_hiGuildId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(UID) FROM playerbugreports");
    if (result != nullptr)
    {
        m_reportId = result->Fetch()[0].GetUInt32() + 1;
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(message_id) FROM mailbox");
    if (result)
    {
        m_mailId = result->Fetch()[0].GetUInt32() + 1;
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(setGUID) FROM equipmentsets");
    if (result != nullptr)
    {
        m_setGuid = result->Fetch()[0].GetUInt32() + 1;
        delete result;
    }

#if VERSION_STRING > WotLK
    result = CharacterDatabase.Query("SELECT MAX(itemId) FROM character_void_storage");
    if (result  != nullptr)
    {
        m_voidItemId = uint64_t(result->Fetch()[0].GetUInt32() + 1);
    }
#endif

    sLogger.info("ObjectMgr : HighGuid(CORPSE) = %lu", m_hiCorpseGuid.load());
    sLogger.info("ObjectMgr : HighGuid(PLAYER) = %lu", m_hiPlayerGuid.load());
    sLogger.info("ObjectMgr : HighGuid(GAMEOBJ) = %lu", m_hiGameObjectSpawnId.load());
    sLogger.info("ObjectMgr : HighGuid(UNIT) = %lu", m_hiCreatureSpawnId.load());
    sLogger.info("ObjectMgr : HighGuid(ITEM) = %lu", m_hiItemGuid.load());
    sLogger.info("ObjectMgr : HighGuid(CONTAINER) = %lu", m_hiItemGuid.load());
    sLogger.info("ObjectMgr : HighGuid(GROUP) = %lu", m_hiGroupId.load());
    sLogger.info("ObjectMgr : HighGuid(CHARTER) = %lu", m_hiCharterId.load());
    sLogger.info("ObjectMgr : HighGuid(GUILD) = %lu", m_hiGuildId.load());
    sLogger.info("ObjectMgr : HighGuid(BUGREPORT) = %lu", m_reportId.load() - 1);
    sLogger.info("ObjectMgr : HighGuid(MAIL) = %lu", m_mailId.load());
    sLogger.info("ObjectMgr : HighGuid(EQUIPMENTSET) = %lu", m_setGuid.load() - 1);
}

uint32_t ObjectMgr::generateReportId() { return ++m_reportId; }
uint32_t ObjectMgr::generateEquipmentSetId() { return ++m_setGuid; }
uint32_t ObjectMgr::generateMailId() { return ++m_mailId; }
uint32_t ObjectMgr::generateLowGuid(uint32_t _guidHigh)
{
    switch (_guidHigh)
    {
        case HIGHGUID_TYPE_PLAYER:
            return ++m_hiPlayerGuid;
        case HIGHGUID_TYPE_ITEM:
        case HIGHGUID_TYPE_CONTAINER:
            return ++m_hiItemGuid;
        default:
            sLogger.failure("ObjectMgr::GenerateLowGuid tried to generate low guid gor non player/item, return 0!");
            return 0;
    }
}

uint32_t ObjectMgr::generateArenaTeamId() { return ++m_hiArenaTeamId; }
uint32_t ObjectMgr::generateGuildId() { return ++m_hiGuildId; }
uint32_t ObjectMgr::generateCreatureSpawnId() { return ++m_hiCreatureSpawnId; }
uint32_t ObjectMgr::generateGameObjectSpawnId() { return ++m_hiGameObjectSpawnId; }
#if VERSION_STRING > WotLK
uint64_t ObjectMgr::generateVoidStorageItemId() { return ++m_voidItemId; }
#endif
