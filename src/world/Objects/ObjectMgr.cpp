/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
 * Copyright (C) 2005-2007 Ascent Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "StdAfx.h"
#include "Management/QuestLogEntry.hpp"
#include "Management/Container.h"
#include "Exceptions/Exceptions.hpp"
#include "Units/Stats.h"
#include "Management/ArenaTeam.h"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Units/Players/PlayerClasses.hpp"
#include "Server/MainServerDefines.h"
#include "Config/Config.h"
#include "Map/MapMgr.h"
#include "Map/MapScriptInterface.h"
#include "Map/WorldCreatorDefines.hpp"
#include "Spell/SpellMgr.h"
#include "Units/Creatures/Pet.h"
#include "Spell/Definitions/SpellEffects.h"
#include "Management/Guild/GuildMgr.hpp"
#include "Management/TaxiMgr.h"
#include "Management/LFG/LFGMgr.hpp"
#if VERSION_STRING < Cata
#include "Management/Guild/Guild.hpp"
#endif

const char* NormalTalkMessage = "DMSG";

ObjectMgr& ObjectMgr::getInstance()
{
    static ObjectMgr mInstance;
    return mInstance;
}

void ObjectMgr::initialize()
{
    m_hiItemGuid = 0;
    m_hiGroupId = 0;
    m_mailid = 0;
    m_reportID = 0;
    m_setGUID = 0;
    m_hiCorpseGuid = 0;
    m_hiGuildId = 0;
    m_hiPetGuid = 0;
    m_hiArenaTeamId = 0;
    m_hiPlayerGuid = 1;
}

void ObjectMgr::finalize()
{
    LogNotice("ObjectMgr : Deleting Corpses...");
    CorpseCollectorUnload();

    LogNotice("ObjectMgr : Deleting Vendors...");
    for (VendorMap::iterator i = mVendors.begin(); i != mVendors.end(); ++i)
    {
        delete i->second;
    }

    LogNotice("ObjectMgr : Deleting Trainers...");
    for (TrainerMap::iterator i = mTrainers.begin(); i != mTrainers.end(); ++i)
    {
        Trainer* t = i->second;
        if (t->UIMessage && t->UIMessage != (char*)NormalTalkMessage)
            delete[] t->UIMessage;
        delete t;
    }

    LogNotice("ObjectMgr : Deleting Level Information...");
    for (LevelInfoMap::iterator i = mLevelInfo.begin(); i != mLevelInfo.end(); ++i)
    {
        LevelMap* l = i->second;
        for (LevelMap::iterator i2 = l->begin(); i2 != l->end(); ++i2)
        {
            delete i2->second;
        }

        l->clear();
        delete l;
    }

    LogNotice("ObjectMgr : Deleting Waypoint Cache...");
    for (std::unordered_map<uint32, Movement::WayPointMap*>::iterator i = mWayPointMap.begin(); i != mWayPointMap.end(); ++i)
    {
        for (Movement::WayPointMap::iterator i2 = i->second->begin(); i2 != i->second->end(); ++i2)
        {
            if ((*i2))
            {
                delete(*i2);
            }
        }

        delete i->second;
    }

    LogNotice("ObjectMgr : Deleting timed emote Cache...");
    for (std::unordered_map<uint32, TimedEmoteList*>::iterator i = m_timedemotes.begin(); i != m_timedemotes.end(); ++i)
    {
        for (TimedEmoteList::iterator i2 = i->second->begin(); i2 != i->second->end(); ++i2)
            if ((*i2))
            {
                delete[](*i2)->msg;
                delete(*i2);
            }

        delete i->second;
    }

    LogNotice("ObjectMgr : Deleting Charters...");
    for (uint8 i = 0; i < NUM_CHARTER_TYPES; ++i)
    {
        for (std::unordered_map<uint32, Charter*>::iterator itr = m_charters[i].begin(); itr != m_charters[i].end(); ++itr)
        {
            delete itr->second;
        }
    }

    LogNotice("ObjectMgr : Deleting Reputation Tables...");
    for (ReputationModMap::iterator itr = this->m_reputation_creature.begin(); itr != m_reputation_creature.end(); ++itr)
    {
        ReputationModifier* mod = itr->second;
        mod->mods.clear();
        delete mod;
    }
    for (ReputationModMap::iterator itr = this->m_reputation_faction.begin(); itr != m_reputation_faction.end(); ++itr)
    {
        ReputationModifier* mod = itr->second;
        mod->mods.clear();
        delete mod;
    }

    for (std::unordered_map<uint32, InstanceReputationModifier*>::iterator itr = this->m_reputation_instance.begin(); itr != this->m_reputation_instance.end(); ++itr)
    {
        InstanceReputationModifier* mod = itr->second;
        mod->mods.clear();
        delete mod;
    }

    LogNotice("ObjectMgr : Deleting Groups...");
    for (GroupMap::iterator itr = m_groups.begin(); itr != m_groups.end();)
    {
        Group* pGroup = itr->second;
        ++itr;
        if (pGroup != nullptr)
        {
            for (uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i)
            {
                SubGroup* pSubGroup = pGroup->GetSubGroup(i);
                if (pSubGroup != nullptr)
                {
                    pSubGroup->Disband();
                }
            }
            delete pGroup;
        }
    }

    LogNotice("ObjectMgr : Deleting Player Information...");
    for (std::unordered_map<uint32, PlayerInfo*>::iterator itr = m_playersinfo.begin(); itr != m_playersinfo.end(); ++itr)
    {
        itr->second->m_Group = nullptr;
        free(itr->second->name);
        delete itr->second;
    }

    LogNotice("ObjectMgr : Deleting Boss Information...");
    for (DungeonEncounterContainer::iterator itr = _dungeonEncounterStore.begin(); itr != _dungeonEncounterStore.end(); ++itr)
        for (DungeonEncounterList::iterator encounterItr = itr->second.begin(); encounterItr != itr->second.end(); ++encounterItr)
            delete *encounterItr;

    LogNotice("ObjectMgr : Deleting Arena Teams...");
    for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeams.begin(); itr != m_arenaTeams.end(); ++itr)
    {
        delete(*itr).second;
    }

    LogNotice("ObjectMgr : Cleaning up spell target constraints...");
    for (SpellTargetConstraintMap::iterator itr = m_spelltargetconstraints.begin(); itr != m_spelltargetconstraints.end(); ++itr)
        delete itr->second;

    m_spelltargetconstraints.clear();

    LogNotice("ObjectMgr : Cleaning up vehicle accessories...");
    for (std::map< uint32, std::vector< VehicleAccessoryEntry* >* >::iterator itr = vehicle_accessories.begin(); itr != vehicle_accessories.end(); ++itr)
    {
        std::vector< VehicleAccessoryEntry* > *v = itr->second;

        for (std::vector< VehicleAccessoryEntry* >::iterator itr2 = v->begin(); itr2 != v->end(); ++itr2)
            delete *itr2;
        v->clear();

        delete v;
    }

    vehicle_accessories.clear();


    LogNotice("ObjectMgr : Cleaning up worldstate templates...");
    for (std::map< uint32, std::multimap< uint32, WorldState >* >::iterator itr = worldstate_templates.begin(); itr != worldstate_templates.end(); ++itr)
    {
        itr->second->clear();
        delete itr->second;
    }

    worldstate_templates.clear();


    LogNotice("ObjectMgr : Clearing up event scripts...");
    mEventScriptMaps.clear();
    mSpellEffectMaps.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Groups
Group* ObjectMgr::GetGroupByLeader(Player* pPlayer)
{
    std::lock_guard<std::mutex> guard(m_groupLock);

    for (GroupMap::iterator itr = m_groups.begin(); itr != m_groups.end(); ++itr)
    {
        if (itr->second->GetLeader() == pPlayer->getPlayerInfo())
        {
            return itr->second;
        }
    }

    return nullptr;
}

Group* ObjectMgr::GetGroupById(uint32 id)
{
    std::lock_guard<std::mutex> guard(m_groupLock);

    GroupMap::iterator itr = m_groups.find(id);
    if (itr != m_groups.end())
        return itr->second;

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Player names
void ObjectMgr::DeletePlayerInfo(uint32 guid)
{
    std::lock_guard<std::mutex> guard(playernamelock);

    std::unordered_map<uint32, PlayerInfo*>::iterator i = m_playersinfo.find(guid);
    if (i == m_playersinfo.end())
        return;

    PlayerInfo* pl = i->second;
    if (pl->m_Group)
        pl->m_Group->RemovePlayer(pl);

    std::string pnam = std::string(pl->name);
    Util::StringToLowerCase(pnam);
    PlayerNameStringIndexMap::iterator i2 = m_playersInfoByName.find(pnam);
    if (i2 != m_playersInfoByName.end() && i2->second == pl)
    {
        m_playersInfoByName.erase(i2);
    }

    free(pl->name);
    delete i->second;
    m_playersinfo.erase(i);
}

PlayerInfo* ObjectMgr::GetPlayerInfo(uint32 guid)
{
    std::lock_guard<std::mutex> guard(playernamelock);

    std::unordered_map<uint32, PlayerInfo*>::iterator i = m_playersinfo.find(guid);
    PlayerInfo* rv = nullptr;
    if (i != m_playersinfo.end())
        return i->second;

    return nullptr;
}

void ObjectMgr::AddPlayerInfo(PlayerInfo* pn)
{
    std::lock_guard<std::mutex> guard(playernamelock);

    m_playersinfo[pn->guid] = pn;

    std::string pnam = std::string(pn->name);
    Util::StringToLowerCase(pnam);
    m_playersInfoByName[pnam] = pn;
}

void ObjectMgr::RenamePlayerInfo(PlayerInfo* pn, const char* oldname, const char* newname)
{
    std::lock_guard<std::mutex> guard(playernamelock);

    std::string oldn = std::string(oldname);
    Util::StringToLowerCase(oldn);

    PlayerNameStringIndexMap::iterator itr = m_playersInfoByName.find(oldn);
    if (itr != m_playersInfoByName.end() && itr->second == pn)
    {
        std::string newn = std::string(newname);
        Util::StringToLowerCase(newn);
        m_playersInfoByName.erase(itr);
        m_playersInfoByName[newn] = pn;
    }
}

void ObjectMgr::LoadSpellSkills()
{
    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); i++)
    {
        auto skill_line_ability = sSkillLineAbilityStore.LookupEntry(i);
        if (skill_line_ability)
        {
            mSpellSkills[skill_line_ability->spell] = skill_line_ability;
        }
    }
    LogDetail("ObjectMgr : %u spell skills loaded.", static_cast<uint32_t>(mSpellSkills.size()));
}

DBC::Structures::SkillLineAbilityEntry const* ObjectMgr::GetSpellSkill(uint32 id)
{
    return mSpellSkills[id];
}

SpellInfo const* ObjectMgr::GetNextSpellRank(SpellInfo const* sp, uint32 level)
{
    // Looks for next spell rank
    if (sp == nullptr)
    {
        return nullptr;
    }

    auto skill_line_ability = GetSpellSkill(sp->getId());
    if (skill_line_ability != nullptr && skill_line_ability->next > 0)
    {
        SpellInfo const* sp1 = sSpellMgr.getSpellInfo(skill_line_ability->next);
        if (sp1 && sp1->getBaseLevel() <= level)   // check level
        {
            return GetNextSpellRank(sp1, level);   // recursive for higher ranks
        }
    }
    return sp;
}

void ObjectMgr::LoadPlayersInfo()
{
    QueryResult* result = CharacterDatabase.Query("SELECT guid, name, race, class, level, gender, zoneid, timestamp, acct FROM characters");
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            PlayerInfo* pn = new PlayerInfo;
            pn->guid = fields[0].GetUInt32();
            pn->name = strdup(fields[1].GetString());
            pn->race = fields[2].GetUInt8();
            pn->cl = fields[3].GetUInt8();
            pn->lastLevel = fields[4].GetUInt32();
            pn->gender = fields[5].GetUInt8();
            pn->lastZone = fields[6].GetUInt32();
            pn->lastOnline = fields[7].GetUInt32();
            pn->acct = fields[8].GetUInt32();
            pn->m_Group = nullptr;
            pn->subGroup = 0;
            pn->m_loggedInPlayer = nullptr;
            pn->m_guild = 0;
            pn->guildRank = GUILD_RANK_NONE;

            // Raid & heroic Instance IDs
            // Must be done before entering world...
            QueryResult* result2 = CharacterDatabase.Query("SELECT instanceid, mode, mapid FROM instanceids WHERE playerguid = %u", pn->guid);
            if (result2)
            {
                PlayerInstanceMap::iterator itr;
                do
                {
                    uint32 instanceId = result2->Fetch()[0].GetUInt32();
                    uint32 mode = result2->Fetch()[1].GetUInt32();
                    uint32 mapId = result2->Fetch()[2].GetUInt32();
                    if (mode >= NUM_INSTANCE_MODES || mapId >= MAX_NUM_MAPS)
                    {
                        continue;
                    }

                    pn->savedInstanceIdsLock.Acquire();
                    itr = pn->savedInstanceIds[mode].find(mapId);
                    if (itr == pn->savedInstanceIds[mode].end())
                    {
                        pn->savedInstanceIds[mode].insert(PlayerInstanceMap::value_type(mapId, instanceId));
                    }
                    else
                    {
                        (*itr).second = instanceId;
                    }

                    ///\todo Instances not loaded yet ~.~
                    //if (!sInstanceMgr.InstanceExists(mapId, pn->m_savedInstanceIds[mapId][mode]))
                    //{
                    //    pn->m_savedInstanceIds[mapId][mode] = 0;
                    //    CharacterDatabase.Execute("DELETE FROM instanceids WHERE mapId = %u AND instanceId = %u AND mode = %u", mapId, instanceId, mode);
                    //}

                    pn->savedInstanceIdsLock.Release();
                } while (result2->NextRow());
                delete result2;
            }

            pn->team = getSideByRace(pn->race);

            if (GetPlayerInfoByName(pn->name) != nullptr)
            {
                // gotta rename him
                char temp[300];
                snprintf(temp, 300, "%s__%X__", pn->name, pn->guid);
                LogNotice("ObjectMgr : Renaming duplicate player %s to %s. (%u)", pn->name, temp, pn->guid);
                CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s', login_flags = %u WHERE guid = %u",
                                              CharacterDatabase.EscapeString(std::string(temp)).c_str(), (uint32)LOGIN_FORCED_RENAME, pn->guid);


                free(pn->name);
                pn->name = strdup(temp);
            }

            std::string lpn = std::string(pn->name);
            Util::StringToLowerCase(lpn);
            m_playersInfoByName[lpn] = pn;

            //this is startup -> no need in lock -> don't use addplayerinfo
            m_playersinfo[pn->guid] = pn;

        }
        while (result->NextRow());
        delete result;
    }
    LogDetail("ObjectMgr : %u players loaded.", static_cast<uint32_t>(m_playersinfo.size()));
}

PlayerInfo* ObjectMgr::GetPlayerInfoByName(const char* name)
{
    std::string lpn = std::string(name);
    Util::StringToLowerCase(lpn);

    std::lock_guard<std::mutex> guard(playernamelock);

    PlayerNameStringIndexMap::iterator i = m_playersInfoByName.find(lpn);
    if (i != m_playersInfoByName.end())
        return i->second;

    return nullptr;
}

#if VERSION_STRING > TBC
void ObjectMgr::LoadCompletedAchievements()
{
    QueryResult* result = CharacterDatabase.Query("SELECT achievement FROM character_achievement GROUP BY achievement");

    if (!result)
    {
        LOG_ERROR("Query failed: SELECT achievement FROM character_achievement");
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        allCompletedAchievements.insert(fields[0].GetUInt32());
    }
    while (result->NextRow());
    delete result;
}
#endif

Corpse* ObjectMgr::LoadCorpse(uint32 guid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM corpses WHERE guid =%u ", guid);

    if (result == nullptr)
        return nullptr;

    Field* fields = result->Fetch();
    Corpse* pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE, fields[0].GetUInt32());
    pCorpse->SetPosition(fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
    pCorpse->SetZoneId(fields[5].GetUInt32());
    pCorpse->SetMapId(fields[6].GetUInt32());
    pCorpse->setCorpseDataFromDbString(fields[7].GetString());
    if (pCorpse->getDisplayId() == 0)
    {
        delete pCorpse;
        return nullptr;
    }

    pCorpse->SetLoadedFromDB(true);
    pCorpse->SetInstanceID(fields[8].GetUInt32());
    pCorpse->AddToWorld();

    delete result;

    return pCorpse;
}


//////////////////////////////////////////////////////////////////////////////////////////
/// Live corpse retreival.
/// comments: I use the same tricky method to start from the last corpse instead of the first
//////////////////////////////////////////////////////////////////////////////////////////
Corpse* ObjectMgr::GetCorpseByOwner(uint32 ownerguid)
{
    Corpse* rv = nullptr;
    _corpseslock.Acquire();
    for (CorpseMap::const_iterator itr = m_corpses.begin(); itr != m_corpses.end(); ++itr)
    {
        WoWGuid wowGuid;
        wowGuid.Init(itr->second->getOwnerGuid());

        if (wowGuid.getGuidLowPart() == ownerguid)
        {
            rv = itr->second;
            break;
        }
    }
    _corpseslock.Release();

    return rv;
}

void ObjectMgr::DelinkPlayerCorpses(Player* pOwner)
{
    //dupe protection agaisnt crashs
    Corpse* c = this->GetCorpseByOwner(pOwner->getGuidLow());
    if (!c)
        return;
    sEventMgr.AddEvent(c, &Corpse::Delink, EVENT_CORPSE_SPAWN_BONES, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    CorpseAddEventDespawn(c);
}

#if VERSION_STRING > TBC
void ObjectMgr::LoadAchievementRewards()
{
    AchievementRewards.clear();                           // need for reload case

    QueryResult* result = WorldDatabase.Query("SELECT entry, gender, title_A, title_H, item, sender, subject, text FROM achievement_reward");

    if (!result)
    {
        LOG_DETAIL("Loaded 0 achievement rewards. DB table `achievement_reward` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();

        if (!sAchievementStore.LookupEntry(entry))
        {
            LogDebugFlag(LF_DB_TABLES, "ObjectMgr : Achievement reward entry %u has wrong achievement, ignore", entry);
            continue;
        }

        AchievementReward reward;
        reward.gender = fields[1].GetUInt32();
        reward.titel_A = fields[2].GetUInt32();
        reward.titel_H = fields[3].GetUInt32();
        reward.itemId = fields[4].GetUInt32();
        reward.sender = fields[5].GetUInt32();
        reward.subject = fields[6].GetString() ? fields[6].GetString() : "";
        reward.text = fields[7].GetString() ? fields[7].GetString() : "";

        if (reward.gender > 2)
            LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement reward %u has wrong gender %u.", entry, reward.gender);

        bool dup = false;
        AchievementRewardsMapBounds bounds = AchievementRewards.equal_range(entry);
        for (AchievementRewardsMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
        {
            if (iter->second.gender == 2 || reward.gender == 2)
            {
                dup = true;
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : Achievement reward %u must have single GENDER_NONE (%u), ignore duplicate case", 2, entry);
                break;
            }
        }

        if (dup)
            continue;

        // must be title or mail at least
        if (!reward.titel_A && !reward.titel_H && !reward.sender)
        {
            LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have title or item reward data, ignore.", entry);
            continue;
        }

        if (reward.titel_A)
        {
            auto const* char_title_entry = sCharTitlesStore.LookupEntry(reward.titel_A);
            if (!char_title_entry)
            {
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid title id (%u) in `title_A`, set to 0", entry, reward.titel_A);
                reward.titel_A = 0;
            }
        }

        if (reward.titel_H)
        {
            auto const* char_title_entry = sCharTitlesStore.LookupEntry(reward.titel_H);
            if (!char_title_entry)
            {
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid title id (%u) in `title_A`, set to 0", entry, reward.titel_H);
                reward.titel_H = 0;
            }
        }

        //check mail data before item for report including wrong item case
        if (reward.sender)
        {
            if (!sMySQLStore.getCreatureProperties(reward.sender))
            {
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid creature entry %u as sender, mail reward skipped.", entry, reward.sender);
                reward.sender = 0;
            }
        }
        else
        {
            if (reward.itemId)
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have sender data but have item reward, item will not rewarded", entry);

            if (!reward.subject.empty())
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have sender data but have mail subject.", entry);

            if (!reward.text.empty())
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u not have sender data but have mail text.", entry);
        }

        if (reward.itemId)
        {
            if (reward.itemId == 0)
            {
                LogDebugFlag(LF_DB_TABLES, "ObjectMgr : achievement_reward %u has invalid item id %u, reward mail will be without item.", entry, reward.itemId);
            }
        }

        AchievementRewards.insert(AchievementRewardsMap::value_type(entry, reward));
        ++count;

    }
    while (result->NextRow());

    delete result;

    LogDetail("ObjectMgr : Loaded %u achievement rewards", count);
}
#endif

void ObjectMgr::SetHighestGuids()
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
        m_hiItemGuid = (uint32)result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM corpses");
    if (result)
    {
        m_hiCorpseGuid = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(id) FROM creature_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0;", VERSION_STRING, VERSION_STRING);
    if (result)
    {
        m_hiCreatureSpawnId = result->Fetch()[0].GetUInt32();
        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(id) FROM gameobject_spawns WHERE min_build <= %u AND max_build >= %u AND event_entry = 0;", VERSION_STRING, VERSION_STRING);
    if (result)
    {
        m_hiGameObjectSpawnId = result->Fetch()[0].GetUInt32();
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
        m_reportID = uint32(result->Fetch()[0].GetUInt64() + 1);
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(message_id) FROM mailbox");
    if (result)
    {
        m_mailid = uint32(result->Fetch()[0].GetUInt64() + 1);
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(setGUID) FROM equipmentsets");
    if (result != nullptr)
    {
        m_setGUID = uint32(result->Fetch()[0].GetUInt32() + 1);
        delete result;
    }



    LogNotice("ObjectMgr : HighGuid(CORPSE) = %lu", m_hiCorpseGuid.load());
    LogNotice("ObjectMgr : HighGuid(PLAYER) = %lu", m_hiPlayerGuid.load());
    LogNotice("ObjectMgr : HighGuid(GAMEOBJ) = %lu", m_hiGameObjectSpawnId.load());
    LogNotice("ObjectMgr : HighGuid(UNIT) = %lu", m_hiCreatureSpawnId.load());
    LogNotice("ObjectMgr : HighGuid(ITEM) = %lu", m_hiItemGuid.load());
    LogNotice("ObjectMgr : HighGuid(CONTAINER) = %lu", m_hiItemGuid.load());
    LogNotice("ObjectMgr : HighGuid(GROUP) = %lu", m_hiGroupId.load());
    LogNotice("ObjectMgr : HighGuid(CHARTER) = %lu", m_hiCharterId.load());
    LogNotice("ObjectMgr : HighGuid(GUILD) = %lu", m_hiGuildId.load());
    LogNotice("ObjectMgr : HighGuid(BUGREPORT) = %u", uint32(m_reportID.load() - 1));
    LogNotice("ObjectMgr : HighGuid(MAIL) = %u", uint32(m_mailid.load()));
    LogNotice("ObjectMgr : HighGuid(EQUIPMENTSET) = %u", uint32(m_setGUID.load() - 1));
}

uint32 ObjectMgr::GenerateReportID()
{
    return ++m_reportID;
}

uint32 ObjectMgr::GenerateEquipmentSetID()
{
    return ++m_setGUID;
}

uint32 ObjectMgr::GenerateMailID()
{
    return ++m_mailid;
}

uint32 ObjectMgr::GenerateLowGuid(uint32 guidhigh)
{
    ARCEMU_ASSERT(guidhigh == HIGHGUID_TYPE_ITEM || guidhigh == HIGHGUID_TYPE_CONTAINER || guidhigh == HIGHGUID_TYPE_PLAYER);

    uint32 ret;
    if (guidhigh == HIGHGUID_TYPE_ITEM)
    {
        ret = ++m_hiItemGuid;
    }
    else if (guidhigh == HIGHGUID_TYPE_PLAYER)
    {
        ret = ++m_hiPlayerGuid;
    }
    else
    {
        ret = ++m_hiItemGuid;
    }
    return ret;
}

Player* ObjectMgr::GetPlayer(const char* name, bool caseSensitive)
{
    std::lock_guard<std::mutex> guard(_playerslock);

    if (!caseSensitive)
    {
        std::string strName = name;
        Util::StringToLowerCase(strName);
        for (PlayerStorageMap::const_iterator itr = _players.begin(); itr != _players.end(); ++itr)
        {
            if (!stricmp(itr->second->getName().c_str(), strName.c_str()))
            {
                return itr->second;
            }
        }
    }
    else
    {
        for (PlayerStorageMap::const_iterator itr = _players.begin(); itr != _players.end(); ++itr)
        {
            if (!strcmp(itr->second->getName().c_str(), name))
            {
                return itr->second;
            }
        }
    }

    return nullptr;
}

Player* ObjectMgr::GetPlayer(uint32 guid)
{
    std::lock_guard<std::mutex> guard(_playerslock);

    PlayerStorageMap::const_iterator itr = _players.find(guid);
    return (itr != _players.end()) ? itr->second : nullptr;
}

void ObjectMgr::LoadVendors()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM vendors");
    if (result != nullptr)
    {
        std::unordered_map<uint32, std::vector<CreatureItem>*>::const_iterator itr;
        std::vector<CreatureItem> *items;

        if (result->GetFieldCount() < 6)
        {
            LOG_ERROR("Invalid format in vendors (%u/6) columns, not enough data to proceed.", result->GetFieldCount());
            delete result;
            return;
        }
        else if (result->GetFieldCount() > 6)
        {
            LOG_ERROR("Invalid format in vendors (%u/6) columns, loading anyway because we have enough data", result->GetFieldCount());
        }

#if VERSION_STRING < Cata
        DBC::Structures::ItemExtendedCostEntry const* item_extended_cost = nullptr;
#else
        DB2::Structures::ItemExtendedCostEntry const* item_extended_cost = nullptr;
#endif
        do
        {
            Field* fields = result->Fetch();

            itr = mVendors.find(fields[0].GetUInt32());

            if (itr == mVendors.end())
            {
                items = new std::vector < CreatureItem > ;
                mVendors[fields[0].GetUInt32()] = items;
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
                    LogDebugFlag(LF_DB_TABLES, "LoadVendors : Extendedcost for item %u references nonexistent EC %u", fields[1].GetUInt32(), fields[5].GetUInt32());
            }
            else
                item_extended_cost = nullptr;

            itm.extended_cost = item_extended_cost;
            items->push_back(itm);
        }
        while (result->NextRow());

        delete result;
    }
    LogDetail("ObjectMgr : %u vendors loaded.", static_cast<uint32_t>(mVendors.size()));
}

void ObjectMgr::ReloadVendors()
{
    mVendors.clear();
    LoadVendors();
}

std::vector<CreatureItem>* ObjectMgr::GetVendorList(uint32 entry)
{
    return mVendors[entry];
}

Item* ObjectMgr::CreateItem(uint32 entry, Player* owner)
{
    ItemProperties const* proto = sMySQLStore.getItemProperties(entry);
    if (proto ==nullptr)
        return nullptr;

    if (proto->InventoryType == INVTYPE_BAG)
    {
        Container* pContainer = new Container(HIGHGUID_TYPE_CONTAINER, GenerateLowGuid(HIGHGUID_TYPE_CONTAINER));
        pContainer->Create(entry, owner);
        pContainer->setStackCount(1);
        return pContainer;
    }
    else
    {
        Item* pItem = new Item;
        pItem->init(HIGHGUID_TYPE_ITEM, GenerateLowGuid(HIGHGUID_TYPE_ITEM));
        pItem->create(entry, owner);
        pItem->setStackCount(1);

#if VERSION_STRING > TBC
        if (owner != nullptr)
        {
            uint32* played = owner->GetPlayedtime();
            pItem->setCreatePlayedTime(played[1]);
        }
#endif

        return pItem;
    }
}

Item* ObjectMgr::LoadItem(uint32 lowguid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM playeritems WHERE guid = %u", lowguid);
    Item* pReturn = nullptr;

    if (result)
    {
        ItemProperties const* pProto = sMySQLStore.getItemProperties(result->Fetch()[2].GetUInt32());
        if (!pProto)
            return nullptr;

        if (pProto->InventoryType == INVTYPE_BAG)
        {
            Container* pContainer = new Container(HIGHGUID_TYPE_CONTAINER, lowguid);
            pContainer->LoadFromDB(result->Fetch());
            pReturn = pContainer;
        }
        else
        {
            Item* pItem = new Item;
            pItem->init(HIGHGUID_TYPE_ITEM, lowguid);
            pItem->LoadFromDB(result->Fetch(), nullptr, false);
            pReturn = pItem;
        }
        delete result;
    }

    return pReturn;
}

void ObjectMgr::LoadCorpses(MapMgr* mgr)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM corpses WHERE instanceid = %u", mgr->GetInstanceID());

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            Corpse* pCorpse = new Corpse(HIGHGUID_TYPE_CORPSE, fields[0].GetUInt32());
            pCorpse->SetPosition(fields[1].GetFloat(), fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat());
            pCorpse->SetZoneId(fields[5].GetUInt32());
            pCorpse->SetMapId(fields[6].GetUInt32());
            pCorpse->SetInstanceID(fields[7].GetUInt32());
            pCorpse->setCorpseDataFromDbString(fields[8].GetString());
            if (pCorpse->getDisplayId() == 0)
            {
                delete pCorpse;
                continue;
            }

            pCorpse->PushToWorld(mgr);
        }
        while (result->NextRow());

        delete result;
    }
}

#if VERSION_STRING > TBC
AchievementCriteriaEntryList const & ObjectMgr::GetAchievementCriteriaByType(AchievementCriteriaTypes type)
{
    return m_AchievementCriteriasByType[type];
}

void ObjectMgr::LoadAchievementCriteriaList()
{
#if VERSION_STRING < Cata
    for (uint32 rowId = 0; rowId < sAchievementCriteriaStore.GetNumRows(); ++rowId)
    {
        auto criteria = sAchievementCriteriaStore.LookupEntry(rowId);
        if (!criteria)
            continue;

        m_AchievementCriteriasByType[criteria->requiredType].push_back(criteria);
    }
#endif
}
#endif

void ObjectMgr::CorpseAddEventDespawn(Corpse* pCorpse)
{
    if (!pCorpse->IsInWorld())
        delete pCorpse;
    else
        sEventMgr.AddEvent(pCorpse->GetMapMgr(), &MapMgr::EventCorpseDespawn, pCorpse->getGuid(), EVENT_CORPSE_DESPAWN, 600000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
}

void ObjectMgr::CorpseCollectorUnload()
{
    CorpseMap::const_iterator itr;
    _corpseslock.Acquire();
    for (itr = m_corpses.begin(); itr != m_corpses.end(); ++itr)
    {
        Corpse* c = itr->second;
        if (c->IsInWorld())
            c->RemoveFromWorld(false);
        delete c;
    }
    m_corpses.clear();
    _corpseslock.Release();
}

#if VERSION_STRING >= Cata
//move to spellmgr or mysqldatastore todo danko
void ObjectMgr::LoadSkillLineAbilityMap()
{
    auto startTime = Util::TimeNow();

    mSkillLineAbilityMap.clear();

    uint32_t count = 0;
    for (uint32_t i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
    {
        DBC::Structures::SkillLineAbilityEntry const* SkillInfo = sSkillLineAbilityStore.LookupEntry(i);
        if (!SkillInfo)
            continue;

        mSkillLineAbilityMap.insert(SkillLineAbilityMap::value_type(SkillInfo->Id, SkillInfo));
        ++count;
    }

    LogDetail("ObjectMgr : Loaded %u SkillLineAbility MultiMap Data in %u ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}

SkillLineAbilityMapBounds ObjectMgr::GetSkillLineAbilityMapBounds(uint32_t spell_id) const
{
    return mSkillLineAbilityMap.equal_range(spell_id);
}

void ObjectMgr::LoadSpellRequired()
{
    auto startTime = Util::TimeNow();

    mSpellsReqSpell.clear();    // need for reload case
    mSpellReq.clear();          // need for reload case

    //                                                   0         1
    QueryResult* result = WorldDatabase.Query("SELECT spell_id, req_spell FROM spell_required");

    if (!result)
    {
        LogDebugFlag(LF_DB_TABLES, "ObjectMgr : Loaded 0 spell required records. DB table `spell_required` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32_t spell_id = fields[0].GetUInt32();
        uint32_t spell_req = fields[1].GetUInt32();

        // check if chain is made with valid first spell
        DBC::Structures::SpellEntry const* spell = sSpellStore.LookupEntry(spell_id);
        if (!spell)
        {
            LogDebugFlag(LF_DB_TABLES, "ObjectMgr : spell_id %u in `spell_required` table is not found in dbcs, skipped", spell_id);
            continue;
        }

        DBC::Structures::SpellEntry const* req_spell = sSpellStore.LookupEntry(spell_req);
        if (!req_spell)
        {
            LogDebugFlag(LF_DB_TABLES, "ObjectMgr : req_spell %u in `spell_required` table is not found in dbcs, skipped", spell_req);
            continue;
        }

        if (IsSpellRequiringSpell(spell_id, spell_req))
        {
            LogDebugFlag(LF_DB_TABLES, "ObjectMgr : duplicated entry of req_spell %u and spell_id %u in `spell_required`, skipped", spell_req, spell_id);
            continue;
        }

        mSpellReq.insert(std::pair<uint32_t, uint32_t>(spell_id, spell_req));
        mSpellsReqSpell.insert(std::pair<uint32_t, uint32_t>(spell_req, spell_id));
        ++count;

    } while (result->NextRow());

    LogNotice("ObjectMgr: Loaded %u spell required records in %u ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}


SpellRequiredMapBounds ObjectMgr::GetSpellsRequiredForSpellBounds(uint32_t spell_id) const
{
    return mSpellReq.equal_range(spell_id);
}

SpellsRequiringSpellMapBounds ObjectMgr::GetSpellsRequiringSpellBounds(uint32_t spell_id) const
{
    return mSpellsReqSpell.equal_range(spell_id);
}

bool ObjectMgr::IsSpellRequiringSpell(uint32_t spellid, uint32_t req_spellid) const
{
    SpellsRequiringSpellMapBounds spellsRequiringSpell = GetSpellsRequiringSpellBounds(req_spellid);
    for (SpellsRequiringSpellMap::const_iterator itr = spellsRequiringSpell.first; itr != spellsRequiringSpell.second; ++itr)
    {
        if (itr->second == spellid)
            return true;
    }
    return false;
}

const SpellsRequiringSpellMap ObjectMgr::GetSpellsRequiringSpell()
{
    return this->mSpellsReqSpell;
}

uint32_t ObjectMgr::GetSpellRequired(uint32_t spell_id) const
{
    SpellRequiredMap::const_iterator itr = mSpellReq.find(spell_id);

    if (itr == mSpellReq.end())
        return 0;

    return itr->second;
}
#endif

//MIT
void ObjectMgr::generateDatabaseGossipMenu(Object* object, uint32_t gossipMenuId, Player* player, uint32_t forcedTextId /*= 0*/)
{
    uint32_t textId = 2;

    if (forcedTextId == 0)
    {
        auto gossipMenuTextStore = sMySQLStore.getGossipMenuInitTextId();
        for (auto &initItr : *gossipMenuTextStore)
        {
            if (initItr.first == gossipMenuId)
            {
                textId = initItr.second.textId;
                break;
            }
        }
    }
    else
    {
        textId = forcedTextId;
    }

    GossipMenu menu(object->getGuid(), textId, player->GetSession()->language, gossipMenuId);

    sQuestMgr.FillQuestMenu(dynamic_cast<Creature*>(object), player, menu);

    typedef MySQLDataStore::GossipMenuItemsContainer::iterator GossipMenuItemsIterator;
    std::pair<GossipMenuItemsIterator, GossipMenuItemsIterator> gossipEqualRange = sMySQLStore._gossipMenuItemsStores.equal_range(gossipMenuId);
    for (GossipMenuItemsIterator itr = gossipEqualRange.first; itr != gossipEqualRange.second; ++itr)
    {
        // check requirements
        // 0 = none
        // 1 = has(active)Quest
        // 2 = has(finished)Quest
        // 3 = canGainXP
        // 4 = canNotGainXP

        if (itr->first == gossipMenuId)
        {
            if (itr->second.requirementType == 1 && !player->HasQuest(itr->second.requirementData))
                continue;

            if (itr->second.requirementType == 3)
            {
                if (player->CanGainXp())
                    menu.addItem(itr->second.icon, itr->second.menuOptionText, itr->second.itemOrder, "", itr->second.onChooseData, player->GetSession()->LocalizedGossipOption(itr->second.onChooseData2));
                
                continue;
            }

            if (itr->second.requirementType == 4)
            {
                if (!player->CanGainXp())
                    menu.addItem(itr->second.icon, itr->second.menuOptionText, itr->second.itemOrder, "", itr->second.onChooseData, player->GetSession()->LocalizedGossipOption(itr->second.onChooseData2));
                
                continue;
            }

            menu.addItem(itr->second.icon, itr->second.menuOptionText, itr->second.itemOrder);
        }
    }

    menu.sendGossipPacket(player);
}

//MIT
void ObjectMgr::generateDatabaseGossipOptionAndSubMenu(Object* object, Player* player, uint32_t gossipItemId, uint32_t gossipMenuId)
{
    LOG_DEBUG("GossipId: %u  gossipItemId: %u", gossipMenuId, gossipItemId);

    // bool openSubMenu = true;

    typedef MySQLDataStore::GossipMenuItemsContainer::iterator GossipMenuItemsIterator;
    std::pair<GossipMenuItemsIterator, GossipMenuItemsIterator> gossipEqualRange = sMySQLStore._gossipMenuItemsStores.equal_range(gossipMenuId);
    for (GossipMenuItemsIterator itr = gossipEqualRange.first; itr != gossipEqualRange.second; ++itr)
    {
        if (itr->second.itemOrder == gossipItemId)
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
                    generateDatabaseGossipMenu(object, itr->second.nextGossipMenu, player, itr->second.nextGossipMenuText);

                    if (itr->second.onChooseData != 0)
                        player->sendPoiById(itr->second.onChooseData);

                } break;
                case 2:
                {
                    if (itr->second.onChooseData != 0)
                    {
                        player->castSpell(player, sSpellMgr.getSpellInfo(itr->second.onChooseData), true);
                        GossipMenu::senGossipComplete(player);
                    }

                } break;
                case 3:
                {
                    if (itr->second.onChooseData != 0)
                    {
                        player->TaxiStart(sTaxiMgr.GetTaxiPath(itr->second.onChooseData), itr->second.onChooseData2, 0);
                        GossipMenu::senGossipComplete(player);
                    }

                } break;
                case 4:
                {
                    if (itr->second.onChooseData != 0)
                    {
                        if (player->GetStanding(itr->second.onChooseData) >= static_cast<int32_t>(itr->second.onChooseData2))
                            player->castSpell(player, sSpellMgr.getSpellInfo(itr->second.onChooseData3), true);
                        else
                            player->BroadcastMessage(player->GetSession()->LocalizedWorldSrv(itr->second.onChooseData4));
                        
                        GossipMenu::senGossipComplete(player);
                    }

                } break;
                case 5:
                {
                    GossipMenu::senGossipComplete(player);

                } break;
                case 6:
                {
                    if (player->hasEnoughCoinage(itr->second.onChooseData))
                    {
                        player->modCoinage(-static_cast<int32_t>(itr->second.onChooseData));
                        player->ToggleXpGain();
                        GossipMenu::senGossipComplete(player);
                    }
                } break;
                default: // action 0
                {
                    generateDatabaseGossipMenu(object, itr->second.nextGossipMenu, player, itr->second.nextGossipMenuText);
                } break;
            }
        }
    }
}

#if VERSION_STRING >= Cata
void ObjectMgr::LoadTrainers()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM trainer_defs");
    LoadDisabledSpells();

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        Trainer* tr = new Trainer;
        tr->RequiredSkill = fields[1].GetUInt32();
        tr->RequiredSkillLine = fields[2].GetUInt32();
        tr->RequiredClass = fields[3].GetUInt32();
        tr->RequiredRace = fields[4].GetUInt32();
        tr->RequiredRepFaction = fields[5].GetUInt32();
        tr->RequiredRepValue = fields[6].GetUInt32();
        tr->TrainerType = fields[7].GetUInt32();
        tr->Can_Train_Gossip_TextId = fields[9].GetUInt32();
        tr->Cannot_Train_GossipTextId = fields[10].GetUInt32();

        if (!tr->Can_Train_Gossip_TextId)
            tr->Can_Train_Gossip_TextId = 1;
        if (!tr->Cannot_Train_GossipTextId)
            tr->Cannot_Train_GossipTextId = 1;

        const char* temp = fields[8].GetString();
        size_t len = strlen(temp);
        if (len)
        {
            tr->UIMessage = new char[len + 1];
            strcpy(tr->UIMessage, temp);
            tr->UIMessage[len] = 0;
        }
        else
        {
            tr->UIMessage = new char[strlen(NormalTalkMessage) + 1];
            strcpy(tr->UIMessage, NormalTalkMessage);
            tr->UIMessage[strlen(NormalTalkMessage)] = 0;
        }

        QueryResult* result2 = WorldDatabase.Query("SELECT * FROM trainer_spells where entry='%u'", entry);
        if (!result2)
        {
            LogDebugFlag(LF_DB_TABLES, "LoadTrainers : Trainer with no spells, entry %u.", entry);
            if (tr->UIMessage != NormalTalkMessage)
                delete[] tr->UIMessage;

            delete tr;
            continue;
        }
        if (result2->GetFieldCount() != 9)
        {
            LOG_ERROR("Trainers table format is invalid. Please update your database.", NULL);
            delete tr;
            delete result;
            delete result2;
            return;
        }
        else
        {
            do
            {
                Field* fields2 = result2->Fetch();
                uint32 entry1 = fields2[0].GetUInt32();
                uint32 spell = fields2[2].GetUInt32();
                uint32 spellCost = fields2[3].GetUInt32();
                uint32 reqSkill = fields2[5].GetUInt16();
                uint32 reqSkillValue = fields2[6].GetUInt16();
                uint32 reqLevel = fields2[7].GetUInt8();

                TrainerSpell ts;
                ts.spell = spell;
                ts.spellCost = spellCost;
                ts.reqSkill = reqSkill;
                ts.reqSkillValue = reqSkillValue;
                ts.reqLevel = reqLevel;

                DBC::Structures::SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell);
                if (spellInfo == nullptr)
                    continue;

                if (!ts.reqLevel)
                    ts.reqLevel = spellInfo->GetSpellLevel();

                // calculate learned spell for profession case when stored cast-spell
                ts.learnedSpell[0] = spell;
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    auto effect = spellInfo->GetSpellEffect(i);
                    if (effect == nullptr)
                        continue;

                    if (effect->Effect != SPELL_EFFECT_LEARN_SPELL)
                        continue;

                    if (ts.learnedSpell[0] == spell)
                        ts.learnedSpell[0] = 0;
                    // player must be able to cast spell on himself
                    if (effect->EffectImplicitTargetA != 0 &&
                        effect->EffectImplicitTargetA != 21 &&
                        effect->EffectImplicitTargetA != 25 &&
                        effect->EffectImplicitTargetA != 1)
                    {
                        LogDebugFlag(LF_DB_TABLES, "LoadTrainers : Table `trainer_spels` has spell %u for trainer entry %u with learn effect which has incorrect target type, ignoring learn effect!", spell, entry1);
                        continue;
                    }

                    ts.learnedSpell[i] = spellInfo->GetSpellEffect(i)->EffectTriggerSpell;

                    if (ts.learnedSpell[i])
                    {
                        /*
                        SpellEntry const* learnedSpellInfo = sSpellMgr.getSpellInfo(ts.learnedSpell[i]);
                        if (learnedSpellInfo && learnedSpellInfo->)
                        tr->TrainerType = 2;
                        */
                    }
                }

                tr->Spells.push_back(ts);
            } while (result2->NextRow());
            delete result2;

            tr->SpellCount = (uint32)tr->Spells.size();

            //and now we insert it to our lookup table
            if (!tr->SpellCount)
            {
                if (tr->UIMessage != NormalTalkMessage)
                    delete[] tr->UIMessage;
                delete tr;
                continue;
            }

            mTrainers.insert(TrainerMap::value_type(entry, tr));
        }

    } while (result->NextRow());
    delete result;
    LogDetail("ObjectMgr : %u trainers loaded.", static_cast<uint32_t>(mTrainers.size()));
}
#else
void ObjectMgr::LoadTrainers()
{
#if VERSION_STRING > TBC    //todo: tbc
    QueryResult* result = WorldDatabase.Query("SELECT * FROM trainer_defs");
    LoadDisabledSpells();

    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        Trainer* tr = new Trainer;
        tr->RequiredSkill = fields[1].GetUInt32();
        tr->RequiredSkillLine = fields[2].GetUInt32();
        tr->RequiredClass = fields[3].GetUInt32();
        tr->RequiredRace = fields[4].GetUInt32();
        tr->RequiredRepFaction = fields[5].GetUInt32();
        tr->RequiredRepValue = fields[6].GetUInt32();
        tr->TrainerType = fields[7].GetUInt32();
        tr->Can_Train_Gossip_TextId = fields[9].GetUInt32();
        tr->Cannot_Train_GossipTextId = fields[10].GetUInt32();

        if (!tr->Can_Train_Gossip_TextId)
            tr->Can_Train_Gossip_TextId = 1;
        if (!tr->Cannot_Train_GossipTextId)
            tr->Cannot_Train_GossipTextId = 1;

        const char* temp = fields[8].GetString();
        size_t len = strlen(temp);
        if (len)
        {
            tr->UIMessage = new char[len + 1];
            strcpy(tr->UIMessage, temp);
            tr->UIMessage[len] = 0;
        }
        else
        {
            tr->UIMessage = new char[strlen(NormalTalkMessage) + 1];
            strcpy(tr->UIMessage, NormalTalkMessage);
            tr->UIMessage[strlen(NormalTalkMessage)] = 0;
        }

        //now load the spells
        QueryResult* result2 = WorldDatabase.Query("SELECT * FROM trainer_spells where entry='%u'", entry);
        if (!result2)
        {
            LogDebugFlag(LF_DB_TABLES, "LoadTrainers : Trainer with no spells, entry %u.", entry);
            if (tr->UIMessage != NormalTalkMessage)
                delete[] tr->UIMessage;

            delete tr;
            continue;
        }
        if (result2->GetFieldCount() != 9)
        {
            LOG_ERROR("Trainers table format is invalid. Please update your database.");
            delete tr;
            delete result;
            delete result2;
            return;
        }
        else
        {
            do
            {
                Field* fields2 = result2->Fetch();
                TrainerSpell ts;
                bool abrt = false;
                uint32 CastSpellID = fields2[1].GetUInt32();
                uint32 LearnSpellID = fields2[2].GetUInt32();

                ts.pCastSpell = NULL;
                ts.pLearnSpell = NULL;
                ts.pCastRealSpell = NULL;

                if (CastSpellID != 0)
                {
                    ts.pCastSpell = sSpellMgr.getSpellInfo(CastSpellID);
                    if (ts.pCastSpell)
                    {
                        for (uint8 k = 0; k < 3; ++k)
                        {
                            if (ts.pCastSpell->getEffect(k) == SPELL_EFFECT_LEARN_SPELL)
                            {
                                ts.pCastRealSpell = sSpellMgr.getSpellInfo(ts.pCastSpell->getEffectTriggerSpell(k));
                                if (ts.pCastRealSpell == NULL)
                                {
                                    LOG_ERROR("Trainer %u contains cast spell %u that is non-teaching", entry, CastSpellID);
                                    abrt = true;
                                }
                                break;
                            }
                        }
                    }

                    if (abrt)
                        continue;
                }

                if (LearnSpellID != 0)
                {
                    ts.pLearnSpell = sSpellMgr.getSpellInfo(LearnSpellID);
                }

                if (ts.pCastSpell == NULL && ts.pLearnSpell == NULL)
                {
                    LOG_ERROR("Trainer %u without valid spells (%u/%u).", entry, CastSpellID, LearnSpellID);
                    continue; //omg a bad spell !
                }

                if (ts.pCastSpell && !ts.pCastRealSpell)
                    continue;

                ts.Cost = fields2[3].GetUInt32();
                ts.RequiredSpell = fields2[4].GetUInt32();
                ts.RequiredSkillLine = fields2[5].GetUInt32();
                ts.RequiredSkillLineValue = fields2[6].GetUInt32();
                ts.RequiredLevel = fields2[7].GetUInt32();
                ts.DeleteSpell = fields2[8].GetUInt32();
                //IsProfession is true if the TrainerSpell will teach a primary profession
                if (ts.RequiredSkillLine == 0 && ts.pCastRealSpell != NULL && ts.pCastRealSpell->getEffect(1) == SPELL_EFFECT_SKILL)
                {
                    uint32 skill = ts.pCastRealSpell->getEffectMiscValue(1);
                    auto skill_line = sSkillLineStore.LookupEntry(skill);
                    ARCEMU_ASSERT(skill_line != NULL);
                    if (skill_line->type == SKILL_TYPE_PROFESSION)
                        ts.IsProfession = true;
                    else
                        ts.IsProfession = false;
                }
                else
                    ts.IsProfession = false;

                tr->Spells.push_back(ts);
            }
            while (result2->NextRow());
            delete result2;

            tr->SpellCount = (uint32)tr->Spells.size();

            //and now we insert it to our lookup table
            if (!tr->SpellCount)
            {
                if (tr->UIMessage != NormalTalkMessage)
                    delete[] tr->UIMessage;
                delete tr;
                continue;
            }

            mTrainers.insert(TrainerMap::value_type(entry, tr));
        }

    }
    while (result->NextRow());
    delete result;
    LogDetail("ObjectMgr : %u trainers loaded.", static_cast<uint32_t>(mTrainers.size()));
#endif
}
#endif

Trainer* ObjectMgr::GetTrainer(uint32 Entry)
{
    TrainerMap::iterator iter = mTrainers.find(Entry);
    if (iter == mTrainers.end())
        return nullptr;

    return iter->second;
}

void ObjectMgr::GenerateLevelUpInfo()
{
    // Generate levelup information for each class.
    for (uint8 Class = WARRIOR; Class <= DRUID; ++Class)
    {
        // These are empty.
        if (Class == 10)
            continue;

        // Search for a playercreateinfo.
        for (uint8 Race = RACE_HUMAN; Race <= DBC_NUM_RACES - 1; ++Race)
        {
            PlayerCreateInfo const* PCI = sMySQLStore.getPlayerCreateInfo(static_cast<uint8>(Race), static_cast<uint8>(Class));

            if (PCI == nullptr)
                continue;   // Class not valid for this race.

            // Generate each level's information
            uint32 MaxLevel = worldConfig.player.playerLevelCap + 1;
            LevelInfo* lvl = nullptr, lastlvl;
            lastlvl.HP = PCI->health;
            lastlvl.Mana = PCI->mana;
            lastlvl.Stat[0] = PCI->strength;
            lastlvl.Stat[1] = PCI->ability;
            lastlvl.Stat[2] = PCI->stamina;
            lastlvl.Stat[3] = PCI->intellect;
            lastlvl.Stat[4] = PCI->spirit;
            LevelMap* lMap = new LevelMap;

            // Create first level.
            lvl = new LevelInfo;
            *lvl = lastlvl;

            // Insert into map
            lMap->insert(LevelMap::value_type(1, lvl));

            uint32 val;
            for (uint32 Level = 2; Level < MaxLevel; ++Level)
            {
                lvl = new LevelInfo;

                // Calculate Stats
                for (uint32 s = 0; s < 5; ++s)
                {
                    val = GainStat(static_cast<uint16>(Level), static_cast<uint8>(Class), static_cast<uint8>(s));
                    lvl->Stat[s] = lastlvl.Stat[s] + val;
                }

                // Calculate HP/Mana
                uint32 TotalHealthGain = 0;
                uint32 TotalManaGain = 0;

                switch (Class)
                {
                    case WARRIOR:
                        if (Level < 13)
                            TotalHealthGain += 19;
                        else if (Level < 36)
                            TotalHealthGain += Level + 6;
                        //                    else if (Level >60) TotalHealthGain+=Level+100;
                        else if (Level > 60)
                            TotalHealthGain += Level + 206;
                        else
                            TotalHealthGain += 2 * Level - 30;
                        break;
                    case HUNTER:
                        if (Level < 13)
                            TotalHealthGain += 17;
                        //                    else if (Level >60) TotalHealthGain+=Level+45;
                        else if (Level > 60)
                            TotalHealthGain += Level + 161;
                        else
                            TotalHealthGain += Level + 4;

                        if (Level < 11)
                            TotalManaGain += 29;
                        else if (Level < 27)
                            TotalManaGain += Level + 18;
                        //                    else if (Level>60)TotalManaGain+=Level+20;
                        else if (Level > 60)
                            TotalManaGain += Level + 150;
                        else
                            TotalManaGain += 45;
                        break;
                    case ROGUE:
                        if (Level < 15)
                            TotalHealthGain += 17;
                        //                    else if (Level >60) TotalHealthGain+=Level+110;
                        else if (Level > 60)
                            TotalHealthGain += Level + 191;
                        else
                            TotalHealthGain += Level + 2;
                        break;
                    case DRUID:
                        if (Level < 17)
                            TotalHealthGain += 17;
                        //                    else if (Level >60) TotalHealthGain+=Level+55;
                        else if (Level > 60)
                            TotalHealthGain += Level + 176;
                        else
                            TotalHealthGain += Level;

                        if (Level < 26)
                            TotalManaGain += Level + 20;
                        //                    else if (Level>60)TotalManaGain+=Level+25;
                        else if (Level > 60)
                            TotalManaGain += Level + 150;
                        else
                            TotalManaGain += 45;
                        break;
                    case MAGE:
                        if (Level < 23)
                            TotalHealthGain += 15;
                        //                    else if (Level >60) TotalHealthGain+=Level+40;
                        else if (Level > 60)
                            TotalHealthGain += Level + 190;
                        else
                            TotalHealthGain += Level - 8;

                        if (Level < 28)
                            TotalManaGain += Level + 23;
                        //                    else if (Level>60)TotalManaGain+=Level+26;
                        else if (Level > 60)
                            TotalManaGain += Level + 115;
                        else
                            TotalManaGain += 51;
                        break;
                    case SHAMAN:
                        if (Level < 16)
                            TotalHealthGain += 17;
                        //                    else if (Level >60) TotalHealthGain+=Level+75;
                        else if (Level > 60)
                            TotalHealthGain += Level + 157;
                        else
                            TotalHealthGain += Level + 1;

                        if (Level < 22)
                            TotalManaGain += Level + 19;
                        //                    else if (Level>60)TotalManaGain+=Level+70;
                        else if (Level > 60)
                            TotalManaGain += Level + 175;
                        else
                            TotalManaGain += 49;
                        break;
                    case WARLOCK:
                        if (Level < 17)
                            TotalHealthGain += 17;
                        //                    else if (Level >60) TotalHealthGain+=Level+50;
                        else if (Level > 60)
                            TotalHealthGain += Level + 192;
                        else
                            TotalHealthGain += Level - 2;

                        if (Level < 30)
                            TotalManaGain += Level + 21;
                        //                    else if (Level>60)TotalManaGain+=Level+25;
                        else if (Level > 60)
                            TotalManaGain += Level + 121;
                        else
                            TotalManaGain += 51;
                        break;
                    case PALADIN:
                        if (Level < 14)
                            TotalHealthGain += 18;
                        //                    else if (Level >60) TotalHealthGain+=Level+55;
                        else if (Level > 60)
                            TotalHealthGain += Level + 167;
                        else
                            TotalHealthGain += Level + 4;

                        if (Level < 30)
                            TotalManaGain += Level + 17;
                        //                    else if (Level>60)TotalManaGain+=Level+100;
                        else if (Level > 60)
                            TotalManaGain += Level + 131;
                        else
                            TotalManaGain += 42;
                        break;
                    case PRIEST:
                        if (Level < 21)
                            TotalHealthGain += 15;
                        //                    else if (Level >60) TotalHealthGain+=Level+40;
                        else if (Level > 60)
                            TotalHealthGain += Level + 157;
                        else
                            TotalHealthGain += Level - 6;

                        if (Level < 22)
                            TotalManaGain += Level + 22;
                        else if (Level < 32)
                            TotalManaGain += Level + 37;
                        //                    else if (Level>60)TotalManaGain+=Level+35;
                        else if (Level > 60)
                            TotalManaGain += Level + 207;
                        else
                            TotalManaGain += 54;
                        break;
#if VERSION_STRING > TBC
                    case DEATHKNIGHT: // Based on 55-56 more testing will be done.
                        if (Level < 60)
                            TotalHealthGain += 92;
                        /*else if (Level <60) TotalHealthGain+=??;
                        else if (Level <70) TotalHealthGain+=??;*/
                        else
                            TotalHealthGain += 92;
                        break;
#endif
                }

                // Apply HP/Mana
                lvl->HP = lastlvl.HP + TotalHealthGain;
                lvl->Mana = lastlvl.Mana + TotalManaGain;

                lastlvl = *lvl;

                // Apply to map.
                lMap->insert(LevelMap::value_type(Level, lvl));
            }

            // Now that our level map is full, let's create the class/race pair
            std::pair<uint32, uint32> p;
            p.first = Race;
            p.second = Class;

            // Insert back into the main map.
            mLevelInfo.insert(LevelInfoMap::value_type(p, lMap));
        }
    }
    LogNotice("ObjectMgr : %u level up information generated.", static_cast<uint32_t>(mLevelInfo.size()));
}

LevelInfo* ObjectMgr::GetLevelInfo(uint32 Race, uint32 Class, uint32 Level)
{
    // Iterate levelinfo map until we find the right class+race.
    LevelInfoMap::iterator itr = mLevelInfo.begin();
    for (; itr != mLevelInfo.end(); ++itr)
    {
        if (itr->first.first == Race && itr->first.second == Class)
        {
            // We got a match.
            // Let's check that our level is valid first.
            if (Level > worldConfig.player.playerLevelCap)
                Level = worldConfig.player.playerLevelCap;

            // Pull the level information from the second map.
            LevelMap::iterator it2 = itr->second->find(Level);
            if (it2 == itr->second->end())
            {
                LogDebugFlag(LF_DB_TABLES, "GetLevelInfo : No level information found for level %u!", Level);
                return nullptr;
            }

            return it2->second;
        }
    }

    return nullptr;
}

void ObjectMgr::LoadPetSpellCooldowns()
{
    for (uint32 i = 0; i < sCreatureSpellDataStore.GetNumRows(); ++i)
    {
        auto creture_spell_data = sCreatureSpellDataStore.LookupEntry(i);

        for (uint8 j = 0; j < 3; ++j)
        {
            if (creture_spell_data == nullptr)
                continue;

            uint32 SpellId = creture_spell_data->Spells[j];
            uint32 Cooldown = creture_spell_data->Cooldowns[j] * 10;

            if (SpellId != 0)
            {
                PetSpellCooldownMap::iterator itr = mPetSpellCooldowns.find(SpellId);
                if (itr == mPetSpellCooldowns.end())
                {
                    if (Cooldown)
                        mPetSpellCooldowns.insert(std::make_pair(SpellId, Cooldown));
                }
                else
                {
                    uint32 SP2 = mPetSpellCooldowns[SpellId];
                    ARCEMU_ASSERT(Cooldown == SP2);
                }
            }
        }
    }
}

uint32 ObjectMgr::GetPetSpellCooldown(uint32 SpellId)
{
    PetSpellCooldownMap::iterator itr = mPetSpellCooldowns.find(SpellId);
    if (itr != mPetSpellCooldowns.end())
        return itr->second;

    SpellInfo const* sp = sSpellMgr.getSpellInfo(SpellId);
    if (sp->getRecoveryTime() > sp->getCategoryRecoveryTime())
        return sp->getRecoveryTime();
    else
        return sp->getCategoryRecoveryTime();
}

void ObjectMgr::SetVendorList(uint32 Entry, std::vector<CreatureItem>* list_)
{
    mVendors[Entry] = list_;
}

void ObjectMgr::LoadCreatureTimedEmotes()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM creature_timed_emotes order by rowid asc");
    if (!result)return;

    do
    {
        Field* fields = result->Fetch();
        spawn_timed_emotes* te = new spawn_timed_emotes;
        te->type = fields[2].GetUInt8();
        te->value = fields[3].GetUInt32();
        char* str = (char*)fields[4].GetString();
        if (str)
        {
            uint32 len = (int)strlen(str);
            te->msg = new char[len + 1];
            memcpy(te->msg, str, len + 1);
        }
        else te->msg = nullptr;
        te->msg_type = static_cast<uint8>(fields[5].GetUInt32());
        te->msg_lang = static_cast<uint8>(fields[6].GetUInt32());
        te->expire_after = fields[7].GetUInt32();

        std::unordered_map<uint32, TimedEmoteList*>::const_iterator i;
        uint32 spawnid = fields[0].GetUInt32();
        i = m_timedemotes.find(spawnid);
        if (i == m_timedemotes.end())
        {
            TimedEmoteList* m = new TimedEmoteList;
            m->push_back(te);
            m_timedemotes[spawnid] = m;
        }
        else
        {
            i->second->push_back(te);
        }
    }
    while (result->NextRow());

    LogNotice("ObjectMgr : %u timed emotes cached.", result->GetRowCount());
    delete result;
}

TimedEmoteList* ObjectMgr::GetTimedEmoteList(uint32 spawnid)
{
    std::unordered_map<uint32, TimedEmoteList*>::const_iterator i;
    i = m_timedemotes.find(spawnid);
    if (i != m_timedemotes.end())
    {
        TimedEmoteList* m = i->second;
        return m;
    }
    else return nullptr;
}

void ObjectMgr::LoadCreatureWaypoints()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM creature_waypoints");
    if (result == nullptr)
        return;

    uint32_t waypointCount = 0;
#ifdef EXTENDED_DB_CHECKS
    uint32_t cachedSpawnId = 0;
    bool isValidSpawn = true;
#endif
    do
    {
        Field* fields = result->Fetch();
        uint32_t spawnid = fields[0].GetUInt32();

        // expensive check
#ifdef EXTENDED_DB_CHECKS
        if (cachedSpawnId != spawnid)
        {
            cachedSpawnId = spawnid;
            isValidSpawn = true;
        }

        if (isValidSpawn == false)
        {
            continue;
        }

        if (isValidSpawn == true)
        {
            QueryResult* spawnResult = WorldDatabase.Query("SELECT * FROM creature_spawns WHERE id = %u", spawnid);
            if (spawnResult == nullptr)
            {
                LogDebugFlag(LF_DB_TABLES, "Table `creature_waypoints` includes waypoints for invalid spawndid %u, Skipped!", spawnid);
                isValidSpawn = false;
                continue;
            }
        }
#endif

        Movement::WayPoint* wp = new Movement::WayPoint;
        wp->id = fields[1].GetUInt32();
        wp->x = fields[2].GetFloat();
        wp->y = fields[3].GetFloat();
        wp->z = fields[4].GetFloat();
        wp->waittime = fields[5].GetUInt32();
        wp->flags = fields[6].GetUInt32();
        wp->forwardemoteoneshot = fields[7].GetBool();
        wp->forwardemoteid = fields[8].GetUInt32();
        wp->backwardemoteoneshot = fields[9].GetBool();
        wp->backwardemoteid = fields[10].GetUInt32();
        wp->forwardskinid = fields[11].GetUInt32();
        wp->backwardskinid = fields[12].GetUInt32();

        std::unordered_map<uint32, Movement::WayPointMap*>::const_iterator i;
        i = mWayPointMap.find(spawnid);
        if (i == mWayPointMap.end())
        {
            Movement::WayPointMap* m = new Movement::WayPointMap;
            if (m->size() <= wp->id)
                m->resize(wp->id + 1);
            (*m)[wp->id] = wp;
            mWayPointMap[spawnid] = m;
        }
        else
        {
            if (i->second->size() <= wp->id)
                i->second->resize(wp->id + 1);

            (*(i->second))[wp->id] = wp;
        }

        ++waypointCount;
    }
    while (result->NextRow());

    LogNotice("ObjectMgr : %u waypoints cached.", waypointCount);
    delete result;
}

Movement::WayPointMap* ObjectMgr::GetWayPointMap(uint32 spawnid)
{
    std::unordered_map<uint32, Movement::WayPointMap*>::const_iterator i;
    i = mWayPointMap.find(spawnid);
    if (i != mWayPointMap.end())
    {
        Movement::WayPointMap* m = i->second;
        // we don't wanna erase from the map, because some are used more
        // than once (for instances)

        //m_waypoints.erase(i);
        return m;
    }
    else return nullptr;
}

Pet* ObjectMgr::CreatePet(uint32 entry)
{
    uint32 guid;
    guid = ++m_hiPetGuid;
    return new Pet(WoWGuid(guid, entry, HIGHGUID_TYPE_PET));
}

Player* ObjectMgr::CreatePlayer(uint8 _class)
{
    uint32 guid;
    guid = ++m_hiPlayerGuid;

    Player* result = nullptr;
    switch (_class)
    {
        case WARRIOR:
            result = new Warrior(guid);
            break;
        case PALADIN:
            result = new Paladin(guid);
            break;
        case HUNTER:
            result = new Hunter(guid);
            break;
        case ROGUE:
            result = new Rogue(guid);
            break;
        case PRIEST:
            result = new Priest(guid);
            break;
#if VERSION_STRING > TBC
        case DEATHKNIGHT:
            result = new DeathKnight(guid);
            break;
#endif
        case SHAMAN:
            result = new Shaman(guid);
            break;
        case MAGE:
            result = new Mage(guid);
            break;
        case WARLOCK:
            result = new Warlock(guid);
            break;
#if VERSION_STRING > CATA
        case MONK:
            result = new Monk(guid);
            break;
#endif
        case DRUID:
            result = new Druid(guid);
            break;
    }
    return result;
}

void ObjectMgr::AddPlayer(Player* p)
{
    std::lock_guard<std::mutex> guard(_playerslock);

    _players[p->getGuidLow()] = p;
}

void ObjectMgr::RemovePlayer(Player* p)
{
    std::lock_guard<std::mutex> guard(_playerslock);

    _players.erase(p->getGuidLow());
}

Corpse* ObjectMgr::CreateCorpse()
{
    uint32 guid;
    guid = ++m_hiCorpseGuid;

    return new Corpse(HIGHGUID_TYPE_CORPSE, guid);
}

void ObjectMgr::AddCorpse(Corpse* p) //add it to global storage
{
    _corpseslock.Acquire();
    m_corpses[p->getGuidLow()] = p;
    _corpseslock.Release();
}

void ObjectMgr::RemoveCorpse(Corpse* p)
{
    _corpseslock.Acquire();
    m_corpses.erase(p->getGuidLow());
    _corpseslock.Release();
}

Corpse* ObjectMgr::GetCorpse(uint32 corpseguid)
{
    Corpse* rv = nullptr;
    _corpseslock.Acquire();
    CorpseMap::const_iterator itr = m_corpses.find(corpseguid);
    rv = (itr != m_corpses.end()) ? itr->second : 0;
    _corpseslock.Release();
    return rv;
}

void ObjectMgr::LoadGuildCharters()
{
    m_hiCharterId = 0;
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM charters");
    if (!result)
        return;
    do
    {
        Charter* c = new Charter(result->Fetch());
        m_charters[c->CharterType].insert(std::make_pair(c->GetID(), c));
        if (c->GetID() > int64(m_hiCharterId.load()))
            m_hiCharterId = c->GetID();
    }
    while (result->NextRow());
    delete result;
    LogDetail("ObjectMgr : %u charters loaded.", static_cast<uint32_t>(m_charters[0].size()));
}

Charter* ObjectMgr::GetCharter(uint32 CharterId, CharterTypes Type)
{
    std::lock_guard<std::mutex> guard(m_charterLock);

    std::unordered_map<uint32, Charter*>::iterator itr = m_charters[Type].find(CharterId);
    return (itr == m_charters[Type].end()) ? nullptr : itr->second;
}

Charter* ObjectMgr::CreateCharter(uint32 LeaderGuid, CharterTypes Type)
{
    uint32 charterid = 0;
    charterid = ++m_hiCharterId;

    Charter* c = new Charter(charterid, LeaderGuid, Type);
    m_charters[c->CharterType].insert(std::make_pair(c->GetID(), c));

    return c;
}

Charter* ObjectMgr::GetCharterByItemGuid(uint64 guid)
{
    std::lock_guard<std::mutex> guard(m_charterLock);

    for (uint8 i = 0; i < NUM_CHARTER_TYPES; ++i)
    {
        for (std::unordered_map<uint32, Charter*>::iterator itr = m_charters[i].begin(); itr != m_charters[i].end(); ++itr)
        {
            if (itr->second->ItemGuid == guid)
                return itr->second;
        }
    }

    return nullptr;
}

Charter* ObjectMgr::GetCharterByGuid(uint64 playerguid, CharterTypes type)
{
    std::lock_guard<std::mutex> guard(m_charterLock);

    for (std::unordered_map<uint32, Charter*>::iterator itr = m_charters[type].begin(); itr != m_charters[type].end(); ++itr)
    {
        if (playerguid == itr->second->LeaderGuid)
            return itr->second;

        for (uint32 j = 0; j < itr->second->SignatureCount; ++j)
        {
            if (itr->second->Signatures[j] == playerguid)
                return itr->second;
        }
    }

    return nullptr;
}

Charter* ObjectMgr::GetCharterByName(std::string & charter_name, CharterTypes Type)
{
    std::lock_guard<std::mutex> guard(m_charterLock);

    for (std::unordered_map<uint32, Charter*>::iterator itr = m_charters[Type].begin(); itr != m_charters[Type].end(); ++itr)
    {
        if (itr->second->GuildName == charter_name)
            return itr->second;
    }

    return nullptr;
}

void ObjectMgr::RemoveCharter(Charter* c)
{
    if (c == nullptr)
        return;

    if (c->CharterType >= NUM_CHARTER_TYPES)
    {
        LogDebugFlag(LF_DB_TABLES, "ObjectMgr : Charter %u cannot be destroyed as type %u is not a sane type value.", c->CharterId, c->CharterType);
        return;
    }

    std::lock_guard<std::mutex> guard(m_charterLock);

    m_charters[c->CharterType].erase(c->CharterId);
}

void ObjectMgr::LoadReputationModifierTable(const char* tablename, ReputationModMap* dmap)
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM %s", tablename);

    if (result)
    {
        do
        {
            ReputationMod mod;
            mod.faction[TEAM_ALLIANCE] = result->Fetch()[1].GetUInt32();
            mod.faction[TEAM_HORDE] = result->Fetch()[2].GetUInt32();
            mod.value = result->Fetch()[3].GetInt32();
            mod.replimit = result->Fetch()[4].GetUInt32();

            ReputationModMap::iterator itr = dmap->find(result->Fetch()[0].GetUInt32());
            if (itr == dmap->end())
            {
                ReputationModifier* modifier = new ReputationModifier;
                modifier->entry = result->Fetch()[0].GetUInt32();
                modifier->mods.push_back(mod);
                dmap->insert(ReputationModMap::value_type(result->Fetch()[0].GetUInt32(), modifier));
            }
            else
            {
                itr->second->mods.push_back(mod);
            }
        }
        while (result->NextRow());
        delete result;
    }
    LogNotice("ObjectMgr : %u reputation modifiers on %s.", static_cast<uint32_t>(dmap->size()), tablename);
}

void ObjectMgr::LoadReputationModifiers()
{
    LoadReputationModifierTable("reputation_creature_onkill", &m_reputation_creature);
    LoadReputationModifierTable("reputation_faction_onkill", &m_reputation_faction);
    LoadInstanceReputationModifiers();
}

ReputationModifier* ObjectMgr::GetReputationModifier(uint32 entry_id, uint32 faction_id)
{
    // first, try fetching from the creature table (by faction is a fallback)
    ReputationModMap::iterator itr = m_reputation_creature.find(entry_id);
    if (itr != m_reputation_creature.end())
        return itr->second;

    // fetch from the faction table
    itr = m_reputation_faction.find(faction_id);
    if (itr != m_reputation_faction.end())
        return itr->second;

    // no data. fallback to default -5 value.
    return nullptr;
}

void ObjectMgr::LoadInstanceReputationModifiers()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM reputation_instance_onkill");
    if (!result)
        return;
    do
    {
        Field* fields = result->Fetch();
        InstanceReputationMod mod;
        mod.mapid = fields[0].GetUInt32();
        mod.mob_rep_reward = fields[1].GetInt32();
        mod.mob_rep_limit = fields[2].GetUInt32();
        mod.boss_rep_reward = fields[3].GetInt32();
        mod.boss_rep_limit = fields[4].GetUInt32();
        mod.faction[TEAM_ALLIANCE] = fields[5].GetUInt32();
        mod.faction[TEAM_HORDE] = fields[6].GetUInt32();

        std::unordered_map<uint32, InstanceReputationModifier*>::iterator itr = m_reputation_instance.find(mod.mapid);
        if (itr == m_reputation_instance.end())
        {
            InstanceReputationModifier* m = new InstanceReputationModifier;
            m->mapid = mod.mapid;
            m->mods.push_back(mod);
            m_reputation_instance.insert(std::make_pair(m->mapid, m));
        }
        else
            itr->second->mods.push_back(mod);

    }
    while (result->NextRow());
    delete result;

    LogDetail("ObjectMgr : %u instance reputation modifiers loaded.", static_cast<uint32_t>(m_reputation_instance.size()));
}

bool ObjectMgr::HandleInstanceReputationModifiers(Player* pPlayer, Unit* pVictim)
{
    uint32 team = pPlayer->getTeam();

    if (!pVictim->isCreature())
        return false;

    std::unordered_map<uint32, InstanceReputationModifier*>::iterator itr = m_reputation_instance.find(pVictim->GetMapId());
    if (itr == m_reputation_instance.end())
        return false;

    bool is_boss = false;
    if (static_cast< Creature* >(pVictim)->GetCreatureProperties()->isBoss)
        is_boss = true;

    // Apply the bonuses as normal.
    int32 replimit;
    int32 value;

    for (std::vector<InstanceReputationMod>::iterator i = itr->second->mods.begin(); i != itr->second->mods.end(); ++i)
    {
        if (!(*i).faction[team])
            continue;

        if (is_boss)
        {
            value = i->boss_rep_reward;
            replimit = i->boss_rep_limit;
        }
        else
        {
            value = i->mob_rep_reward;
            replimit = i->mob_rep_limit;
        }

        if (!value || (replimit && pPlayer->GetStanding(i->faction[team]) >= replimit))
            continue;

        //value *= sWorld.getRate(RATE_KILLREPUTATION);
        value = float2int32(value * worldConfig.getFloatRate(RATE_KILLREPUTATION));
        pPlayer->ModStanding(i->faction[team], value);
    }

    return true;
}

void ObjectMgr::LoadDisabledSpells()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM spell_disable");
    if (result)
    {
        do
        {
            m_disabled_spells.insert(result->Fetch()[0].GetUInt32());

        } while (result->NextRow());

        delete result;
    }

    LogNotice("ObjectMgr : %u disabled spells.", static_cast<uint32_t>(m_disabled_spells.size()));
}

void ObjectMgr::ReloadDisabledSpells()
{
    m_disabled_spells.clear();
    LoadDisabledSpells();
}

void ObjectMgr::LoadGroups()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM `groups`");
    if (result)
    {
        if (result->GetFieldCount() != 52)
        {
            LOG_ERROR("groups table format is invalid. Please update your database.");
            return;
        }
        do
        {
            Group* g = new Group(false);
            g->LoadFromDB(result->Fetch());
        }
        while (result->NextRow());
        delete result;
    }

    LogDetail("ObjectMgr : %u groups loaded.", static_cast<uint32_t>(this->m_groups.size()));
}

void ObjectMgr::LoadArenaTeams()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM arenateams");
    if (result != nullptr)
    {
        if (result->GetFieldCount() != 22)
        {
            LOG_ERROR("arenateams table format is invalid. Please update your database.");
            return;
        }
        do
        {
            ArenaTeam* team = new ArenaTeam(result->Fetch());
            AddArenaTeam(team);
            if (team->m_id > uint32(m_hiArenaTeamId.load()))
                m_hiArenaTeamId = uint32(team->m_id);

        }
        while (result->NextRow());
        delete result;
    }

    /* update the ranking */
    UpdateArenaTeamRankings();
}

ArenaTeam* ObjectMgr::GetArenaTeamByGuid(uint32 guid, uint32 Type)
{
    m_arenaTeamLock.Acquire();
    for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeamMap[Type].begin(); itr != m_arenaTeamMap[Type].end(); ++itr)
    {
        if (itr->second->isMember(guid))
        {
            m_arenaTeamLock.Release();
            return itr->second;
        }
    }
    m_arenaTeamLock.Release();
    return nullptr;
}

ArenaTeam* ObjectMgr::GetArenaTeamById(uint32 id)
{
    std::unordered_map<uint32, ArenaTeam*>::iterator itr;
    m_arenaTeamLock.Acquire();
    itr = m_arenaTeams.find(id);
    m_arenaTeamLock.Release();
    return (itr == m_arenaTeams.end()) ? nullptr : itr->second;
}

ArenaTeam* ObjectMgr::GetArenaTeamByName(std::string & name, uint32 /*Type*/)
{
    m_arenaTeamLock.Acquire();
    for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeams.begin(); itr != m_arenaTeams.end(); ++itr)
    {
        if (!strnicmp(itr->second->m_name.c_str(), name.c_str(), name.size()))
        {
            m_arenaTeamLock.Release();
            return itr->second;
        }
    }
    m_arenaTeamLock.Release();
    return nullptr;
}

void ObjectMgr::RemoveArenaTeam(ArenaTeam* team)
{
    m_arenaTeamLock.Acquire();
    m_arenaTeams.erase(team->m_id);
    m_arenaTeamMap[team->m_type].erase(team->m_id);
    m_arenaTeamLock.Release();
}

void ObjectMgr::AddArenaTeam(ArenaTeam* team)
{
    m_arenaTeamLock.Acquire();
    m_arenaTeams[team->m_id] = team;
    m_arenaTeamMap[team->m_type].insert(std::make_pair(team->m_id, team));
    m_arenaTeamLock.Release();
}

class ArenaSorter
{
    public:

        bool operator()(ArenaTeam* const & a, ArenaTeam* const & b)
        {
            return (a->m_stats.rating > b->m_stats.rating);
        }

        bool operator()(ArenaTeam*& a, ArenaTeam*& b)
        {
            return (a->m_stats.rating > b->m_stats.rating);
        }
};

void ObjectMgr::UpdateArenaTeamRankings()
{
    m_arenaTeamLock.Acquire();
    for (uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
    {
        std::vector<ArenaTeam*> ranking;

        for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeamMap[i].begin(); itr != m_arenaTeamMap[i].end(); ++itr)
            ranking.push_back(itr->second);

        std::sort(ranking.begin(), ranking.end(), ArenaSorter());
        uint32 rank = 1;
        for (std::vector<ArenaTeam*>::iterator itr = ranking.begin(); itr != ranking.end(); ++itr)
        {
            if ((*itr)->m_stats.ranking != rank)
            {
                (*itr)->m_stats.ranking = rank;
                (*itr)->SaveToDB();
            }
            ++rank;
        }
    }
    m_arenaTeamLock.Release();
}

void ObjectMgr::ResetArenaTeamRatings()
{
    m_arenaTeamLock.Acquire();
    for (uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
    {
        for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeamMap[i].begin(); itr != m_arenaTeamMap[i].end(); ++itr)
        {
            ArenaTeam* team = itr->second;
            if (team)
            {
                team->m_stats.played_season = 0;
                team->m_stats.played_week = 0;
                team->m_stats.won_season = 0;
                team->m_stats.won_week = 0;
                team->m_stats.rating = 1500;
                for (uint32 j = 0; j < team->m_memberCount; ++j)
                {
                    team->m_members[j].Played_ThisSeason = 0;
                    team->m_members[j].Played_ThisWeek = 0;
                    team->m_members[j].Won_ThisSeason = 0;
                    team->m_members[j].Won_ThisWeek = 0;
                    team->m_members[j].PersonalRating = 1500;
                }
                team->SaveToDB();
            }
        }
    }
    m_arenaTeamLock.Release();

    UpdateArenaTeamRankings();
}

void ObjectMgr::UpdateArenaTeamWeekly()
{
    // reset weekly matches count for all teams and all members
    m_arenaTeamLock.Acquire();
    for (uint8 i = 0; i < NUM_ARENA_TEAM_TYPES; ++i)
    {
        for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeamMap[i].begin(); itr != m_arenaTeamMap[i].end(); ++itr)
        {
            ArenaTeam* team = itr->second;
            if (team)
            {
                team->m_stats.played_week = 0;
                team->m_stats.won_week = 0;
                for (uint32 j = 0; j < team->m_memberCount; ++j)
                {
                    team->m_members[j].Played_ThisWeek = 0;
                    team->m_members[j].Won_ThisWeek = 0;
                }
                team->SaveToDB();
            }
        }
    }
    m_arenaTeamLock.Release();
}

void ObjectMgr::ResetDailies()
{
    std::lock_guard<std::mutex> guard(_playerslock);

    for (PlayerStorageMap::iterator itr = _players.begin(); itr != _players.end(); ++itr)
    {
        Player* pPlayer = itr->second;
        pPlayer->DailyMutex.Acquire();
        pPlayer->m_finishedDailies.clear();
        pPlayer->DailyMutex.Release();
    }
}

void ObjectMgr::LoadSpellTargetConstraints()
{
    LogNotice("ObjectMgr : Loading spell target constraints...");

    // Let's try to be idiot proof :/
    QueryResult* result = WorldDatabase.Query("SELECT * FROM spelltargetconstraints WHERE SpellID > 0 ORDER BY SpellID");
    if (result != nullptr)
    {
        uint32 oldspellid = 0;
        SpellTargetConstraint* stc = nullptr;

        do
        {
            Field* fields = result->Fetch();

            if (fields != nullptr)
            {
                uint32 spellid = fields[0].GetUInt32();

                if (oldspellid != spellid)
                {
                    stc = new SpellTargetConstraint;

                    m_spelltargetconstraints.insert(std::pair< uint32, SpellTargetConstraint* >(spellid, stc));
                }

                uint8_t type = fields[1].GetUInt8();
                uint32 value = fields[2].GetUInt32();

                if (type == SPELL_CONSTRAINT_EXPLICIT_CREATURE)
                {
                    if (stc != nullptr)
                    {
                        stc->addCreature(value);
                        stc->addExplicitTarget(value);
                    }
                }
                else if (type == SPELL_CONSTRAINT_EXPLICIT_GAMEOBJECT)
                {
                    if (stc != nullptr)
                    {
                        stc->addGameObject(value);
                        stc->addExplicitTarget(value);
                    }
                }
                else if (type == SPELL_CONSTRAINT_IMPLICIT_CREATURE)
                {
                    if (stc != nullptr)
                    {
                        stc->addCreature(value);
                    }
                }
                else if (type == SPELL_CONSTRAINT_IMPLICIT_GAMEOBJECT)
                {
                    if (stc != nullptr)
                    {
                        stc->addGameObject(value);
                    }
                }

                oldspellid = spellid;
            }
        } while (result->NextRow());
    }

    delete result;

    LogNotice("ObjectMgr : Loaded constraints for %u spells...", static_cast<uint32_t>(m_spelltargetconstraints.size()));
}

SpellTargetConstraint* ObjectMgr::GetSpellTargetConstraintForSpell(uint32 spellid)
{
    SpellTargetConstraintMap::const_iterator itr = m_spelltargetconstraints.find(spellid);

    if (itr != m_spelltargetconstraints.end())
        return itr->second;
    else
        return nullptr;
}

uint32 ObjectMgr::GenerateArenaTeamId()
{
    uint32 ret;
    ret = ++m_hiArenaTeamId;

    return ret;
}

uint32 ObjectMgr::GenerateGroupId()
{
    uint32 r;
    r = ++m_hiGroupId;

    return r;
}

uint32 ObjectMgr::GenerateGuildId()
{
    uint32 r;
    r = ++m_hiGuildId;

    return r;
}

uint32 ObjectMgr::GenerateCreatureSpawnID()
{
    uint32 r;
    r = ++m_hiCreatureSpawnId;

    return r;
}

uint32 ObjectMgr::GenerateGameObjectSpawnID()
{
    uint32 r;
    r = ++m_hiGameObjectSpawnId;

    return r;
}

void ObjectMgr::AddPlayerCache(uint32 guid, PlayerCache* cache)
{
    m_playerCacheLock.Acquire();
    PlayerCacheMap::iterator itr = m_playerCache.find(guid);
    if (itr != m_playerCache.end())
    {
        itr->second = cache;
    }
    else
    {
        m_playerCache.insert(std::make_pair(guid, cache));
    }

    m_playerCacheLock.Release();
}

void ObjectMgr::RemovePlayerCache(uint32 guid)
{
    m_playerCacheLock.Acquire();
    PlayerCacheMap::iterator itr = m_playerCache.find(guid);
    if (itr != m_playerCache.end())
    {
        m_playerCache.erase(itr);
    }

    m_playerCacheLock.Release();
}

PlayerCache* ObjectMgr::GetPlayerCache(uint32 guid)
{
    m_playerCacheLock.Acquire();
    PlayerCacheMap::iterator itr = m_playerCache.find(guid);
    if (itr != m_playerCache.end())
    {
        PlayerCache* ret = itr->second;
        m_playerCacheLock.Release();
        return ret;
    }
    m_playerCacheLock.Release();

    return nullptr;
}

PlayerCache* ObjectMgr::GetPlayerCache(const char* name, bool caseSensitive /*= true*/)
{
    PlayerCache* ret = nullptr;
    m_playerCacheLock.Acquire();
    PlayerCacheMap::iterator itr;

    if (!caseSensitive)
    {
        std::string strName = name;
        Util::StringToLowerCase(strName);
        for (itr = m_playerCache.begin(); itr != m_playerCache.end(); ++itr)
        {
            std::string cachename;
            itr->second->GetStringValue(CACHE_PLAYER_NAME, cachename);
            if (!stricmp(cachename.c_str(), strName.c_str()))
            {
                ret = itr->second;
                break;
            }
        }
    }
    else
    {
        for (itr = m_playerCache.begin(); itr != m_playerCache.end(); ++itr)
        {
            std::string cachename;
            itr->second->GetStringValue(CACHE_PLAYER_NAME, cachename);
            if (!strcmp(cachename.c_str(), name))
            {
                ret = itr->second;
                break;
            }
        }
    }

    m_playerCacheLock.Release();

    return ret;
}

void ObjectMgr::LoadVehicleAccessories()
{
    QueryResult* result = WorldDatabase.Query("SELECT creature_entry, accessory_entry, seat FROM vehicle_accessories;");
    if (result != nullptr)
    {
        do
        {
            Field* row = result->Fetch();
            VehicleAccessoryEntry* entry = new VehicleAccessoryEntry();
            uint32 creature_entry = row[0].GetUInt32();
            entry->accessory_entry = row[1].GetUInt32();
            entry->seat = row[2].GetUInt32();

            std::map< uint32, std::vector< VehicleAccessoryEntry* >* >::iterator itr = vehicle_accessories.find(creature_entry);

            if (itr != vehicle_accessories.end())
            {
                itr->second->push_back(entry);
            }
            else
            {
                std::vector< VehicleAccessoryEntry* >* v = new std::vector< VehicleAccessoryEntry* >();
                v->push_back(entry);
                vehicle_accessories.insert(std::make_pair(creature_entry, v));
            }

        } while (result->NextRow());

        delete result;
    }
}

std::vector< VehicleAccessoryEntry* >* ObjectMgr::GetVehicleAccessories(uint32 creature_entry)
{
    std::map< uint32, std::vector< VehicleAccessoryEntry* >* >::iterator itr = vehicle_accessories.find(creature_entry);

    if (itr == vehicle_accessories.end())
        return nullptr;
    else
        return itr->second;
}

void ObjectMgr::LoadWorldStateTemplates()
{
    QueryResult* result = WorldDatabase.QueryNA("SELECT DISTINCT map FROM worldstate_templates ORDER BY map;");
    if (result == nullptr)
        return;

    do
    {
        Field* row = result->Fetch();
        uint32 mapid = row[0].GetUInt32();

        worldstate_templates.insert(std::make_pair(mapid, new std::multimap< uint32, WorldState >()));

    } while (result->NextRow());

    delete result;

    result = WorldDatabase.QueryNA("SELECT map, zone, field, value FROM worldstate_templates;");
    if (result == nullptr)
        return;

    do
    {
        Field* row = result->Fetch();
        WorldState ws;

        uint32 mapid = row[0].GetUInt32();
        uint32 zone = row[1].GetUInt32();
        ws.field = row[2].GetUInt32();
        ws.value = row[3].GetUInt32();

        std::map< uint32, std::multimap< uint32, WorldState >* >::iterator itr = worldstate_templates.find(mapid);
        if (itr == worldstate_templates.end())
            continue;

        itr->second->insert(std::make_pair(zone, ws));

    } while (result->NextRow());

    delete result;
}

std::multimap< uint32, WorldState >* ObjectMgr::GetWorldStatesForMap(uint32 map) const
{
    std::map< uint32, std::multimap< uint32, WorldState >* >::const_iterator itr = worldstate_templates.find(map);

    if (itr == worldstate_templates.end())
        return nullptr;
    else
        return itr->second;
}

void ObjectMgr::LoadEventScripts()
{
    LogNotice("ObjectMgr : Loading Event Scripts...");

    bool success = false;
    const char* eventScriptsQuery = "SELECT `event_id`, `function`, `script_type`, `data_1`, `data_2`, `data_3`, `data_4`, `data_5`, `x`, `y`, `z`, `o`, `delay`, `next_event` FROM `event_scripts` WHERE `event_id` > 0 ORDER BY `event_id`";
    auto result = WorldDatabase.Query(&success, eventScriptsQuery);

    if (!success)
    {
        LogDebugFlag(LF_DB_TABLES, "LoadEventScripts : Failed on Loading Queries from event_scripts.");
        return;
    }
    else
    {
        if (!result)
        {
            LogDebugFlag(LF_DB_TABLES, "LoadEventScripts : Loaded 0 event_scripts. DB table `event_scripts` is empty.");
            return;
        }
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 event_id = fields[0].GetUInt32();
        SimpleEventScript eventscript;

        eventscript.eventId     = event_id;
        eventscript.function    = static_cast<uint8>(ScriptCommands(fields[1].GetUInt8()));
        eventscript.scripttype  = static_cast<uint8>(EasyScriptTypes(fields[2].GetUInt8()));
        eventscript.data_1      = fields[3].GetUInt32();
        eventscript.data_2      = fields[4].GetUInt32();
        eventscript.data_3      = fields[5].GetUInt32();
        eventscript.data_4      = fields[6].GetUInt32();
        eventscript.data_5      = fields[7].GetUInt32();
        eventscript.x           = fields[8].GetUInt32();
        eventscript.y           = fields[9].GetUInt32();
        eventscript.z           = fields[10].GetUInt32();
        eventscript.o           = fields[11].GetUInt32();
        eventscript.delay       = fields[12].GetUInt32();
        eventscript.nextevent   = fields[13].GetUInt32();

        SimpleEventScript* SimpleEventScript = &mEventScriptMaps.insert(EventScriptMaps::value_type(event_id, eventscript))->second;

        // for search by spellid ( data_1 is spell id )
        if (eventscript.data_1 && eventscript.scripttype == static_cast<uint8>(EasyScriptTypes::SCRIPT_TYPE_SPELL_EFFECT))
            mSpellEffectMaps.insert(SpellEffectMaps::value_type(eventscript.data_1, SimpleEventScript));


        ++count;

    } while (result->NextRow());

    delete result;

    LogDetail("ObjectMgr : Loaded event_scripts for %u events...", count);
}

EventScriptBounds ObjectMgr::GetEventScripts(uint32 event_id) const
{
    return EventScriptBounds(mEventScriptMaps.lower_bound(event_id), mEventScriptMaps.upper_bound(event_id));
}

SpellEffectMapBounds ObjectMgr::GetSpellEffectBounds(uint32 data_1) const
{
    return SpellEffectMapBounds(mSpellEffectMaps.lower_bound(data_1), mSpellEffectMaps.upper_bound(data_1));
}

bool ObjectMgr::CheckforScripts(Player* plr, uint32 event_id)
{
    EventScriptBounds EventScript = sObjectMgr.GetEventScripts(event_id);
    if (EventScript.first == EventScript.second)
        return false;

    for (EventScriptMaps::const_iterator itr = EventScript.first; itr != EventScript.second; ++itr)
    {
        sEventMgr.AddEvent(this, &ObjectMgr::EventScriptsUpdate, plr, itr->second.eventId, EVENT_EVENT_SCRIPTS, itr->second.delay, 1, 0);
    }

    return true;
}

bool ObjectMgr::CheckforDummySpellScripts(Player* plr, uint32 data_1)
{
    SpellEffectMapBounds EventScript = sObjectMgr.GetSpellEffectBounds(data_1);
    if (EventScript.first == EventScript.second)
        return false;

    for (SpellEffectMaps::const_iterator itr = EventScript.first; itr != EventScript.second; ++itr)
    {
        sEventMgr.AddEvent(this, &ObjectMgr::EventScriptsUpdate, plr, itr->second->eventId, EVENT_EVENT_SCRIPTS, itr->second->delay, 1, 0);
    }

    return true;
}

void ObjectMgr::EventScriptsUpdate(Player* plr, uint32 next_event)
{
    EventScriptBounds EventScript = sObjectMgr.GetEventScripts(next_event);

    for (EventScriptMaps::const_iterator itr = EventScript.first; itr != EventScript.second; ++itr)
    {
        if (itr->second.scripttype == static_cast<uint8>(EasyScriptTypes::SCRIPT_TYPE_SPELL_EFFECT) || itr->second.scripttype == static_cast<uint8>(EasyScriptTypes::SCRIPT_TYPE_DUMMY))
        {
            switch (itr->second.function)
            {
            case static_cast<uint8>(ScriptCommands::SCRIPT_COMMAND_RESPAWN_GAMEOBJECT):
            {
                Object* target = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), itr->second.data_1);
                if (target == nullptr)
                    return;

                static_cast<GameObject*>(target)->Despawn(1000, itr->second.data_2);

                break;
            }

            case static_cast<uint8>(ScriptCommands::SCRIPT_COMMAND_KILL_CREDIT):
            {
                QuestLogEntry* pQuest = plr->GetQuestLogForEntry(itr->second.data_2);
                if (pQuest != nullptr)
                {
                    if (pQuest->getQuestProperties()->required_mob_or_go[itr->second.data_5] >= 0)
                    {
                        uint32 required_mob = static_cast<uint32>(pQuest->getQuestProperties()->required_mob_or_go[itr->second.data_5]);
                        if (pQuest->getMobCountByIndex(itr->second.data_5) < required_mob)
                        {
                            pQuest->setMobCountForIndex(itr->second.data_5, pQuest->getMobCountByIndex(itr->second.data_5) + 1);
                            pQuest->SendUpdateAddKill(itr->second.data_5);
                            pQuest->updatePlayerFields();
                        }
                    }
                }
                break;
            }
            }
        }

        if (itr->second.scripttype == static_cast<uint8>(EasyScriptTypes::SCRIPT_TYPE_GAMEOBJECT) || itr->second.scripttype == static_cast<uint8>(EasyScriptTypes::SCRIPT_TYPE_DUMMY))
        {
            switch (itr->second.function)
            {
            case static_cast<uint8>(ScriptCommands::SCRIPT_COMMAND_ACTIVATE_OBJECT):
            {
                if ((itr->second.x || itr->second.y || itr->second.z) == 0)
                {
                    Object* target = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), itr->second.data_1);
                    if (target == nullptr)
                        return;

                    if (static_cast<GameObject*>(target)->getState() != GO_STATE_OPEN)
                    {
                        static_cast<GameObject*>(target)->setState(GO_STATE_OPEN);
                    }
                    else
                    {
                        static_cast<GameObject*>(target)->setState(GO_STATE_CLOSED);
                    }
                }
                else
                {
                    Object* target = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(float(itr->second.x), float(itr->second.y), float(itr->second.z), itr->second.data_1);
                    if (target == nullptr)
                        return;

                    if (static_cast<GameObject*>(target)->getState() != GO_STATE_OPEN)
                    {
                        static_cast<GameObject*>(target)->setState(GO_STATE_OPEN);
                    }
                    else
                    {
                        static_cast<GameObject*>(target)->setState(GO_STATE_CLOSED);
                    }
                }
            }
            break;
            }
        }

        if (itr->second.nextevent != 0)
        {
            sObjectMgr.CheckforScripts(plr, itr->second.nextevent);
        }
    }
}



void ObjectMgr::LoadCreatureAIAgents()
{
    // Load AI Agents
    QueryResult* result = WorldDatabase.Query("SELECT * FROM ai_agents");
    if (result != nullptr)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            CreatureProperties const* cn = sMySQLStore.getCreatureProperties(entry);
            SpellInfo const* spe = sSpellMgr.getSpellInfo(fields[6].GetUInt32());

            if (spe == nullptr)
            {
                LogDebugFlag(LF_DB_TABLES, "AIAgent : For %u has nonexistent spell %u.", fields[0].GetUInt32(), fields[6].GetUInt32());
                continue;
            }

            if (!cn)
                continue;

            AI_Spell* sp = new AI_Spell;
            sp->entryId = fields[0].GetUInt32();
            sp->instance_mode = fields[1].GetUInt8();
            sp->agent = fields[2].GetUInt16();
            sp->procChance = fields[4].GetUInt32();
            sp->procCount = fields[5].GetUInt32();
            sp->spell = spe;
            sp->spellType = static_cast<uint8>(fields[7].GetUInt32());

            int32  targettype = fields[8].GetInt32();
            if (targettype == -1)
                sp->spelltargetType = static_cast<uint8>(spe->aiTargetType());
            else
                sp->spelltargetType = static_cast<uint8>(targettype);

            sp->cooldown = fields[9].GetInt32();
            sp->floatMisc1 = fields[10].GetFloat();
            sp->autocast_type = (uint32)-1;
            sp->cooldowntime = Util::getMSTime();
            sp->procCounter = 0;
            sp->Misc2 = fields[11].GetUInt32();
            if (sp->agent == AGENT_SPELL)
            {
                if (!sp->spell)
                {
                    LogDebugFlag(LF_DB_TABLES, "SpellId %u in ai_agent for %u is invalid.", (unsigned int)fields[6].GetUInt32(), (unsigned int)sp->entryId);
                    delete sp;
                    sp = nullptr;
                    continue;
                }

                if (sp->spell->getEffect(0) == SPELL_EFFECT_LEARN_SPELL || sp->spell->getEffect(1) == SPELL_EFFECT_LEARN_SPELL ||
                    sp->spell->getEffect(2) == SPELL_EFFECT_LEARN_SPELL)
                {
                    LogDebugFlag(LF_DB_TABLES, "Teaching spell %u in ai_agent for %u", (unsigned int)fields[6].GetUInt32(), (unsigned int)sp->entryId);
                    delete sp;
                    sp = nullptr;
                    continue;
                }

                sp->minrange = GetMinRange(sSpellRangeStore.LookupEntry(sp->spell->getRangeIndex()));
                sp->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(sp->spell->getRangeIndex()));

                //omg the poor darling has no clue about making ai_agents
                if (sp->cooldown == (uint32)-1)
                {
                    //now this will not be exact cooldown but maybe a bigger one to not make him spam spells to often
                    int cooldown;
                    auto spell_duration = sSpellDurationStore.LookupEntry(sp->spell->getDurationIndex());
                    int Dur = 0;
                    int Casttime = 0; //most of the time 0
                    int RecoveryTime = sp->spell->getRecoveryTime();
                    if (sp->spell->getDurationIndex())
                        Dur = ::GetDuration(spell_duration);
                    Casttime = GetCastTime(sSpellCastTimesStore.LookupEntry(sp->spell->getCastingTimeIndex()));
                    cooldown = Dur + Casttime + RecoveryTime;
                    if (cooldown < 0)
                        sp->cooldown = 2000; //huge value that should not loop while adding some timestamp to it
                    else sp->cooldown = cooldown;
                }
            }

            if (sp->agent == AGENT_RANGED)
            {
                const_cast<CreatureProperties*>(cn)->m_canRangedAttack = true;
                delete sp;
                sp = nullptr;
            }
            else if (sp->agent == AGENT_FLEE)
            {
                const_cast<CreatureProperties*>(cn)->m_canFlee = true;
                if (sp->floatMisc1)
                    const_cast<CreatureProperties*>(cn)->m_canFlee = (sp->floatMisc1 > 0.0f ? true : false);
                else
                    const_cast<CreatureProperties*>(cn)->m_fleeHealth = 0.2f;

                if (sp->Misc2)
                    const_cast<CreatureProperties*>(cn)->m_fleeDuration = sp->Misc2;
                else
                    const_cast<CreatureProperties*>(cn)->m_fleeDuration = 10000;

                delete sp;
                sp = nullptr;
            }
            else if (sp->agent == AGENT_CALLFORHELP)
            {
                const_cast<CreatureProperties*>(cn)->m_canCallForHelp = true;
                if (sp->floatMisc1)
                    const_cast<CreatureProperties*>(cn)->m_callForHelpHealth = 0.2f;

                delete sp;
                sp = nullptr;
            }
            else
            {
                const_cast<CreatureProperties*>(cn)->spells.push_back(sp);
            }

        } while (result->NextRow());

        delete result;
    }
}

void ObjectMgr::LoadInstanceEncounters()
{
    const auto startTime = Util::TimeNow();

    //                                                 0         1            2                3               4       5
    QueryResult* result = WorldDatabase.Query("SELECT entry, creditType, creditEntry, lastEncounterDungeon, comment, mapid FROM instance_encounters");
    if (result == nullptr)
    {
        LogDebugFlag(LF_DB_TABLES, ">> Loaded 0 instance encounters, table is empty!");
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
            LogDebugFlag(LF_DB_TABLES, "Table `instance_encounters` has an invalid encounter id %u, skipped!", entry);
            continue;
        }

#if VERSION_STRING == WotLK
        dungeonEncounterName = dungeonEncounter->encounterName[0];
#else
        dungeonEncounterName = dungeonEncounter->encounterName;
#endif
#endif

        if (lastEncounterDungeon && sLfgMgr.GetLFGDungeon(lastEncounterDungeon) == 0)
        {
            LogDebugFlag(LF_DB_TABLES, "Table `instance_encounters` has an encounter %u (%s) marked as final for invalid dungeon id %u, skipped!", entry, dungeonEncounterName, lastEncounterDungeon);
            continue;
        }

#if VERSION_STRING >= WotLK
        if (lastEncounterDungeon)
        {
            const auto itr = dungeonLastBosses.find(lastEncounterDungeon);
            if (itr != dungeonLastBosses.end())
            {
#if VERSION_STRING == WotLK
                const auto itrEncounterName = itr->second->encounterName[0];
#else
                const auto itrEncounterName = itr->second->encounterName;
#endif
                LogDebugFlag(LF_DB_TABLES, "Table `instance_encounters` specified encounter %u (%s) as last encounter but %u (%s) is already marked as one, skipped!", entry, dungeonEncounterName, itr->second->id, itrEncounterName);
                continue;
            }

            dungeonLastBosses[lastEncounterDungeon] = dungeonEncounter;
        }
#endif

        switch (creditType)
        {
            case ENCOUNTER_CREDIT_KILL_CREATURE:
            {
                auto creatureprop = sMySQLStore.getCreatureProperties(creditEntry);
                if (creatureprop == nullptr)
                {
                    LogDebugFlag(LF_DB_TABLES, "Table `instance_encounters` has an invalid creature (entry %u) linked to the encounter %u (%s), skipped!", creditEntry, entry, dungeonEncounterName);
                    continue;
                }
                const_cast<CreatureProperties*>(creatureprop)->extra_a9_flags |= 0x10000000; // Flagged Dungeon Boss
                break;
            }
            case ENCOUNTER_CREDIT_CAST_SPELL:
            {
                if (sSpellMgr.getSpellInfo(creditEntry) == nullptr)
                {
                    LogDebugFlag(LF_DB_TABLES, "Table `instance_encounters` has an invalid spell (entry %u) linked to the encounter %u (%s), skipped!", creditEntry, entry, dungeonEncounterName);
                    continue;
                }
                break;
            }
            default:
            {
                LogDebugFlag(LF_DB_TABLES, "Table `instance_encounters` has an invalid credit type (%u) for encounter %u (%s), skipped!", creditType, entry, dungeonEncounterName);
                continue;
            }
        }

#if VERSION_STRING <= TBC
        DungeonEncounterList& encounters = _dungeonEncounterStore[mapId];
        encounters.push_back(new DungeonEncounter(EncounterCreditType(creditType), creditEntry));
#else
        DungeonEncounterList& encounters = _dungeonEncounterStore[static_cast<int32_t>(static_cast<uint16_t>(dungeonEncounter->mapId) | (static_cast<uint32_t>(dungeonEncounter->difficulty) << 16))];
        encounters.push_back(new DungeonEncounter(dungeonEncounter, EncounterCreditType(creditType), creditEntry, lastEncounterDungeon));
#endif
        ++count;
    } while (result->NextRow());

    LogDetail("ObjectMgr : Loaded %u instance encounters in %u ms", count, static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
}
