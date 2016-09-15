/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2015 AscEmu Team <http://www.ascemu.org/>
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

initialiseSingleton(GuildMgr);

GuildMgr::GuildMgr()
{ }

GuildMgr::~GuildMgr()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        delete itr->second;
}

void GuildMgr::AddGuild(Guild* guild)
{
    GuildStore[guild->GetId()] = guild;
}

void GuildMgr::RemoveGuild(uint32 guildId)
{
    GuildStore.erase(guildId);
}

void GuildMgr::SaveGuilds()
{
    for (GuildContainer::iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        itr->second->SaveToDB();
}

uint32 GuildMgr::GenerateGuildId()
{
    return objmgr.GenerateGuildId();
}

// Guild collection
Guild* GuildMgr::GetGuildById(uint32 guildId) const
{
    GuildContainer::const_iterator itr = GuildStore.find(guildId);
    if (itr != GuildStore.end())
        return itr->second;

    return nullptr;
}

Guild* GuildMgr::GetGuildByName(const std::string& guildName) const
{
    std::string search = guildName;
    std::transform(search.begin(), search.end(), search.begin(), ::toupper);
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
    {
        std::string gname = itr->second->GetName();
        std::transform(gname.begin(), gname.end(), gname.begin(), ::toupper);
        if (search == gname)
            return itr->second;
    }
    return nullptr;
}

std::string GuildMgr::GetGuildNameById(uint32 guildId) const
{
    if (Guild* guild = GetGuildById(guildId))
        return guild->GetName();

    return "";
}

Guild* GuildMgr::GetGuildByLeader(uint64 guid) const
{
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        if (itr->second->GetLeaderGUID() == guid)
            return itr->second;

    return nullptr;
}

uint32 GuildMgr::GetXPForGuildLevel(uint8 level) const
{
    if (level < GuildXPperLevel.size())
        return GuildXPperLevel[level];
    return 0;
}

void GuildMgr::LoadGuilds()
{
    // 1. Load all guilds
    Log.Debug("GuildMgr", "Loading guilds definitions...");
    {
        uint32 oldMSTime = getMSTime();

        //                                                         0          1            2             3              4              5              6
        QueryResult* result = CharacterDatabase.Query("SELECT g.guildId, g.guildName, g.leaderGuid, g.emblemStyle, g.emblemColor, g.borderStyle, g.borderColor, "
        //           7               8          9          10            11            12              13                 4                   15
            "g.backgroundColor, g.guildInfo, g.motd, g.createdate, g.bankBalance, g.guildLevel, g.guildExperience, g.todayExperience, COUNT(gbt.guildId) "
            "FROM guild g LEFT JOIN guild_bank_tab gbt ON g.guildId = gbt.guildId GROUP BY g.guildId ORDER BY g.guildId ASC");

        if (!result)
        {
            Log.Debug("GuildMgr", "Loaded 0 guild definitions. DB table `guild` is empty.");
            return;
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                Guild* guild = new Guild();

                if (!guild->LoadFromDB(fields))
                {
                    delete guild;
                    continue;
                }

                AddGuild(guild);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 2. Load all guild ranks
    Log.Debug("GuildMgr", "Loading guild ranks...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild rank entries before loading the valid ones
        CharacterDatabase.Execute("DELETE gr FROM guild_rank gr LEFT JOIN guild g ON gr.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0      1      2      3           4
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, rid, rname, rights, bankMoneyPerDay FROM guild_rank ORDER BY guildid ASC, rid ASC");

        if (!result)
        {
            Log.Debug("GuildMgr", "Loaded 0 guild ranks. DB table `guild_rank` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadRankFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild ranks in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 3. Load all guild members
    Log.Debug("GuildMgr", "Loading guild members...");
    {
        uint32 oldMSTime = getMSTime();

        QueryResult* result = CharacterDatabase.Query("SELECT guildId, playerGuid, rank, pnote, offnote FROM guild_member");
        QueryResult* result2 = CharacterDatabase.Query("SELECT guid, tab0, tab1, tab2, tab3, tab4, tab5, tab6, tab7, money FROM guild_member_withdraw");

        if (!result && !result2)
            Log.Debug("GuildMgr", "Loaded 0 guild members. DB table `guild_member` OR `guild_member_withdraw` is empty.");
        else
        {
            uint32 count = 0;

            do
            {
                Field* fields = result->Fetch();
                Field* fields2 = result2->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadMemberFromDB(fields, fields2);

                ++count;
            } while (result->NextRow() && result2->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild members int %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 4. Load all guild bank tab rights
    Log.Debug("GuildMgr", "Loading bank tab rights...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild bank right entries before loading the valid ones
        CharacterDatabase.Execute("DELETE gbr FROM guild_bank_right gbr LEFT JOIN guild g ON gbr.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0        1    2      3        4
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, rid, gbright, slotPerDay FROM guild_bank_right ORDER BY guildId ASC, tabId ASC");

        if (!result)
        {
            Log.Debug("GuildMgr", "Loaded 0 guild bank tab rights. DB table `guild_bank_right` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankRightFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u bank tab rights in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 5. Load all event logs
    Log.Debug("GuildMgr", "Loading guild event logs...");
    {
        uint32 oldMSTime = getMSTime();

        CharacterDatabase.Execute("DELETE FROM guild_eventlog WHERE logGuid > %u", 100);

        //                                                      0         1        2            3            4          5        6
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, logGuid, eventType, playerGuid1, playerGuid2, newRank, timeStamp FROM guild_eventlog ORDER BY timeStamp DESC, logGuid DESC");

        if (!result)
        {
            Log.Debug("GuildMgr", "Loaded 0 guild event logs. DB table `guild_eventlog` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadEventLogFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild event logs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 6. Load all bank event logs
    Log.Debug("GuildMgr", "Loading guild bank event logs...");
    {
        uint32 oldMSTime = getMSTime();

        // Remove log entries that exceed the number of allowed entries per guild
        CharacterDatabase.Execute("DELETE FROM guild_bank_eventlog WHERE logGuid > %u", 25);

        //                                                       0       1       2         3           4           5             6             7          8
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, logGuid, eventType, playerGuid, itemOrMoney, itemStackCount, destTabId, timeStamp FROM guild_bank_eventlog ORDER BY timeStamp DESC, logGuid DESC");

        if (!result)
        {
            Log.Debug("GuildMgr", "Loaded 0 guild bank event logs. DB table `guild_bank_eventlog` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankEventLogFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild bank event logs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 7. Load all news event logs
    Log.Debug("GuildMgr", "Loading Guild News...");
    {
        uint32 oldMSTime = getMSTime();

        CharacterDatabase.Execute("DELETE FROM guild_newslog WHERE logGuid > %u", 250);

        //                                                       0        1         2          3         4      5        6
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, logGuid, eventType, playerGuid, flags, value, timeStamp FROM guild_newslog ORDER BY timeStamp DESC, logGuid DESC");

        if (!result)
            Log.Debug("GuildMgr", "Loaded 0 guild event logs. DB table `guild_newslog` is empty.");
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadGuildNewsLogFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild new logs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }


    // 8. Load all guild bank tabs
    Log.Debug("GuildMgr", "Loading guild bank tabs...");
    {
        uint32 oldMSTime = getMSTime();

        // Delete orphaned guild bank tab entries before loading the valid ones
        CharacterDatabase.Execute("DELETE gbt FROM guild_bank_tab gbt LEFT JOIN guild g ON gbt.guildId = g.guildId WHERE g.guildId IS NULL");

        //                                                       0       1       2        3        4
        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, tabName, tabIcon, tabText FROM guild_bank_tab ORDER BY guildId ASC, tabId ASC");

        if (!result)
        {
            Log.Debug("GuildMgr", "Loaded 0 guild bank tabs. DB table `guild_bank_tab` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankTabFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Debug("GuildMgr", "Loaded %u guild bank tabs in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 9. Fill all guild bank tabs
    Log.Debug("GuildMgr", "Filling bank tabs with items...");
    {
        uint32 oldMSTime = getMSTime();

        QueryResult* result = CharacterDatabase.Query("SELECT guildId, tabId, slotId, itemGuid FROM guild_bank_item");

        if (!result)
        {
            Log.Error("GuildMgr", "Loaded 0 guild bank tab items. DB table `guild_bank_item` or `item_instance` is empty.");
        }
        else
        {
            uint32 count = 0;
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();

                if (Guild* guild = GetGuildById(guildId))
                    guild->LoadBankItemFromDB(fields);

                ++count;
            } while (result->NextRow());

            Log.Success("GuildMgr", "Loaded %u guild bank tab items in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
        }
    }

    // 10. Load guild achievements TODO
    {
        //TODO
    }
}

void GuildMgr::LoadGuildXpForLevel()
{
    uint32 oldMSTime = getMSTime();

    GuildXPperLevel.resize(sWorld.m_guild.MaxLevel);
    for (uint8 level = 0; level < sWorld.m_guild.MaxLevel; ++level)
        GuildXPperLevel[level] = 0;

    //                                                 0         1
    QueryResult* result = WorldDatabase.Query("SELECT lvl, xp_for_next_level FROM guild_xp_for_level");

    if (!result)
    {
        Log.Debug("GuildMgr", "Loaded 0 xp for guild level definitions. DB table `guild_xp_for_level` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        Field* fields = result->Fetch();

        uint32 level = fields[0].GetUInt8();
        uint32 requiredXP = fields[1].GetUInt64();

        if (level >= sWorld.m_guild.MaxLevel)
        {
            Log.Debug("GuildMgr", "Unused (> Guild.MaxLevel in worldserver.conf) level %u in `guild_xp_for_level` table, ignoring.", uint32(level));
            continue;
        }

        GuildXPperLevel[level] = requiredXP;
        ++count;

    } while (result->NextRow());

    // fill level gaps
    for (uint8 level = 1; level < sWorld.m_guild.MaxLevel ; ++level)
    {
        if (!GuildXPperLevel[level])
        {
            Log.Error("GuildMgr", "Level %i does not have XP for guild level data. Using data of level [%i] + 1660000.", level + 1, level);
            GuildXPperLevel[level] = GuildXPperLevel[level - 1] + 1660000;
        }
    }

    Log.Debug("GuildMgr", "Loaded %u xp for guild level definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildMgr::LoadGuildRewards()
{
    uint32 oldMSTime = getMSTime();

    //                                                  0       1         2        3         4
    QueryResult* result = WorldDatabase.Query("SELECT entry, standing, racemask, price, achievement FROM guild_rewards");

    if (!result)
    {
        Log.Debug("GuildMgr", "Loaded 0 guild reward definitions. DB table `guild_rewards` is empty.");
        return;
    }

    uint32 count = 0;

    do
    {
        GuildReward reward;
        Field* fields = result->Fetch();
        reward.Entry = fields[0].GetUInt32();
        reward.Standing = fields[1].GetUInt8();
        reward.Racemask = fields[2].GetInt32();
        reward.Price = fields[3].GetUInt64();
        reward.AchievementId = fields[4].GetUInt32();

        if (!sItemStore.LookupEntry(reward.Entry))
        {
            Log.Error("GuildMgr", "Guild rewards constains not existing item entry %u", reward.Entry);
            continue;
        }

        if (reward.Standing >= 8)
        {
            Log.Error("GuildMgr", "Guild rewards contains wrong reputation standing %u, max is %u", uint32(reward.Standing), 8 - 1);
            continue;
        }

        GuildRewards.push_back(reward);
        ++count;
    } while (result->NextRow());

    Log.Debug("GuildMgr", "Loaded %u guild reward definitions in %u ms", count, GetMSTimeDiffToNow(oldMSTime));
}

void GuildMgr::ResetTimes(bool week)
{
    for (GuildContainer::const_iterator itr = GuildStore.begin(); itr != GuildStore.end(); ++itr)
        if (Guild* guild = itr->second)
            guild->ResetTimes(week);
}

void GuildMgr::Update(uint32 diff)
{
    if (!firstSave)
    {
        lastSave = time(NULL);
        firstSave = true;
    }

    if (time(NULL) >= lastSave)
    {
        lastSave = time(NULL) + sWorld.m_guild.SaveInterval;
        sGuildMgr.SaveGuilds();
    }
}
