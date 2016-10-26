/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2016 AscEmu Team <http://www.ascemu.org/>
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
#include <Exceptions/Exceptions.hpp>

initialiseSingleton(ObjectMgr);

const char* NormalTalkMessage = "DMSG";

ObjectMgr::ObjectMgr() :
m_hiItemGuid(0),
m_hiGroupId(0),
m_mailid(0),
m_reportID(0),
m_ticketid(0),
m_setGUID(0),
m_hiCorpseGuid(0),
m_hiGuildId(0),
m_hiPetGuid(0),
m_hiArenaTeamId(0),
TransportersCount(0),
m_hiPlayerGuid(1)
{
    memset(m_InstanceBossInfoMap, 0, sizeof(InstanceBossInfoMap*) * NUM_MAPS);
}


ObjectMgr::~ObjectMgr()
{
    Log.Notice("ObjectMgr", "Deleting Corpses...");
    CorpseCollectorUnload();

    Log.Notice("ObjectMgr", "Deleting Guilds...");
    for (GuildMap::iterator i = mGuild.begin(); i != mGuild.end(); ++i)
    {
        delete i->second;
    }

    Log.Notice("ObjectMgr", "Deleting Vendors...");
    for (VendorMap::iterator i = mVendors.begin(); i != mVendors.end(); ++i)
    {
        delete i->second;
    }

    Log.Notice("ObjectMgr", "Deleting Trainers...");
    for (TrainerMap::iterator i = mTrainers.begin(); i != mTrainers.end(); ++i)
    {
        Trainer* t = i->second;
        if (t->UIMessage && t->UIMessage != (char*)NormalTalkMessage)
            delete[] t->UIMessage;
        delete t;
    }

    Log.Notice("ObjectMgr", "Deleting Level Information...");
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

    Log.Notice("ObjectMgr", "Deleting Waypoint Cache...");
    for (std::unordered_map<uint32, Movement::WayPointMap*>::iterator i = m_waypoints.begin(); i != m_waypoints.end(); ++i)
    {
        for (Movement::WayPointMap::iterator i2 = i->second->begin(); i2 != i->second->end(); ++i2)
            if ((*i2))
                delete(*i2);

        delete i->second;
    }

    Log.Notice("ObjectMgr", "Deleting timed emote Cache...");
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

    Log.Notice("ObjectMgr", "Deleting NPC Say Texts...");
    for (uint8 i = 0; i < NUM_MONSTER_SAY_EVENTS; ++i)
    {
        NpcMonsterSay* p;
        for (MonsterSayMap::iterator itr = mMonsterSays[i].begin(); itr != mMonsterSays[i].end(); ++itr)
        {
            p = itr->second;
            for (uint32 j = 0; j < p->TextCount; ++j)
                free((char*)p->Texts[j]);
            delete[] p->Texts;
            free((char*)p->MonsterName);
            delete p;
        }

        mMonsterSays[i].clear();
    }

    Log.Notice("ObjectMgr", "Deleting Charters...");
    for (uint8 i = 0; i < NUM_CHARTER_TYPES; ++i)
    {
        for (std::unordered_map<uint32, Charter*>::iterator itr = m_charters[i].begin(); itr != m_charters[i].end(); ++itr)
        {
            delete itr->second;
        }
    }

    Log.Notice("ObjectMgr", "Deleting Reputation Tables...");
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

    Log.Notice("ObjectMgr", "Deleting Groups...");
    for (GroupMap::iterator itr = m_groups.begin(); itr != m_groups.end();)
    {
        Group* pGroup = itr->second;
        ++itr;
        if (pGroup != nullptr)
        {
            for (uint32 i = 0; i < pGroup->GetSubGroupCount(); ++i)
            {
                SubGroup* pSubGroup = pGroup->GetSubGroup(i);
                if (pSubGroup != NULL)
                {
                    pSubGroup->Disband();
                }
            }
            delete pGroup;
        }
    }

    Log.Notice("ObjectMgr", "Deleting Player Information...");
    for (std::unordered_map<uint32, PlayerInfo*>::iterator itr = m_playersinfo.begin(); itr != m_playersinfo.end(); ++itr)
    {
        itr->second->m_Group = NULL;
        free(itr->second->name);
        delete itr->second;
    }

    Log.Notice("ObjectMgr", "Deleting GM Tickets...");
    for (GmTicketList::iterator itr = GM_TicketList.begin(); itr != GM_TicketList.end(); ++itr)
        delete(*itr);

    Log.Notice("ObjectMgr", "Deleting Boss Information...");
    for (uint32 i = 0; i < NUM_MAPS; i++)
    {
        if (this->m_InstanceBossInfoMap[i] != NULL)
        {
            for (InstanceBossInfoMap::iterator itr = this->m_InstanceBossInfoMap[i]->begin(); itr != m_InstanceBossInfoMap[i]->end(); ++itr)
                delete(*itr).second;
            delete this->m_InstanceBossInfoMap[i];
            this->m_InstanceBossInfoMap[i] = NULL;
        }
    }

    Log.Notice("ObjectMgr", "Deleting Arena Teams...");
    for (std::unordered_map<uint32, ArenaTeam*>::iterator itr = m_arenaTeams.begin(); itr != m_arenaTeams.end(); ++itr)
    {
        delete(*itr).second;
    }

    Log.Notice("ObjectMgr", "Deleting Profession Discoveries...");
    std::set<ProfessionDiscovery*>::iterator itr = ProfessionDiscoveryTable.begin();
    for (; itr != ProfessionDiscoveryTable.end(); ++itr)
        delete(*itr);

    Log.Notice("ObjectMgr", "Cleaning up BroadCastStorages...");
    m_BCEntryStorage.clear();

    Log.Notice("ObjectMgr", "Cleaning up spell target constraints...");
    for (SpellTargetConstraintMap::iterator itr = m_spelltargetconstraints.begin(); itr != m_spelltargetconstraints.end(); ++itr)
        delete itr->second;

    m_spelltargetconstraints.clear();

    Log.Notice("ObjectMgr", "Cleaning up vehicle accessories...");
    for (std::map< uint32, std::vector< VehicleAccessoryEntry* >* >::iterator itr = vehicle_accessories.begin(); itr != vehicle_accessories.end(); ++itr)
    {
        std::vector< VehicleAccessoryEntry* > *v = itr->second;

        for (std::vector< VehicleAccessoryEntry* >::iterator itr2 = v->begin(); itr2 != v->end(); ++itr2)
            delete *itr2;
        v->clear();

        delete v;
    }

    vehicle_accessories.clear();


    Log.Notice("ObjectMgr", "Cleaning up worldstate templates...");
    for (std::map< uint32, std::multimap< uint32, WorldState >* >::iterator itr = worldstate_templates.begin(); itr != worldstate_templates.end(); ++itr)
    {
        itr->second->clear();
        delete itr->second;
    }

    worldstate_templates.clear();

    Log.Notice("ObjectMgr", "Clearing up areatrigger data");
    _areaTriggerStore.clear();

    Log.Notice("ObjectMgr", "Clearing up event scripts...");
    mEventScriptMaps.clear();
    mSpellEffectMaps.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Groups
Group* ObjectMgr::GetGroupByLeader(Player* pPlayer)
{
    GroupMap::iterator itr;
    Group* ret = NULL;
    m_groupLock.AcquireReadLock();
    for (itr = m_groups.begin(); itr != m_groups.end(); ++itr)
    {
        if (itr->second->GetLeader() == pPlayer->getPlayerInfo())
        {
            ret = itr->second;
            break;
        }
    }

    m_groupLock.ReleaseReadLock();
    return ret;
}

Group* ObjectMgr::GetGroupById(uint32 id)
{
    GroupMap::iterator itr;
    Group* ret = NULL;
    m_groupLock.AcquireReadLock();
    itr = m_groups.find(id);
    if (itr != m_groups.end())
        ret = itr->second;

    m_groupLock.ReleaseReadLock();
    return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Player names
void ObjectMgr::DeletePlayerInfo(uint32 guid)
{
    PlayerInfo* pl;
    std::unordered_map<uint32, PlayerInfo*>::iterator i;
    PlayerNameStringIndexMap::iterator i2;
    playernamelock.AcquireWriteLock();
    i = m_playersinfo.find(guid);
    if (i == m_playersinfo.end())
    {
        playernamelock.ReleaseWriteLock();
        return;
    }

    pl = i->second;
    if (pl->m_Group)
    {
        pl->m_Group->RemovePlayer(pl);
    }

    if (pl->guild)
    {
        if (pl->guild->GetGuildLeader() == pl->guid)
            pl->guild->Disband();
        else
            pl->guild->RemoveGuildMember(pl, NULL);
    }

    std::string pnam = std::string(pl->name);
    arcemu_TOLOWER(pnam);
    i2 = m_playersInfoByName.find(pnam);
    if (i2 != m_playersInfoByName.end() && i2->second == pl)
        m_playersInfoByName.erase(i2);

    free(pl->name);
    delete i->second;
    m_playersinfo.erase(i);

    playernamelock.ReleaseWriteLock();
}

PlayerInfo* ObjectMgr::GetPlayerInfo(uint32 guid)
{
    std::unordered_map<uint32, PlayerInfo*>::iterator i;
    PlayerInfo* rv;
    playernamelock.AcquireReadLock();
    i = m_playersinfo.find(guid);
    if (i != m_playersinfo.end())
        rv = i->second;
    else
        rv = NULL;
    playernamelock.ReleaseReadLock();
    return rv;
}

void ObjectMgr::AddPlayerInfo(PlayerInfo* pn)
{
    playernamelock.AcquireWriteLock();
    m_playersinfo[pn->guid] = pn;
    std::string pnam = std::string(pn->name);
    arcemu_TOLOWER(pnam);
    m_playersInfoByName[pnam] = pn;
    playernamelock.ReleaseWriteLock();
}

void ObjectMgr::RenamePlayerInfo(PlayerInfo* pn, const char* oldname, const char* newname)
{
    playernamelock.AcquireWriteLock();
    std::string oldn = std::string(oldname);
    arcemu_TOLOWER(oldn);

    PlayerNameStringIndexMap::iterator itr = m_playersInfoByName.find(oldn);
    if (itr != m_playersInfoByName.end() && itr->second == pn)
    {
        std::string newn = std::string(newname);
        arcemu_TOLOWER(newn);
        m_playersInfoByName.erase(itr);
        m_playersInfoByName[newn] = pn;
    }

    playernamelock.ReleaseWriteLock();
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
    Log.Success("ObjectMgr", "%u spell skills loaded.", mSpellSkills.size());
}

DBC::Structures::SkillLineAbilityEntry const* ObjectMgr::GetSpellSkill(uint32 id)
{
    return mSpellSkills[id];
}

SpellEntry* ObjectMgr::GetNextSpellRank(SpellEntry* sp, uint32 level)
{
    // Looks for next spell rank
    if (sp == nullptr)
        return NULL;

    auto skill_line_ability = GetSpellSkill(sp->Id);
    if (skill_line_ability != nullptr && skill_line_ability->next > 0)
    {
        SpellEntry* sp1 = dbcSpell.LookupEntry(skill_line_ability->next);
        if (sp1 && sp1->baseLevel <= level)   // check level
            return GetNextSpellRank(sp1, level);   // recursive for higher ranks
    }
    return sp;
}

void ObjectMgr::LoadPlayersInfo()
{
    QueryResult* result = CharacterDatabase.Query("SELECT guid, name, race, class, level, gender, zoneid, timestamp, acct FROM characters");
    uint32 c = 0;
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
            pn->m_Group = 0;
            pn->subGroup = 0;
            pn->m_loggedInPlayer = NULL;
            pn->guild = NULL;
            pn->guildRank = NULL;
            pn->guildMember = NULL;

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
                    if (mode >= NUM_INSTANCE_MODES || mapId >= NUM_MAPS)
                        continue;

                    pn->savedInstanceIdsLock.Acquire();
                    itr = pn->savedInstanceIds[mode].find(mapId);
                    if (itr == pn->savedInstanceIds[mode].end())
                        pn->savedInstanceIds[mode].insert(PlayerInstanceMap::value_type(mapId, instanceId));
                    else
                        (*itr).second = instanceId;

                    ///\todo Instances not loaded yet ~.~
                    //if (!sInstanceMgr.InstanceExists(mapId, pn->m_savedInstanceIds[mapId][mode]))
                    //{
                    //    pn->m_savedInstanceIds[mapId][mode] = 0;
                    //    CharacterDatabase.Execute("DELETE FROM instanceids WHERE mapId = %u AND instanceId = %u AND mode = %u", mapId, instanceId, mode);
                    //}

                    pn->savedInstanceIdsLock.Release();
                }
                while (result2->NextRow());
                delete result2;
            }

            if (pn->race == RACE_HUMAN || pn->race == RACE_DWARF || pn->race == RACE_GNOME || pn->race == RACE_NIGHTELF || pn->race == RACE_DRAENEI)
                pn->team = 0;
            else
                pn->team = 1;

            if (GetPlayerInfoByName(pn->name) != NULL)
            {
                // gotta rename him
                char temp[300];
                snprintf(temp, 300, "%s__%X__", pn->name, pn->guid);
                Log.Notice("ObjectMgr", "Renaming duplicate player %s to %s. (%u)", pn->name, temp, pn->guid);
                CharacterDatabase.WaitExecute("UPDATE characters SET name = '%s', login_flags = %u WHERE guid = %u",
                                              CharacterDatabase.EscapeString(std::string(temp)).c_str(), (uint32)LOGIN_FORCED_RENAME, pn->guid);


                free(pn->name);
                pn->name = strdup(temp);
            }

            std::string lpn = std::string(pn->name);
            arcemu_TOLOWER(lpn);
            m_playersInfoByName[lpn] = pn;

            //this is startup -> no need in lock -> don't use addplayerinfo
            m_playersinfo[pn->guid] = pn;

        }
        while (result->NextRow());
        delete result;
    }
    Log.Success("ObjectMgr", "%u players loaded.", m_playersinfo.size());
    LoadGuilds();
}

PlayerInfo* ObjectMgr::GetPlayerInfoByName(const char* name)
{
    std::string lpn = std::string(name);
    arcemu_TOLOWER(lpn);
    PlayerNameStringIndexMap::iterator i;
    PlayerInfo* rv = NULL;
    playernamelock.AcquireReadLock();

    i = m_playersInfoByName.find(lpn);
    if (i != m_playersInfoByName.end())
        rv = i->second;

    playernamelock.ReleaseReadLock();
    return rv;
}

#ifdef ENABLE_ACHIEVEMENTS
void ObjectMgr::LoadCompletedAchievements()
{
    QueryResult* result = CharacterDatabase.Query("SELECT achievement FROM character_achievement GROUP BY achievement");

    if (!result)
    {
        Log.Error("MySQL", "Query failed: SELECT achievement FROM character_achievement");
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


//////////////////////////////////////////////////////////////////////////////////////////
// DK:LoadGuilds()
void ObjectMgr::LoadGuilds()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM guilds");
    if (result)
    {
        do
        {
            Guild* pGuild = Guild::Create();
            if (!pGuild->LoadFromDB(result->Fetch()))
            {
                delete pGuild;
            }
            else
                mGuild.insert(std::make_pair(pGuild->GetGuildId(), pGuild));
        }
        while (result->NextRow());
        delete result;
    }
    Log.Success("ObjectMgr", "%u guilds loaded.", mGuild.size());
}

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
    pCorpse->LoadValues(fields[7].GetString());
    if (pCorpse->GetDisplayId() == 0)
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
        if (GET_LOWGUID_PART(itr->second->GetOwner()) == ownerguid)
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
    Corpse* c = this->GetCorpseByOwner(pOwner->GetLowGUID());
    if (!c)
        return;
    sEventMgr.AddEvent(c, &Corpse::Delink, EVENT_CORPSE_SPAWN_BONES, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
    CorpseAddEventDespawn(c);
}

void ObjectMgr::LoadGMTickets()
{
    QueryResult* result = CharacterDatabase.Query("SELECT ticketid, playerGuid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment FROM gm_tickets WHERE deleted = false");
    if (result == 0)
        return;

    uint32 deleted = 0;

    do
    {
        Field* fields = result->Fetch();

        GM_Ticket* ticket = new GM_Ticket;
        ticket->guid = fields[0].GetUInt64();
        ticket->playerGuid = fields[1].GetUInt64();
        ticket->name = fields[2].GetString();
        ticket->level = fields[3].GetUInt32();
        ticket->map = fields[4].GetUInt32();
        ticket->posX = fields[5].GetFloat();
        ticket->posY = fields[6].GetFloat();
        ticket->posZ = fields[7].GetFloat();
        ticket->message = fields[8].GetString();
        ticket->timestamp = fields[9].GetUInt32();

        deleted = fields[10].GetUInt32();

        if (deleted == 1)
            ticket->deleted = true;
        else
            ticket->deleted = false;

        ticket->assignedToPlayer = fields[11].GetUInt64();
        ticket->comment = fields[12].GetString();

        AddGMTicket(ticket, true);

    }
    while (result->NextRow());

    Log.Success("ObjectMgr", "%u active GM Tickets loaded.", result->GetRowCount());
    delete result;
}

void ObjectMgr::LoadInstanceBossInfos()
{
    char* p, *q, *trash;
    QueryResult* result = WorldDatabase.Query("SELECT mapid, creatureid, trash, trash_respawn_override FROM instance_bosses");
    if (result == NULL)
        return;

    uint32 cnt = 0;
    do
    {
        InstanceBossInfo* bossInfo = new InstanceBossInfo();
        bossInfo->mapid = (uint32)result->Fetch()[0].GetUInt32();

        MapInfo const* mapInfo = sMySQLStore.GetWorldMapInfo(bossInfo->mapid);
        if (mapInfo == NULL || mapInfo->type == INSTANCE_NULL)
        {
            LOG_DETAIL("Not loading boss information for map %u! (continent or unknown map)", bossInfo->mapid);
            delete bossInfo;
            continue;
        }
        if (bossInfo->mapid >= NUM_MAPS)
        {
            LOG_DETAIL("Not loading boss information for map %u! (map id out of range)", bossInfo->mapid);
            delete bossInfo;
            continue;
        }

        bossInfo->creatureid = (uint32)result->Fetch()[1].GetUInt32();
        trash = strdup(result->Fetch()[2].GetString());
        q = trash;
        p = strchr(q, ' ');
        while (p)
        {
            *p = 0;
            uint32 val = atoi(q);
            if (val)
                bossInfo->trash.insert(val);
            q = p + 1;
            p = strchr(q, ' ');
        }
        free(trash);
        bossInfo->trashRespawnOverride = (uint32)result->Fetch()[3].GetUInt32();


        if (this->m_InstanceBossInfoMap[bossInfo->mapid] == NULL)
            this->m_InstanceBossInfoMap[bossInfo->mapid] = new InstanceBossInfoMap;
        this->m_InstanceBossInfoMap[bossInfo->mapid]->insert(InstanceBossInfoMap::value_type(bossInfo->creatureid, bossInfo));
        ++cnt;
    }
    while (result->NextRow());

    delete result;
    Log.Success("ObjectMgr", "%u boss information loaded.", cnt);
}

void ObjectMgr::SaveGMTicket(GM_Ticket* ticket, QueryBuffer* buf)
{
    std::stringstream ss;

    ss << "DELETE FROM gm_tickets WHERE ticketid = ";
    ss << ticket->guid;
    ss << ";";

    if (buf == NULL)
        CharacterDatabase.ExecuteNA(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO gm_tickets (ticketid, playerguid, name, level, map, posX, posY, posZ, message, timestamp, deleted, assignedto, comment) VALUES(";
    ss << ticket->guid << ", ";
    ss << ticket->playerGuid << ", '";
    ss << CharacterDatabase.EscapeString(ticket->name) << "', ";
    ss << ticket->level << ", ";
    ss << ticket->map << ", ";
    ss << ticket->posX << ", ";
    ss << ticket->posY << ", ";
    ss << ticket->posZ << ", '";
    ss << CharacterDatabase.EscapeString(ticket->message) << "', ";
    ss << ticket->timestamp << ", ";

    if (ticket->deleted)
        ss << uint32(1);
    else
        ss << uint32(0);
    ss << ",";

    ss << ticket->assignedToPlayer << ", '";
    ss << CharacterDatabase.EscapeString(ticket->comment) << "');";

    if (buf == NULL)
        CharacterDatabase.ExecuteNA(ss.str().c_str());
    else
        buf->AddQueryStr(ss.str());
}

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
            sLog.Error("ObjectMgr", "Achievement reward entry %u has wrong achievement, ignore", entry);
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
            sLog.Error("ObjectMgr", "achievement reward %u has wrong gender %u.", entry, reward.gender);

        bool dup = false;
        AchievementRewardsMapBounds bounds = AchievementRewards.equal_range(entry);
        for (AchievementRewardsMap::const_iterator iter = bounds.first; iter != bounds.second; ++iter)
        {
            if (iter->second.gender == 2 || reward.gender == 2)
            {
                dup = true;
                sLog.Error("ObjectMgr", "Achievement reward %u must have single GENDER_NONE (%u), ignore duplicate case", 2, entry);
                break;
            }
        }

        if (dup)
            continue;

        // must be title or mail at least
        if (!reward.titel_A && !reward.titel_H && !reward.sender)
        {
            sLog.Error("ObjectMgr", "achievement_reward %u not have title or item reward data, ignore.", entry);
            continue;
        }

        if (reward.titel_A)
        {
            auto const* char_title_entry = sCharTitlesStore.LookupEntry(reward.titel_A);
            if (!char_title_entry)
            {
                sLog.Error("ObjectMgr", "achievement_reward %u has invalid title id (%u) in `title_A`, set to 0", entry, reward.titel_A);
                reward.titel_A = 0;
            }
        }

        if (reward.titel_H)
        {
            auto const* char_title_entry = sCharTitlesStore.LookupEntry(reward.titel_H);
            if (!char_title_entry)
            {
                sLog.Error("ObjectMgr", "achievement_reward %u has invalid title id (%u) in `title_A`, set to 0", entry, reward.titel_H);
                reward.titel_H = 0;
            }
        }

        //check mail data before item for report including wrong item case
        if (reward.sender)
        {
            if (!sMySQLStore.GetCreatureProperties(reward.sender))
            {
                sLog.Error("ObjectMgr", "achievement_reward %u has invalid creature entry %u as sender, mail reward skipped.", entry, reward.sender);
                reward.sender = 0;
            }
        }
        else
        {
            if (reward.itemId)
                sLog.Error("ObjectMgr", "achievement_reward %u not have sender data but have item reward, item will not rewarded", entry);

            if (!reward.subject.empty())
                sLog.Error("ObjectMgr", "achievement_reward %u not have sender data but have mail subject.", entry);

            if (!reward.text.empty())
                sLog.Error("ObjectMgr", "achievement_reward %u not have sender data but have mail text.", entry);
        }

        if (reward.itemId)
        {
            if (reward.itemId == 0)
            {
                sLog.Error("ObjectMgr", "achievement_reward %u has invalid item id %u, reward mail will be without item.", entry, reward.itemId);
            }
        }

        AchievementRewards.insert(AchievementRewardsMap::value_type(entry, reward));
        ++count;

    }
    while (result->NextRow());

    delete result;

    sLog.Success("ObjectMgr", "Loaded %u achievement rewards", count);
}

void ObjectMgr::SetHighestGuids()
{
    QueryResult* result = CharacterDatabase.Query("SELECT MAX(guid) FROM characters");
    if (result)
    {
        m_hiPlayerGuid.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM playeritems");
    if (result)
    {
        m_hiItemGuid.SetVal((uint32)result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guid) FROM corpses");
    if (result)
    {
        m_hiCorpseGuid.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(id) FROM creature_spawns");
    if (result)
    {
        m_hiCreatureSpawnId.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = WorldDatabase.Query("SELECT MAX(id) FROM gameobject_spawns");
    if (result)
    {
        m_hiGameObjectSpawnId.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(group_id) FROM groups");
    if (result)
    {
        m_hiGroupId.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(charterid) FROM charters");
    if (result)
    {
        m_hiCharterId.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(guildid) FROM guilds");
    if (result)
    {
        m_hiGuildId.SetVal(result->Fetch()[0].GetUInt32());
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(UID) FROM playerbugreports");
    if (result != NULL)
    {
        m_reportID.SetVal(uint32(result->Fetch()[0].GetUInt64() + 1));
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(ticketid) FROM gm_tickets");
    if (result)
    {
        m_ticketid.SetVal(uint32(result->Fetch()[0].GetUInt64() + 1));
        delete result;
    }


    result = CharacterDatabase.Query("SELECT MAX(message_id) FROM mailbox");
    if (result)
    {
        m_mailid.SetVal(uint32(result->Fetch()[0].GetUInt64() + 1));
        delete result;
    }

    result = CharacterDatabase.Query("SELECT MAX(setGUID) FROM equipmentsets");
    if (result != NULL)
    {
        m_setGUID.SetVal(uint32(result->Fetch()[0].GetUInt32() + 1));
        delete result;
    }

    Log.Notice("ObjectMgr", "HighGuid(CORPSE) = %u", m_hiCorpseGuid.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(PLAYER) = %u", m_hiPlayerGuid.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(GAMEOBJ) = %u", m_hiGameObjectSpawnId.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(UNIT) = %u", m_hiCreatureSpawnId.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(ITEM) = %u", m_hiItemGuid.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(CONTAINER) = %u", m_hiItemGuid.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(GROUP) = %u", m_hiGroupId.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(CHARTER) = %u", m_hiCharterId.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(GUILD) = %u", m_hiGuildId.GetVal());
    Log.Notice("ObjectMgr", "HighGuid(BUGREPORT) = %u", uint32(m_reportID.GetVal() - 1));
    Log.Notice("ObjectMgr", "HighGuid(TICKET) = %u", uint32(m_ticketid.GetVal() - 1));
    Log.Notice("ObjectMgr", "HighGuid(MAIL) = %u", uint32(m_mailid.GetVal()));
    Log.Notice("ObjectMgr", "HighGuid(EQUIPMENTSET) = %u", uint32(m_setGUID.GetVal() - 1));
}

uint32 ObjectMgr::GenerateReportID()
{
    return ++m_reportID;
}

uint32 ObjectMgr::GenerateTicketID()
{
    return ++m_ticketid;
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
    Player* rv = NULL;
    PlayerStorageMap::const_iterator itr;
    _playerslock.AcquireReadLock();

    if (!caseSensitive)
    {
        std::string strName = name;
        arcemu_TOLOWER(strName);
        for (itr = _players.begin(); itr != _players.end(); ++itr)
        {
            if (!stricmp(itr->second->GetNameString()->c_str(), strName.c_str()))
            {
                rv = itr->second;
                break;
            }
        }
    }
    else
    {
        for (itr = _players.begin(); itr != _players.end(); ++itr)
        {
            if (!strcmp(itr->second->GetName(), name))
            {
                rv = itr->second;
                break;
            }
        }
    }

    _playerslock.ReleaseReadLock();

    return rv;
}

Player* ObjectMgr::GetPlayer(uint32 guid)
{
    Player* rv = nullptr;

    _playerslock.AcquireReadLock();
    PlayerStorageMap::const_iterator itr = _players.find(guid);
    rv = (itr != _players.end()) ? itr->second : nullptr;
    _playerslock.ReleaseReadLock();

    return rv;
}

void ObjectMgr::AddGuild(Guild* pGuild)
{
    ARCEMU_ASSERT(pGuild != NULL);
    mGuild[pGuild->GetGuildId()] = pGuild;
}

uint32 ObjectMgr::GetTotalGuildCount()
{
    return (uint32)mGuild.size();
}

bool ObjectMgr::RemoveGuild(uint32 guildId)
{
    GuildMap::iterator i = mGuild.find(guildId);
    if (i == mGuild.end())
    {
        return false;
    }

    mGuild.erase(i);
    return true;
}

Guild* ObjectMgr::GetGuild(uint32 guildId)
{
    GuildMap::const_iterator itr = mGuild.find(guildId);
    if (itr == mGuild.end())
        return NULL;
    return itr->second;
}

Guild* ObjectMgr::GetGuildByLeaderGuid(uint64 leaderGuid)
{
    GuildMap::const_iterator itr;
    for (itr = mGuild.begin(); itr != mGuild.end(); ++itr)
    {
        if (itr->second->GetGuildLeader() == leaderGuid)
            return itr->second;
    }
    return NULL;
}

Guild* ObjectMgr::GetGuildByGuildName(std::string guildName)
{
    GuildMap::const_iterator itr;
    for (itr = mGuild.begin(); itr != mGuild.end(); ++itr)
    {
        if (itr->second->GetGuildName() == guildName)
            return itr->second;
    }
    return NULL;
}


void ObjectMgr::AddGMTicket(GM_Ticket* ticket, bool startup)
{
    ARCEMU_ASSERT(ticket != NULL);
    GM_TicketList.push_back(ticket);

    if (!startup)
        SaveGMTicket(ticket, NULL);
}

void ObjectMgr::UpdateGMTicket(GM_Ticket* ticket)
{
    SaveGMTicket(ticket, NULL);
}

void ObjectMgr::DeleteGMTicketPermanently(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end();)
    {
        if ((*i)->guid == ticketGuid)
        {
            i = GM_TicketList.erase(i);
        }
        else
        {
            ++i;
        }
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE guid=%u", ticketGuid);      // kill from db
}

void ObjectMgr::DeleteAllRemovedGMTickets()
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end(); ++i)
    {
        if ((*i)->deleted)
        {
            i = GM_TicketList.erase(i);
        }
        else
        {
            ++i;
        }
    }

    CharacterDatabase.Execute("DELETE FROM gm_tickets WHERE deleted=1");
}

void ObjectMgr::RemoveGMTicketByPlayer(uint64 playerGuid)
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end(); ++i)
    {
        if ((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            SaveGMTicket((*i), NULL);
            break;
        }
    }
}

void ObjectMgr::RemoveGMTicket(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            SaveGMTicket((*i), NULL);
            break;
        }
    }
}

void ObjectMgr::CloseTicket(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid && !(*i)->deleted)
        {
            (*i)->deleted = true;
            break;
        }
    }
}

GM_Ticket* ObjectMgr::GetGMTicketByPlayer(uint64 playerGuid)
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end(); ++i)
    {
        if ((*i)->playerGuid == playerGuid && !(*i)->deleted)
        {
            return (*i);
        }
    }
    return NULL;
}

GM_Ticket* ObjectMgr::GetGMTicket(uint64 ticketGuid)
{
    for (GmTicketList::iterator i = GM_TicketList.begin(); i != GM_TicketList.end(); ++i)
    {
        if ((*i)->guid == ticketGuid)
        {
            return (*i);
        }
    }
    return NULL;
}

void ObjectMgr::LoadVendors()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM vendors");
    if (result != NULL)
    {
        std::unordered_map<uint32, std::vector<CreatureItem>*>::const_iterator itr;
        std::vector<CreatureItem> *items;

        if (result->GetFieldCount() < 6)
        {
            Log.Notice("ObjectMgr", "Invalid format in vendors (%u/6) columns, not enough data to proceed.", result->GetFieldCount());
            delete result;
            return;
        }
        else if (result->GetFieldCount() > 6)
        {
            Log.Notice("ObjectMgr", "Invalid format in vendors (%u/6) columns, loading anyway because we have enough data", result->GetFieldCount());
        }

        DBC::Structures::ItemExtendedCostEntry const* item_extended_cost = NULL;
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
                    Log.Error("LoadVendors", "Extendedcost for item %u references nonexistent EC %u", fields[1].GetUInt32(), fields[5].GetUInt32());
            }
            else
                item_extended_cost = NULL;
            itm.extended_cost = item_extended_cost;
            items->push_back(itm);
        }
        while (result->NextRow());

        delete result;
    }
    Log.Success("ObjectMgr", "%u vendors loaded.", mVendors.size());
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

void ObjectMgr::LoadAIThreatToSpellId()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM ai_threattospellid");

    if (!result)
    {
        return;
    }

    do
    {
        Field* fields = result->Fetch();
        SpellEntry* sp = dbcSpell.LookupEntryForced(fields[0].GetUInt32());
        if (sp != NULL)
        {
            sp->custom_ThreatForSpell = fields[1].GetUInt32();
            sp->custom_ThreatForSpellCoef = fields[2].GetFloat();
        }
        else
            Log.Error("AIThreatSpell", "Cannot apply to spell %u; spell is nonexistent.", fields[0].GetUInt32());

    }
    while (result->NextRow());

    delete result;
}

void ObjectMgr::LoadSpellEffectsOverride()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM spell_effects_override");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            uint32 seo_SpellId = f[0].GetUInt32();
            uint32 seo_EffectId = f[1].GetUInt32();
            uint32 seo_Disable = f[2].GetUInt32();
            uint32 seo_Effect = f[3].GetUInt32();
            uint32 seo_BasePoints = f[4].GetUInt32();
            uint32 seo_ApplyAuraName = f[5].GetUInt32();
            //uint32 seo_SpellGroupRelation = f[6].GetUInt32();
            uint32 seo_MiscValue = f[7].GetUInt32();
            uint32 seo_TriggerSpell = f[8].GetUInt32();
            uint32 seo_ImplicitTargetA = f[9].GetUInt32();
            uint32 seo_ImplicitTargetB = f[10].GetUInt32();
            uint32 seo_EffectCustomFlag = f[11].GetUInt32();

            if (seo_SpellId)
            {
                SpellEntry* sp = dbcSpell.LookupEntryForced(seo_SpellId);
                if (sp != NULL)
                {
                    if (seo_Disable)
                        sp->Effect[seo_EffectId] = SPELL_EFFECT_NULL;

                    if (seo_Effect)
                        sp->Effect[seo_EffectId] = seo_Effect;

                    if (seo_BasePoints)
                        sp->EffectBasePoints[seo_EffectId] = seo_BasePoints;

                    if (seo_ApplyAuraName)
                        sp->EffectApplyAuraName[seo_EffectId] = seo_ApplyAuraName;

                    //                    if (seo_SpellGroupRelation)
                    //                        sp->EffectSpellGroupRelation[seo_EffectId] = seo_SpellGroupRelation;

                    if (seo_MiscValue)
                        sp->EffectMiscValue[seo_EffectId] = seo_MiscValue;

                    if (seo_TriggerSpell)
                        sp->EffectTriggerSpell[seo_EffectId] = seo_TriggerSpell;

                    if (seo_ImplicitTargetA)
                        sp->EffectImplicitTargetA[seo_EffectId] = seo_ImplicitTargetA;

                    if (seo_ImplicitTargetB)
                        sp->EffectImplicitTargetB[seo_EffectId] = seo_ImplicitTargetB;

                    if (seo_EffectCustomFlag != 0)
                        sp->EffectCustomFlag[seo_Effect] = seo_EffectCustomFlag;
                }
                else
                {
                    Log.Error("ObjectMgr", "Tried to load a spell effect override for a nonexistant spell: %u", seo_SpellId);
                }
            }

        }
        while (result->NextRow());
        delete result;
    }
}

Item* ObjectMgr::CreateItem(uint32 entry, Player* owner)
{
    ItemProperties const* proto = sMySQLStore.GetItemProperties(entry);
    if (proto ==nullptr)
        return nullptr;

    if (proto->InventoryType == INVTYPE_BAG)
    {
        Container* pContainer = new Container(HIGHGUID_TYPE_CONTAINER, GenerateLowGuid(HIGHGUID_TYPE_CONTAINER));
        pContainer->Create(entry, owner);
        pContainer->SetStackCount(1);
        return pContainer;
    }
    else
    {
        Item* pItem = new Item;
        pItem->Init(HIGHGUID_TYPE_ITEM, GenerateLowGuid(HIGHGUID_TYPE_ITEM));
        pItem->Create(entry, owner);
        pItem->SetStackCount(1);

        if (owner != NULL)
        {
            uint32* played = owner->GetPlayedtime();
            pItem->SetCreationTime(played[1]);
        }

        return pItem;
    }
}

Item* ObjectMgr::LoadItem(uint32 lowguid)
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM playeritems WHERE guid = %u", lowguid);
    Item* pReturn = nullptr;

    if (result)
    {
        ItemProperties const* pProto = sMySQLStore.GetItemProperties(result->Fetch()[2].GetUInt32());
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
            pItem->Init(HIGHGUID_TYPE_ITEM, lowguid);
            pItem->LoadFromDB(result->Fetch(), NULL, false);
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
            pCorpse->LoadValues(fields[8].GetString());
            if (pCorpse->GetDisplayId() == 0)
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

#ifdef ENABLE_ACHIEVEMENTS
AchievementCriteriaEntryList const & ObjectMgr::GetAchievementCriteriaByType(AchievementCriteriaTypes type)
{
    return m_AchievementCriteriasByType[type];
}

void ObjectMgr::LoadAchievementCriteriaList()
{
    for (uint32 rowId = 0; rowId < sAchievementCriteriaStore.GetNumRows(); ++rowId)
    {
        auto criteria = sAchievementCriteriaStore.LookupEntry(rowId);
        if (!criteria)
            continue;

        m_AchievementCriteriasByType[criteria->requiredType].push_back(criteria);
    }
}
#endif

void ObjectMgr::CorpseAddEventDespawn(Corpse* pCorpse)
{
    if (!pCorpse->IsInWorld())
        delete pCorpse;
    else
        sEventMgr.AddEvent(pCorpse->GetMapMgr(), &MapMgr::EventCorpseDespawn, pCorpse->GetGUID(), EVENT_CORPSE_DESPAWN, 600000, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
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

void ObjectMgr::CreateGossipMenuForPlayer(GossipMenu** Location, uint64 Guid, uint32 TextID, Player* Plr)
{
    if (TextID == 0)
    {
        //TextID = 0 will not show the gossip to the player. Using "2" since it's the default value in GossipScript::GossipHello()
        LOG_ERROR("Object with GUID " I64FMT " is trying to create a GossipMenu with TextID == 0", Guid);
        TextID = 2;
    }

    GossipMenu* Menu = new GossipMenu(Guid, TextID);
    ARCEMU_ASSERT(Menu != NULL);

    if (Plr->CurrentGossipMenu != NULL)
        delete Plr->CurrentGossipMenu;

    Plr->CurrentGossipMenu = Menu;
    *Location = Menu;
}

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

        //now load the spells
        QueryResult* result2 = WorldDatabase.Query("SELECT * FROM trainer_spells where entry='%u'", entry);
        if (!result2)
        {
            Log.Error("LoadTrainers", "Trainer with no spells, entry %u.", entry);
            if (tr->UIMessage != NormalTalkMessage)
                delete[] tr->UIMessage;

            delete tr;
            continue;
        }
        if (result2->GetFieldCount() != 9)
        {
            Log.LargeErrorMessage("Trainers table format is invalid. Please update your database.", NULL);
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
                    ts.pCastSpell = dbcSpell.LookupEntryForced(CastSpellID);
                    if (ts.pCastSpell)
                    {
                        for (uint8 k = 0; k < 3; ++k)
                        {
                            if (ts.pCastSpell->Effect[k] == SPELL_EFFECT_LEARN_SPELL)
                            {
                                ts.pCastRealSpell = dbcSpell.LookupEntryForced(ts.pCastSpell->EffectTriggerSpell[k]);
                                if (ts.pCastRealSpell == NULL)
                                {
                                    Log.Error("Trainers", "Trainer %u contains cast spell %u that is non-teaching", entry, CastSpellID);
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
                    ts.pLearnSpell = dbcSpell.LookupEntryForced(LearnSpellID);
                }

                if (ts.pCastSpell == NULL && ts.pLearnSpell == NULL)
                {
                    Log.Error("LoadTrainers", "Trainer %u without valid spells (%u/%u).", entry, CastSpellID, LearnSpellID);
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
                if (ts.RequiredSkillLine == 0 && ts.pCastRealSpell != NULL && ts.pCastRealSpell->Effect[1] == SPELL_EFFECT_SKILL)
                {
                    uint32 skill = ts.pCastRealSpell->EffectMiscValue[1];
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
    Log.Success("ObjectMgr", "%u trainers loaded.", mTrainers.size());
}

Trainer* ObjectMgr::GetTrainer(uint32 Entry)
{
    TrainerMap::iterator iter = mTrainers.find(Entry);
    if (iter == mTrainers.end())
        return NULL;

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
        for (uint8 Race = RACE_HUMAN; Race <= RACE_DRAENEI; ++Race)
        {
            PlayerCreateInfo const* PCI = sMySQLStore.GetPlayerCreateInfo(static_cast<uint8>(Race), static_cast<uint8>(Class));

            if (PCI == nullptr)
                continue;   // Class not valid for this race.

            // Generate each level's information
            uint32 MaxLevel = sWorld.m_levelCap + 1;
            LevelInfo* lvl = 0, lastlvl;
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
                    case DEATHKNIGHT: // Based on 55-56 more testing will be done.
                        if (Level < 60)
                            TotalHealthGain += 92;
                        /*else if (Level <60) TotalHealthGain+=??;
                        else if (Level <70) TotalHealthGain+=??;*/
                        else
                            TotalHealthGain += 92;
                        break;
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
    Log.Notice("ObjectMgr", "%u level up information generated.", mLevelInfo.size());
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
            if (Level > sWorld.m_levelCap)
                Level = sWorld.m_levelCap;

            // Pull the level information from the second map.
            LevelMap::iterator it2 = itr->second->find(Level);
            if (it2 == itr->second->end())
            {
                Log.Error("ObjectMgr::GetLevelInfo", "No level information found for level %u!", Level);
                return nullptr;
            }

            return it2->second;
        }
    }

    return nullptr;
}

void ObjectMgr::LoadDefaultPetSpells()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM petdefaultspells");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            uint32 Entry = f[0].GetUInt32();
            uint32 spell = f[1].GetUInt32();
            SpellEntry* sp = dbcSpell.LookupEntryForced(spell);

            if (spell && Entry && sp)
            {
                PetDefaultSpellMap::iterator itr = mDefaultPetSpells.find(Entry);
                if (itr != mDefaultPetSpells.end())
                    itr->second.insert(sp);
                else
                {
                    std::set<SpellEntry*> s;
                    s.insert(sp);
                    mDefaultPetSpells[Entry] = s;
                }
            }
        }
        while (result->NextRow());
        delete result;
    }
}

std::set<SpellEntry*>* ObjectMgr::GetDefaultPetSpells(uint32 Entry)
{
    PetDefaultSpellMap::iterator itr = mDefaultPetSpells.find(Entry);
    if (itr == mDefaultPetSpells.end())
        return 0;

    return &(itr->second);
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

    SpellEntry* sp = dbcSpell.LookupEntry(SpellId);
    if (sp->RecoveryTime > sp->CategoryRecoveryTime)
        return sp->RecoveryTime;
    else
        return sp->CategoryRecoveryTime;
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
        else te->msg = NULL;
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

    Log.Notice("ObjectMgr", "%u timed emotes cached.", result->GetRowCount());
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
    else return NULL;
}

void ObjectMgr::LoadCreatureWaypoints()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM creature_waypoints");
    if (!result)return;

    do
    {
        Field* fields = result->Fetch();
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
        uint32 spawnid = fields[0].GetUInt32();
        i = m_waypoints.find(spawnid);
        if (i == m_waypoints.end())
        {
            Movement::WayPointMap* m = new Movement::WayPointMap;
            if (m->size() <= wp->id)
                m->resize(wp->id + 1);
            (*m)[wp->id] = wp;
            m_waypoints[spawnid] = m;
        }
        else
        {
            if (i->second->size() <= wp->id)
                i->second->resize(wp->id + 1);

            (*(i->second))[wp->id] = wp;
        }
    }
    while (result->NextRow());

    Log.Notice("ObjectMgr", "%u waypoints cached.", result->GetRowCount());
    delete result;
}

Movement::WayPointMap* ObjectMgr::GetWayPointMap(uint32 spawnid)
{
    std::unordered_map<uint32, Movement::WayPointMap*>::const_iterator i;
    i = m_waypoints.find(spawnid);
    if (i != m_waypoints.end())
    {
        Movement::WayPointMap* m = i->second;
        // we don't wanna erase from the map, because some are used more
        // than once (for instances)

        //m_waypoints.erase(i);
        return m;
    }
    else return NULL;
}

Pet* ObjectMgr::CreatePet(uint32 entry)
{
    uint32 guid;
    guid = ++m_hiPetGuid;
    return new Pet(Arcemu::Util::MAKE_PET_GUID(entry, guid));
}

Player* ObjectMgr::CreatePlayer(uint8 _class)
{
    uint32 guid;
    guid = ++m_hiPlayerGuid;

    Player* result = NULL;
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
        case DEATHKNIGHT:
            result = new DeathKnight(guid);
            break;
        case SHAMAN:
            result = new Shaman(guid);
            break;
        case MAGE:
            result = new Mage(guid);
            break;
        case WARLOCK:
            result = new Warlock(guid);
            break;
        case DRUID:
            result = new Druid(guid);
            break;
    }
    return result;
}

void ObjectMgr::AddPlayer(Player* p) //add it to global storage
{
    _playerslock.AcquireWriteLock();
    _players[p->GetLowGUID()] = p;
    _playerslock.ReleaseWriteLock();
}

void ObjectMgr::RemovePlayer(Player* p)
{
    _playerslock.AcquireWriteLock();
    _players.erase(p->GetLowGUID());
    _playerslock.ReleaseWriteLock();

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
    m_corpses[p->GetLowGUID()] = p;
    _corpseslock.Release();
}

void ObjectMgr::RemoveCorpse(Corpse* p)
{
    _corpseslock.Acquire();
    m_corpses.erase(p->GetLowGUID());
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

Transporter* ObjectMgr::GetTransporter(uint32 guid)
{
    Transporter* rv = nullptr;
    _TransportLock.Acquire();
    std::unordered_map<uint32, Transporter*>::const_iterator itr = m_Transports.find(guid);
    rv = (itr != m_Transports.end()) ? itr->second : 0;
    _TransportLock.Release();
    return rv;
}

Transporter* ObjectMgr::GetTransportOrThrow(uint32 guid)
{
    Transporter* transport = this->GetTransporter(guid);
    if (transport == nullptr)
        throw AscEmu::Exception::AscemuException("Transport not found");
    return transport;
}

void ObjectMgr::AddTransport(Transporter*transport)
{
    _TransportLock.Acquire();
    m_Transports[transport->GetUIdFromGUID()] = transport;
    _TransportLock.Release();
}

Transporter* ObjectMgr::GetTransporterByEntry(uint32 entry)
{
    Transporter* ret = nullptr;
    _TransportLock.Acquire();
    auto transporter = m_Transports.find(entry);
    if (transporter != m_Transports.end())
        ret = transporter->second;
    _TransportLock.Release();
    return ret;
}

void ObjectMgr::LoadGuildCharters()
{
    m_hiCharterId.SetVal(0);
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM charters");
    if (!result)
        return;
    do
    {
        Charter* c = new Charter(result->Fetch());
        m_charters[c->CharterType].insert(std::make_pair(c->GetID(), c));
        if (c->GetID() > int64(m_hiCharterId.GetVal()))
            m_hiCharterId.SetVal(c->GetID());
    }
    while (result->NextRow());
    delete result;
    Log.Success("ObjectMgr", "%u charters loaded.", m_charters[0].size());
}

Charter* ObjectMgr::GetCharter(uint32 CharterId, CharterTypes Type)
{
    Charter* rv;
    std::unordered_map<uint32, Charter*>::iterator itr;
    m_charterLock.AcquireReadLock();
    itr = m_charters[Type].find(CharterId);
    rv = (itr == m_charters[Type].end()) ? 0 : itr->second;
    m_charterLock.ReleaseReadLock();
    return rv;
}

Charter* ObjectMgr::CreateCharter(uint32 LeaderGuid, CharterTypes Type)
{
    uint32 charterid = 0;
    charterid = ++m_hiCharterId;

    Charter* c = new Charter(charterid, LeaderGuid, Type);
    m_charters[c->CharterType].insert(std::make_pair(c->GetID(), c));

    return c;
}

Charter::Charter(Field* fields)
{
    uint32 f = 0;
    CharterId = fields[f++].GetUInt32();
    CharterType = fields[f++].GetUInt32();
    LeaderGuid = fields[f++].GetUInt32();
    GuildName = fields[f++].GetString();
    ItemGuid = fields[f++].GetUInt64();
    SignatureCount = 0;
    Slots = GetNumberOfSlotsByType();
    Signatures = new uint32[Slots];

    for (uint32 i = 0; i < Slots; ++i)
    {
        Signatures[i] = fields[f++].GetUInt32();
        if (Signatures[i])
            ++SignatureCount;
    }

    // Unknown... really?
    Unk1 = 0;
    Unk2 = 0;
    Unk3 = 0;
    PetitionSignerCount = 0;
}

void Charter::AddSignature(uint32 PlayerGuid)
{
    if (SignatureCount >= Slots)
        return;

    ++SignatureCount;
    uint32 i = 0;
    for (; i < Slots; ++i)
    {
        if (Signatures[i] == 0)
        {
            Signatures[i] = PlayerGuid;
            break;
        }
    }

    ARCEMU_ASSERT(i != Slots);
}

void Charter::RemoveSignature(uint32 PlayerGuid)
{
    for (uint32 i = 0; i < Slots; ++i)
    {
        if (Signatures[i] == PlayerGuid)
        {
            Signatures[i] = 0;
            --SignatureCount;
            SaveToDB();
            break;
        }
    }
}

void Charter::Destroy()
{
    objmgr.RemoveCharter(this);

    CharacterDatabase.Execute("DELETE FROM charters WHERE charterId = %u", CharterId);

    for (uint32 i = 0; i < Slots; ++i)
    {
        if (!Signatures[i])
            continue;

        Player* p = objmgr.GetPlayer(Signatures[i]);
        if (p != nullptr)
            p->m_charters[CharterType] = 0;
    }

    delete this;
}

void Charter::SaveToDB()
{
    std::stringstream ss;
    uint32 i;

    ss << "DELETE FROM charters WHERE charterId = ";
    ss << CharterId;
    ss << ";";

    CharacterDatabase.Execute(ss.str().c_str());

    ss.rdbuf()->str("");

    ss << "INSERT INTO charters VALUES(" << CharterId << "," << CharterType << "," << LeaderGuid << ",'" << GuildName << "'," << ItemGuid;

    for (i = 0; i < Slots; ++i)
        ss << "," << Signatures[i];

    for (; i < 9; ++i)
        ss << ",0";

    ss << ")";
    CharacterDatabase.Execute(ss.str().c_str());
}

Charter* ObjectMgr::GetCharterByItemGuid(uint64 guid)
{
    m_charterLock.AcquireReadLock();
    for (uint8 i = 0; i < NUM_CHARTER_TYPES; ++i)
    {
        std::unordered_map<uint32, Charter*>::iterator itr = m_charters[i].begin();
        for (; itr != m_charters[i].end(); ++itr)
        {
            if (itr->second->ItemGuid == guid)
            {
                m_charterLock.ReleaseReadLock();
                return itr->second;
            }
        }
    }
    m_charterLock.ReleaseReadLock();
    return nullptr;
}

Charter* ObjectMgr::GetCharterByGuid(uint64 playerguid, CharterTypes type)
{
    m_charterLock.AcquireReadLock();
    std::unordered_map<uint32, Charter*>::iterator itr = m_charters[type].begin();
    for (; itr != m_charters[type].end(); ++itr)
    {
        if (playerguid == itr->second->LeaderGuid)
        {
            m_charterLock.ReleaseReadLock();
            return itr->second;
        }

        for (uint32 j = 0; j < itr->second->SignatureCount; ++j)
        {
            if (itr->second->Signatures[j] == playerguid)
            {
                m_charterLock.ReleaseReadLock();
                return itr->second;
            }
        }
    }
    m_charterLock.ReleaseReadLock();
    return nullptr;
}

Charter* ObjectMgr::GetCharterByName(std::string & charter_name, CharterTypes Type)
{
    m_charterLock.AcquireReadLock();
    std::unordered_map<uint32, Charter*>::iterator itr = m_charters[Type].begin();
    for (; itr != m_charters[Type].end(); ++itr)
    {
        if (itr->second->GuildName == charter_name)
        {
            return itr->second;
        }
    }

    m_charterLock.ReleaseReadLock();
    return nullptr;
}

void ObjectMgr::RemoveCharter(Charter* c)
{
    if (c == nullptr)
        return;

    if (c->CharterType >= NUM_CHARTER_TYPES)
    {
        Log.Notice("ObjectMgr", "Charter %u cannot be destroyed as type %u is not a sane type value.", c->CharterId, c->CharterType);
        return;
    }
    m_charterLock.AcquireWriteLock();
    m_charters[c->CharterType].erase(c->CharterId);
    m_charterLock.ReleaseWriteLock();
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
    Log.Notice("ObjectMgr", "%u reputation modifiers on %s.", dmap->size(), tablename);
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

void ObjectMgr::LoadMonsterSay()
{
    QueryResult* result = WorldDatabase.Query("SELECT * FROM npc_monstersay");
    if (!result)
        return;

    do
    {
        Field* fields = result->Fetch();
        uint32 Entry = fields[0].GetUInt32();
        uint32 Event = fields[1].GetUInt32();

        if (Event >= NUM_MONSTER_SAY_EVENTS)
            continue;

        if (mMonsterSays[Event].find(Entry) != mMonsterSays[Event].end())
        {
            LOG_ERROR("Duplicate monstersay event %u for entry %u, skipping", Event, Entry);
            continue;
        }

        NpcMonsterSay* ms = new NpcMonsterSay;
        ms->Chance = fields[2].GetFloat();
        ms->Language = fields[3].GetUInt32();
        ms->Type = fields[4].GetUInt32();
        ms->MonsterName = fields[5].GetString() ? strdup(fields[5].GetString()) : strdup("None");

        char* texts[5];
        char* text;
        uint32 textcount = 0;

        for (uint8 i = 0; i < 5; ++i)
        {
            text = (char*)fields[6 + i].GetString();
            if (!text) continue;
            if (strlen(fields[6 + i].GetString()) < 5)
                continue;

            texts[textcount] = strdup(fields[6 + i].GetString());

            // check for ;
            if (texts[textcount][strlen(texts[textcount]) - 1] == ';')
                texts[textcount][strlen(texts[textcount]) - 1] = 0;

            ++textcount;
        }

        if (!textcount)
        {
            free(((char*)ms->MonsterName));
            delete ms;
            continue;
        }

        ms->Texts = new const char*[textcount];
        memcpy(ms->Texts, texts, sizeof(char*) * textcount);
        ms->TextCount = textcount;

        mMonsterSays[Event].insert(std::make_pair(Entry, ms));

    }
    while (result->NextRow());
    Log.Success("ObjectMgr", "%u monster say events loaded.", result->GetRowCount());
    delete result;
}

NpcMonsterSay* ObjectMgr::HasMonsterSay(uint32 Entry, MONSTER_SAY_EVENTS Event)
{
    if (mMonsterSays[Event].empty())
        return nullptr;

    MonsterSayMap::iterator itr = mMonsterSays[Event].find(Entry);
    return itr == mMonsterSays[Event].end() ? nullptr : itr->second;
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

    Log.Success("ObjectMgr", "%u instance reputation modifiers loaded.", m_reputation_instance.size());
}

bool ObjectMgr::HandleInstanceReputationModifiers(Player* pPlayer, Unit* pVictim)
{
    uint32 team = pPlayer->GetTeam();

    if (!pVictim->IsCreature())
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
        value = float2int32(value * sWorld.getRate(RATE_KILLREPUTATION));
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

    Log.Notice("ObjectMgr", "%u disabled spells.", m_disabled_spells.size());
}

void ObjectMgr::ReloadDisabledSpells()
{
    m_disabled_spells.clear();
    LoadDisabledSpells();
}

void ObjectMgr::LoadGroups()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM groups");
    if (result)
    {
        if (result->GetFieldCount() != 52)
        {
            Log.LargeErrorMessage("groups table format is invalid. Please update your database.", NULL);
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

    Log.Success("ObjectMgr", "%u groups loaded.", this->m_groups.size());
}

void ObjectMgr::LoadArenaTeams()
{
    QueryResult* result = CharacterDatabase.Query("SELECT * FROM arenateams");
    if (result != NULL)
    {
        if (result->GetFieldCount() != 22)
        {
            Log.LargeErrorMessage("arenateams table format is invalid. Please update your database.", NULL);
            return;
        }
        do
        {
            ArenaTeam* team = new ArenaTeam(result->Fetch());
            AddArenaTeam(team);
            if (team->m_id > uint32(m_hiArenaTeamId.GetVal()))
                m_hiArenaTeamId.SetVal(uint32(team->m_id));

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
        if (itr->second->HasMember(guid))
        {
            m_arenaTeamLock.Release();
            return itr->second;
        }
    }
    m_arenaTeamLock.Release();
    return NULL;
}

ArenaTeam* ObjectMgr::GetArenaTeamById(uint32 id)
{
    std::unordered_map<uint32, ArenaTeam*>::iterator itr;
    m_arenaTeamLock.Acquire();
    itr = m_arenaTeams.find(id);
    m_arenaTeamLock.Release();
    return (itr == m_arenaTeams.end()) ? nullptr : itr->second;
}

ArenaTeam* ObjectMgr::GetArenaTeamByName(std::string & name, uint32 Type)
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
            return (a->m_stat_rating > b->m_stat_rating);
        }

        bool operator()(ArenaTeam*& a, ArenaTeam*& b)
        {
            return (a->m_stat_rating > b->m_stat_rating);
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
            if ((*itr)->m_stat_ranking != rank)
            {
                (*itr)->m_stat_ranking = rank;
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
                team->m_stat_gamesplayedseason = 0;
                team->m_stat_gamesplayedweek = 0;
                team->m_stat_gameswonseason = 0;
                team->m_stat_gameswonweek = 0;
                team->m_stat_rating = 1500;
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
                team->m_stat_gamesplayedweek = 0;
                team->m_stat_gameswonweek = 0;
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
    _playerslock.AcquireReadLock();
    PlayerStorageMap::iterator itr = _players.begin();
    for (; itr != _players.end(); ++itr)
    {
        Player* pPlayer = itr->second;
        pPlayer->DailyMutex.Acquire();
        pPlayer->m_finishedDailies.clear();
        pPlayer->DailyMutex.Release();
    }
    _playerslock.ReleaseReadLock();
}

void ObjectMgr::LoadSpellTargetConstraints()
{
    enum { CREATURE_FOCUS_TYPE, GAMEOBJECT_FOCUS_TYPE , CREATURE_TYPE, GAMEOBJECT_TYPE };

    Log.Notice("ObjectMgr", "Loading spell target constraints...");

    // Let's try to be idiot proof :/
    QueryResult* result = WorldDatabase.Query("SELECT * FROM spelltargetconstraints WHERE SpellID > 0 ORDER BY SpellID");
    if (result != nullptr)
    {
        uint32 oldspellid = 0;
        SpellTargetConstraint* stc = nullptr;

        do
        {
            Field* fields = result->Fetch();

            if (fields != NULL)
            {
                uint32 spellid = fields[0].GetUInt32();

                if (oldspellid != spellid)
                {
                    stc = new SpellTargetConstraint;

                    m_spelltargetconstraints.insert(std::pair< uint32, SpellTargetConstraint* >(spellid, stc));
                }

                uint32 type = fields[1].GetUInt32();
                uint32 value = fields[2].GetUInt32();

                if (type == CREATURE_FOCUS_TYPE)
                {
					if (stc != nullptr)
					{
						stc->AddCreature(value);
						stc->AddFocused(value, 1);
					}
                }              
				else if(type == GAMEOBJECT_FOCUS_TYPE)
                {
					if (stc != nullptr)
					{
						stc->AddGameobject(value);
						stc->AddFocused(value, 1);
					}
                }
				else if (type == CREATURE_TYPE)
				{
					if (stc != nullptr)
					{
						stc->AddCreature(value);
						stc->AddFocused(value, 0);
					}
				}
				else if (type == GAMEOBJECT_TYPE)
				{
					if (stc != nullptr)
					{
						stc->AddGameobject(value);
						stc->AddFocused(value, 0);
					}
				}

                oldspellid = spellid;
            }
        } while (result->NextRow());
    }

    delete result;

    Log.Notice("ObjectMgr", "Loaded constraints for %u spells...", m_spelltargetconstraints.size());
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
    cache->AddRef();
    PlayerCacheMap::iterator itr = m_playerCache.find(guid);
    if (itr != m_playerCache.end())
    {
        itr->second->DecRef();
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
        itr->second->DecRef();
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
        ret->AddRef();
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
        arcemu_TOLOWER(strName);
        for (itr = m_playerCache.begin(); itr != m_playerCache.end(); ++itr)
        {
            std::string cachename;
            itr->second->GetStringValue(CACHE_PLAYER_NAME, cachename);
            if (!stricmp(cachename.c_str(), strName.c_str()))
            {
                ret = itr->second;
                ret->AddRef();
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
                itr->second->AddRef();
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

AreaTrigger const* ObjectMgr::GetMapEntranceTrigger(uint32 Map) const
{
    for (AreaTriggerContainer::const_iterator itr = _areaTriggerStore.begin(); itr != _areaTriggerStore.end(); ++itr)
    {
        if (itr->second.Mapid == Map)
        {
            auto const* area_trigger_entry = sAreaTriggerStore.LookupEntry(itr->first);
            if (area_trigger_entry)
                return &itr->second;
        }
    }
    return nullptr;
}

void ObjectMgr::LoadAreaTrigger()
{
    _areaTriggerStore.clear();                                  // need for reload case
    //													0		1	2		3		4	5			6			7			8				9					10
    QueryResult* result = WorldDatabase.Query("SELECT entry, type, map, screen, name, position_x, position_y, position_z, orientation, required_honor_rank, required_level FROM areatriggers");
    if (!result)
    {
        Log.Notice("AreaTrigger", "Loaded 0 area trigger teleport definitions. DB table `areatriggers` is empty.");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        uint32 Trigger_ID = fields[0].GetUInt32();
        uint8 trigger_type = fields[1].GetUInt8();

        AreaTrigger at;
        at.AreaTriggerID = Trigger_ID;
        at.Type = trigger_type;
        at.PendingScreen = 0;

        at.Mapid = fields[2].GetUInt16();
        at.x = fields[5].GetFloat();
        at.y = fields[6].GetFloat();
        at.z = fields[7].GetFloat();
        at.o = fields[8].GetFloat();

        auto area_trigger_entry = sAreaTriggerStore.LookupEntry(Trigger_ID);
        if (!area_trigger_entry)
        {
            Log.Notice("AreaTrigger", "Area trigger (ID:%u) does not exist in `AreaTrigger.dbc`.", Trigger_ID);
            continue;
        }

        auto map_entry = sMapStore.LookupEntry(at.Mapid);
        if (!map_entry)
        {
            Log.Notice("AreaTrigger", "Area trigger (ID:%u) target map (ID: %u) does not exist in `Map.dbc`.", Trigger_ID, at.Mapid);
            continue;
        }

        if (at.x == 0 && at.y == 0 && at.z == 0 && (trigger_type == 1 || trigger_type == 4))    // check target coordinates only for teleport triggers
        {
            Log.Notice("AreaTrigger", "Area trigger (ID:%u) target coordinates not provided.", Trigger_ID);
            continue;
        }

        _areaTriggerStore[Trigger_ID] = at;
        ++count;

    } while (result->NextRow());

    delete result;

    Log.Success("AreaTrigger", "Loaded %u area trigger teleport definitions", count);
}

void ObjectMgr::LoadEventScripts()
{
    Log.Notice("ObjectMgr", "Loading Event Scripts...");

    bool success = false;
    const char* eventScriptsQuery = "SELECT event_id, function, script_type, data_1, data_2, data_3, data_4, data_5, x, y, z, o, delay, next_event FROM event_scripts WHERE event_id > 0 ORDER BY event_id";
    auto result = WorldDatabase.Query(&success, eventScriptsQuery);

    if (!success)
    {
        Log.Error("ObjectMgr", "Failed on Loading Queries from event_scripts.");
        return;
    }
    else
    {
        if (!result)
        {
            Log.Notice("ObjectMgr", "Loaded 0 event_scripts. DB table `event_scripts` is empty.");
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

    Log.Success("ObjectMgr", "Loaded event_scripts for %u events...", count);
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
    EventScriptBounds EventScript = objmgr.GetEventScripts(event_id);
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
    SpellEffectMapBounds EventScript = objmgr.GetSpellEffectBounds(data_1);
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
    EventScriptBounds EventScript = objmgr.GetEventScripts(next_event);

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
                    if (pQuest->GetMobCount(itr->second.data_5) < pQuest->GetQuest()->required_mob_or_go[itr->second.data_5])
                    {
                        pQuest->SetMobCount(itr->second.data_5, pQuest->GetMobCount(itr->second.data_5) + 1);
                        pQuest->SendUpdateAddKill(itr->second.data_5);
                        pQuest->UpdatePlayerFields();
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

                    if (static_cast<GameObject*>(target)->GetState() != GO_STATE_OPEN)
                    {
                        static_cast<GameObject*>(target)->SetState(GO_STATE_OPEN);
                    }
                    else
                    {
                        static_cast<GameObject*>(target)->SetState(GO_STATE_CLOSED);
                    }
                }
                else
                {
                    Object* target = plr->GetMapMgr()->GetInterface()->GetGameObjectNearestCoords(itr->second.x, itr->second.y, itr->second.z, itr->second.data_1);
                    if (target == nullptr)
                        return;

                    if (static_cast<GameObject*>(target)->GetState() != GO_STATE_OPEN)
                    {
                        static_cast<GameObject*>(target)->SetState(GO_STATE_OPEN);
                    }
                    else
                    {
                        static_cast<GameObject*>(target)->SetState(GO_STATE_CLOSED);
                    }
                }
            }
            break;
            }
        }

        if (itr->second.nextevent != 0)
        {
            objmgr.CheckforScripts(plr, itr->second.nextevent);
        }
    }
}

void ObjectMgr::LoadProfessionDiscoveries()
{
    QueryResult* result = WorldDatabase.Query("SELECT * from professiondiscoveries");
    if (result != nullptr)
    {
        do
        {
            Field* f = result->Fetch();
            ProfessionDiscovery* pf = new ProfessionDiscovery;
            pf->SpellId = f[0].GetUInt32();
            pf->SpellToDiscover = f[1].GetUInt32();
            pf->SkillValue = f[2].GetUInt32();
            pf->Chance = f[3].GetFloat();
            ProfessionDiscoveryTable.insert(pf);
        } while (result->NextRow());
        delete result;
    }
}

void ObjectMgr::LoadCreatureAIAgents()
{
    // Load AI Agents
    if (Config.MainConfig.GetBoolDefault("Server", "LoadAIAgents", true))
    {
        QueryResult* result = WorldDatabase.Query("SELECT * FROM ai_agents");
        if (result != nullptr)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 entry = fields[0].GetUInt32();
                CreatureProperties const* cn = sMySQLStore.GetCreatureProperties(entry);
                SpellEntry* spe = dbcSpell.LookupEntryForced(fields[6].GetUInt32());

                if (spe == nullptr)
                {
                    Log.Error("AIAgent", "For %u has nonexistent spell %u.", fields[0].GetUInt32(), fields[6].GetUInt32());
                    continue;
                }

                if (!cn)
                    continue;

                AI_Spell* sp = new AI_Spell;
                sp->entryId = fields[0].GetUInt32();
                sp->instance_mode = fields[1].GetUInt32();
                sp->agent = fields[2].GetUInt16();
                sp->procChance = fields[4].GetUInt32();
                sp->procCount = fields[5].GetUInt32();
                sp->spell = spe;
                sp->spellType = static_cast<uint8>(fields[7].GetUInt32());

                int32  targettype = fields[8].GetInt32();
                if (targettype == -1)
                    sp->spelltargetType = static_cast<uint8>(GetAiTargetType(spe));
                else
                    sp->spelltargetType = static_cast<uint8>(targettype);

                sp->cooldown = fields[9].GetInt32();
                sp->floatMisc1 = fields[10].GetFloat();
                sp->autocast_type = (uint32)-1;
                sp->cooldowntime = getMSTime();
                sp->procCounter = 0;
                sp->Misc2 = fields[11].GetUInt32();
                if (sp->agent == AGENT_SPELL)
                {
                    if (!sp->spell)
                    {
                        LOG_DEBUG("SpellId %u in ai_agent for %u is invalid.", (unsigned int)fields[6].GetUInt32(), (unsigned int)sp->entryId);
                        delete sp;
                        sp = nullptr;
                        continue;
                    }

                    if (sp->spell->Effect[0] == SPELL_EFFECT_LEARN_SPELL || sp->spell->Effect[1] == SPELL_EFFECT_LEARN_SPELL ||
                        sp->spell->Effect[2] == SPELL_EFFECT_LEARN_SPELL)
                    {
                        LOG_DEBUG("Teaching spell %u in ai_agent for %u", (unsigned int)fields[6].GetUInt32(), (unsigned int)sp->entryId);
                        delete sp;
                        sp = nullptr;
                        continue;
                    }

                    sp->minrange = GetMinRange(sSpellRangeStore.LookupEntry(sp->spell->rangeIndex));
                    sp->maxrange = GetMaxRange(sSpellRangeStore.LookupEntry(sp->spell->rangeIndex));

                    //omg the poor darling has no clue about making ai_agents
                    if (sp->cooldown == (uint32)-1)
                    {
                        //now this will not be exact cooldown but maybe a bigger one to not make him spam spells to often
                        int cooldown;
                        auto spell_duration = sSpellDurationStore.LookupEntry(sp->spell->DurationIndex);
                        int Dur = 0;
                        int Casttime = 0; //most of the time 0
                        int RecoveryTime = sp->spell->RecoveryTime;
                        if (sp->spell->DurationIndex)
                            Dur = ::GetDuration(spell_duration);
                        Casttime = GetCastTime(sSpellCastTimesStore.LookupEntry(sp->spell->CastingTimeIndex));
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
}

void ObjectMgr::StoreBroadCastGroupKey()
{
    if (!sWorld.BCSystemEnable)
    {
        Log.Notice("ObjectMgr", "BCSystem Disabled.");
        return;
    }

    std::vector<std::string> keyGroup;
    QueryResult* result = WorldDatabase.Query("SELECT DISTINCT percent FROM `worldbroadcast` ORDER BY percent DESC");
    if (result != nullptr)
    {
        do
        {
            Field* f = result->Fetch();
            keyGroup.push_back(std::string(f[0].GetString()));
        } while (result->NextRow());

        delete result;
    }

    if (keyGroup.empty())
    {
        Log.Notice("ObjectMgr", "BCSystem error! worldbroadcast empty? fill it first!");
        sWorld.BCSystemEnable = false;
        return;
    }
    else
    {
        Log.Notice("ObjectMgr", "BCSystem Enabled with %u KeyGroups.", keyGroup.size());
    }

    for (std::vector<std::string>::iterator itr = keyGroup.begin(); itr != keyGroup.end(); ++itr)
    {
        std::string curKey = (*itr);
        QueryResult* percentResult = WorldDatabase.Query("SELECT entry,percent FROM `worldbroadcast` WHERE percent='%s' ", curKey.c_str());
        if (percentResult != nullptr)
        {
            do
            {
                Field* f = percentResult->Fetch();
                m_BCEntryStorage.insert(std::pair<uint32, uint32>(uint32(atoi(curKey.c_str())), f[0].GetUInt32()));

            } while (percentResult->NextRow());

            delete percentResult;
        }
    }
}
