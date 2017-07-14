/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * String Localization Manager
 * Copyright (C) 2007-2012 Burlex <burlex@gmail.com>
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
#include "Management/LocalizationMgr.h"
#include "Server/MainServerDefines.h"
#include "Server/World.h"
#include "Server/World.Legacy.h"
#include "Config/Config.h"
#include "Log.hpp"
#include "Database/Database.h"
#include "Server/WUtil.h"
#include "../shared/Util.hpp"

LocalizationMgr sLocalizationMgr;

void LocalizationMgr::Shutdown()
{
    if (m_disabled)
        return;

#define SAFE_FREE_PTR(x) if (deletedPointers.find((x)) == deletedPointers.end()) { deletedPointers.insert((x)); free((x)); }

    std::set<void*> deletedPointers;
    uint32 maxid = 0;
    uint32 i, j;
    std::vector<std::pair<uint32, uint32> >::iterator xtr = m_languages.begin();
    for (; xtr != m_languages.end(); ++xtr)
        if (xtr->second > maxid)
            maxid = xtr->second;

    maxid++;
    LogNotice("LocalizationMgr : Beginning pointer cleanup...");
    uint32 t = getMSTime();

    for (i = 0; i < maxid; ++i)
    {
        for (std::unordered_map<uint32, LocalizedQuest>::iterator itr = m_Quests[i].begin(); itr != m_Quests[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Title);
            SAFE_FREE_PTR(itr->second.Details);
            SAFE_FREE_PTR(itr->second.Objectives);
            SAFE_FREE_PTR(itr->second.CompletionText);
            SAFE_FREE_PTR(itr->second.IncompleteText);
            SAFE_FREE_PTR(itr->second.EndText);
            SAFE_FREE_PTR(itr->second.ObjectiveText[0]);
            SAFE_FREE_PTR(itr->second.ObjectiveText[1]);
            SAFE_FREE_PTR(itr->second.ObjectiveText[2]);
            SAFE_FREE_PTR(itr->second.ObjectiveText[3]);
        }

        for (std::unordered_map<uint32, LocalizedItem>::iterator itr = m_Items[i].begin(); itr != m_Items[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Name);
            SAFE_FREE_PTR(itr->second.Description);
        }

        for (std::unordered_map<uint32, LocalizedNpcText>::iterator itr = m_NpcTexts[i].begin(); itr != m_NpcTexts[i].end(); ++itr)
        {
            for (j = 0; j < 8; ++j)
            {
                SAFE_FREE_PTR(itr->second.Texts[j][0]);
                SAFE_FREE_PTR(itr->second.Texts[j][1]);
            }
        }

        for (std::unordered_map<uint32, LocalizedCreatureName>::iterator itr = m_CreatureNames[i].begin(); itr != m_CreatureNames[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Name);
            SAFE_FREE_PTR(itr->second.SubName);
        }

        for (std::unordered_map<uint32, LocalizedGameObjectName>::iterator itr = m_GameObjectNames[i].begin(); itr != m_GameObjectNames[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Name);
        }

        for (std::unordered_map<uint32, LocalizedItemPage>::iterator itr = m_ItemPages[i].begin(); itr != m_ItemPages[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Text);
        }

        for (std::unordered_map<uint32, LocalizedNpcScriptText>::iterator itr = m_NpcScriptTexts[i].begin(); itr != m_NpcScriptTexts[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Text);
        }

        for (std::unordered_map<uint32, LocalizedGossipMenuOption>::iterator itr = m_GossipMenuOption[i].begin(); itr != m_GossipMenuOption[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Text);
        }

        for (std::unordered_map<uint32, LocalizedWorldStringTable>::iterator itr = m_WorldStrings[i].begin(); itr != m_WorldStrings[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Text);
        }

        for (std::unordered_map<uint32, LocalizedWorldBroadCast>::iterator itr = m_WorldBroadCast[i].begin(); itr != m_WorldBroadCast[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Text);
        }

        for (std::unordered_map<uint32, LocalizedWorldMapInfo>::iterator itr = m_WorldMapInfo[i].begin(); itr != m_WorldMapInfo[i].end(); ++itr)
        {
            SAFE_FREE_PTR(itr->second.Text);
        }
    }

    deletedPointers.clear();
    delete[] m_ItemPages;
    delete[] m_CreatureNames;
    delete[] m_GameObjectNames;
    delete[] m_Items;
    delete[] m_NpcTexts;
    delete[] m_Quests;
    delete[] m_NpcScriptTexts;
    delete[] m_GossipMenuOption;
    delete[] m_WorldStrings;
    delete[] m_WorldBroadCast;
    delete[] m_WorldMapInfo;
    m_languages.clear();

    LogNotice("LocalizationMgr : Pointer cleanup completed in %.4f seconds.", (getMSTime() - t) / 1000.0f);

#undef SAFE_FREE_PTR
}

void LocalizationMgr::Lower(std::string & conv)
{
    for (size_t i = 0; i < conv.length(); ++i)
        conv[i] = static_cast<char>(tolower(conv[i]));
}

void LocalizationMgr::GetDistinctLanguages(std::set<std::string>& dest, const char* table)
{
    QueryResult* result = WorldDatabase.Query("SELECT DISTINCT language_code FROM %s", table);
    if (result == NULL)
        return;

    std::string lc;
    do
    {
        lc = result->Fetch()[0].GetString();
        sLocalizationMgr.Lower(lc);
        if (dest.find(lc) == dest.end())
            dest.insert(lc);

    }
    while (result->NextRow());
    delete result;
}

#define MAX_LOCALIZED_CHAR 200
void LocalizationMgr::Reload(bool first)
{
    if (first)
    {
        return;
    }

    QueryResult* result;
    std::set<std::string> languages;
    languages.insert("enGB");
    languages.insert("enUS");
    languages.insert("koKR");
    languages.insert("frFR");
    languages.insert("deDE");
    languages.insert("esES");
    languages.insert("ruRU");


    std::map<std::string, std::string> bound_languages;
    GetDistinctLanguages(languages, "locales_creature");
    GetDistinctLanguages(languages, "locales_gameobject");
    GetDistinctLanguages(languages, "locales_item");
    GetDistinctLanguages(languages, "locales_quest");
    GetDistinctLanguages(languages, "locales_npc_text");
    GetDistinctLanguages(languages, "locales_npc_script_text");
    GetDistinctLanguages(languages, "locales_item_pages");
    GetDistinctLanguages(languages, "locales_gossip_menu_option");
    GetDistinctLanguages(languages, "locales_worldstring_table");
    GetDistinctLanguages(languages, "locales_worldbroadcast");
    GetDistinctLanguages(languages, "locales_worldmap_info");
    GetDistinctLanguages(languages, "locales_npc_monstersay");


    m_CreatureNames = new std::unordered_map<uint32, LocalizedCreatureName>[languages.size()];
    m_GameObjectNames = new std::unordered_map<uint32, LocalizedGameObjectName>[languages.size()];
    m_Quests = new std::unordered_map<uint32, LocalizedQuest>[languages.size()];
    m_NpcTexts = new std::unordered_map<uint32, LocalizedNpcText>[languages.size()];
    m_Items = new std::unordered_map<uint32, LocalizedItem>[languages.size()];
    m_ItemPages = new std::unordered_map<uint32, LocalizedItemPage>[languages.size()];
    m_NpcScriptTexts = new std::unordered_map<uint32, LocalizedNpcScriptText>[languages.size()];
    m_GossipMenuOption = new std::unordered_map<uint32, LocalizedGossipMenuOption>[languages.size()];
    m_WorldStrings = new std::unordered_map<uint32, LocalizedWorldStringTable>[languages.size()];
    m_WorldBroadCast = new std::unordered_map<uint32, LocalizedWorldBroadCast>[languages.size()];
    m_WorldMapInfo = new std::unordered_map<uint32, LocalizedWorldMapInfo>[languages.size()];

    //////////////////////////////////////////////////////////////////////////////////////////
    // Creature Names
    {
        LocalizedCreatureName cn;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_creature");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;        // no loading enUS/enGB stuff.. lawl
                }

                cn.Name = strdup(f[2].GetString());
                cn.SubName = strdup(f[3].GetString());
                m_CreatureNames[lid].insert(std::make_pair(entry, cn));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // GameObject Names
    {
        LocalizedGameObjectName gn;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_gameobject");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;        // no loading enUS stuff.. lawl
                }

                gn.Name = strdup(f[2].GetString());
                m_GameObjectNames[lid].insert(std::make_pair(entry, gn));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Items
    {
        LocalizedItem it;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_item");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;        // no loading enUS stuff.. lawl
                }

                if (m_Items[lid].find(entry) != m_Items[lid].end())
                {
                    continue;
                }

                it.Name = strdup(f[2].GetString());
                it.Description = strdup(f[3].GetString());
                m_Items[lid].insert(std::make_pair(entry, it));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Quests
    {
        LocalizedQuest q;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_quest");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;        // no loading enUS stuff.. lawl
                }

                q.Title = strdup(f[2].GetString());
                q.Details = strdup(f[3].GetString());
                q.Objectives = strdup(f[4].GetString());
                q.CompletionText = strdup(f[5].GetString());
                q.IncompleteText = strdup(f[6].GetString());
                q.EndText = strdup(f[7].GetString());
                q.ObjectiveText[0] = strdup(f[8].GetString());
                q.ObjectiveText[1] = strdup(f[9].GetString());
                q.ObjectiveText[2] = strdup(f[10].GetString());
                q.ObjectiveText[3] = strdup(f[11].GetString());

                m_Quests[lid].insert(std::make_pair(entry, q));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // NPC Texts
    {
        LocalizedNpcText nt;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;
        uint32 counter;

        result = WorldDatabase.Query("SELECT * FROM locales_npc_text");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;        // no loading enUS stuff.. lawl
                }

                counter = 2;
                for (uint8 i = 0; i < 8; ++i)
                {
                    nt.Texts[i][0] = strdup(f[counter++].GetString());
                    nt.Texts[i][1] = strdup(f[counter++].GetString());
                }

                m_NpcTexts[lid].insert(std::make_pair(entry, nt));
            }
            while (result->NextRow());
            delete result;
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////////
    // Item Pages
    {
        LocalizedItemPage nt;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_item_pages");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;        // no loading enUS stuff.. lawl
                }

                nt.Text = strdup(f[2].GetString());
                m_ItemPages[lid].insert(std::make_pair(entry, nt));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Creature Text
    {
        uint32_t locales_npc_script_text_count = 0;
        uint32_t start_loc_npc_script_txt = getMSTime();
        result = WorldDatabase.Query("SELECT * FROM `locales_npc_script_text`");
        if (result)
        {
            do
            {
                Field* field = result->Fetch();
                uint32_t entry = field[0].GetUInt32();
                std::string languageString = std::string(field[1].GetString());

                uint32 languageId = getLanguagesIdFromString(languageString);
                if (languageId == 0)
                {
                    continue;
                }

                LocalizedNpcScriptText nt;
                nt.Text = strdup(field[2].GetString());
                m_NpcScriptTexts[languageId].insert(std::make_pair(entry, nt));
                ++locales_npc_script_text_count;
            }
            while (result->NextRow());
            delete result;
        }

        LogDetail("LocalizationMgr : Loaded %u rows from table `locales_npc_script_text` in %u ms!", locales_npc_script_text_count, getMSTime() - start_loc_npc_script_txt);
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // Gossip Menu Option
    {
        LocalizedGossipMenuOption nt;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_gossip_menu_option");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;
                }

                nt.Text = strdup(f[2].GetString());
                m_GossipMenuOption[lid].insert(std::make_pair(entry, nt));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // World Common Message
    {
        LocalizedWorldStringTable nt;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_worldstring_table");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;
                }

                nt.Text = strdup(f[2].GetString());
                m_WorldStrings[lid].insert(std::make_pair(entry, nt));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // World BroadCast Messages
    {
        LocalizedWorldBroadCast nt;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_worldbroadcast");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;
                }

                nt.Text = strdup(f[2].GetString());
                m_WorldBroadCast[lid].insert(std::make_pair(entry, nt));
            }
            while (result->NextRow());
            delete result;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // World MapInfo Entry Name
    {
        LocalizedWorldMapInfo nt;
        std::string str;
        uint32 entry;
        Field* f;
        uint32 lid;

        result = WorldDatabase.Query("SELECT * FROM locales_worldmap_info");
        if (result)
        {
            do
            {
                f = result->Fetch();
                str = std::string(f[1].GetString());
                entry = f[0].GetUInt32();

                lid = getLanguagesIdFromString(str);
                if (lid == 0)
                {
                    continue;
                }

                nt.Text = strdup(f[2].GetString());
                m_WorldMapInfo[lid].insert(std::make_pair(entry, nt));
            }
            while (result->NextRow());
            delete result;
        }
    }
}

#define MAKE_LOOKUP_FUNCTION(t, hm, fn) t * LocalizationMgr::fn(uint32 id, uint32 language) { \
    if (m_disabled) { return NULL; } \
    std::unordered_map<uint32, t>::iterator itr = hm[language].find(id); \
    return (itr == hm[language].end()) ? NULL : &itr->second; }

MAKE_LOOKUP_FUNCTION(LocalizedCreatureName, m_CreatureNames, GetLocalizedCreatureName);
MAKE_LOOKUP_FUNCTION(LocalizedGameObjectName, m_GameObjectNames, GetLocalizedGameObjectName);
MAKE_LOOKUP_FUNCTION(LocalizedQuest, m_Quests, GetLocalizedQuest);
MAKE_LOOKUP_FUNCTION(LocalizedItem, m_Items, GetLocalizedItem);
MAKE_LOOKUP_FUNCTION(LocalizedNpcText, m_NpcTexts, GetLocalizedNpcText);
MAKE_LOOKUP_FUNCTION(LocalizedItemPage, m_ItemPages, GetLocalizedItemPage);
MAKE_LOOKUP_FUNCTION(LocalizedNpcScriptText, m_NpcScriptTexts, GetLocalizedNpcScriptText);
MAKE_LOOKUP_FUNCTION(LocalizedGossipMenuOption, m_GossipMenuOption, GetLocalizedGossipMenuOption);
MAKE_LOOKUP_FUNCTION(LocalizedWorldStringTable, m_WorldStrings, GetLocalizedWorldStringTable);
MAKE_LOOKUP_FUNCTION(LocalizedWorldBroadCast, m_WorldBroadCast, GetLocalizedWorldBroadCast);
MAKE_LOOKUP_FUNCTION(LocalizedWorldMapInfo, m_WorldMapInfo, GetLocalizedWorldMapInfo);
