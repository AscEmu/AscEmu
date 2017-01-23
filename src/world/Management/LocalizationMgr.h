/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (C) 2014-2017 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2008-2012 ArcEmu Team <http://www.ArcEmu.org/>
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
 */
#pragma once 

#include <set>
#include <unordered_map>

struct LocalizedCreatureName
{
    char* Name;
    char* SubName;
};

struct LocalizedGameObjectName
{
    char* Name;
};

struct LocalizedNpcText
{
    char* Texts[8][2];
};

struct LocalizedItemPage
{
    char* Text;
};

struct LocalizedItem
{
    char* Name;
    char* Description;
};

struct LocalizedQuest
{
    char* Title;
    char* Details;
    char* Objectives;
    char* CompletionText;
    char* IncompleteText;
    char* EndText;
    char* ObjectiveText[4];
};

struct LocalizedWorldBroadCast
{
    char* Text;
};

struct LocalizedGossipMenuOption
{
    char* Text;
};

struct LocalizedCreatureText
{
    char* Text;
};

struct LocalizedWorldStringTable
{
    char* Text;
};

struct LocalizedWorldMapInfo
{
    char* Text;
};

struct LocalizedMonstersay
{
    char* monstername;
    char* text0;
    char* text1;
    char* text2;
    char* text3;
    char* text4;
};

class LocalizationMgr
{
public:

    void Shutdown();
    void Reload(bool first);
    void Lower(std::string& conv);
    uint32_t GetLanguageId(uint32_t full);

    uint32_t GetLanguageId(std::string langstr)
    {
        std::string ns = langstr;
        Lower(ns);

        uint32_t lid = *(uint32_t*)ns.c_str();
        return GetLanguageId(lid);
    }

    void GetDistinctLanguages(std::set<std::string>& dest, const char* table);

    LocalizedQuest* GetLocalizedQuest(uint32_t id, uint32_t language);
    LocalizedItem* GetLocalizedItem(uint32_t id, uint32_t language);
    LocalizedNpcText* GetLocalizedNpcText(uint32_t id, uint32_t language);
    LocalizedCreatureName* GetLocalizedCreatureName(uint32_t id, uint32_t language);
    LocalizedGameObjectName* GetLocalizedGameObjectName(uint32_t id, uint32_t language);
    LocalizedItemPage* GetLocalizedItemPage(uint32_t id, uint32_t language);
    LocalizedCreatureText* GetLocalizedCreatureText(uint32_t id, uint32_t language);
    LocalizedGossipMenuOption* GetLocalizedGossipMenuOption(uint32_t id, uint32_t language);
    LocalizedWorldStringTable* GetLocalizedWorldStringTable(uint32_t id, uint32_t language);
    LocalizedWorldBroadCast* GetLocalizedWorldBroadCast(uint32_t id, uint32_t language);
    LocalizedWorldMapInfo* GetLocalizedWorldMapInfo(uint32_t id, uint32_t language);
    LocalizedMonstersay* GetLocalizedMonstersay(uint32_t id, uint32_t language);

    template <typename T>
    void CopyHashMap(std::unordered_map<uint32_t, T>* src, std::unordered_map<uint32_t, T>* dest)
    {
        for (typename std::unordered_map<uint32_t, T>::iterator itr = src->begin(); itr != src->end(); ++itr)
            dest->insert(std::make_pair(itr->first, itr->second));
    }

private:

    std::unordered_map<uint32_t, LocalizedQuest>* m_Quests;
    std::unordered_map<uint32_t, LocalizedItem>* m_Items;
    std::unordered_map<uint32_t, LocalizedNpcText>* m_NpcTexts;
    std::unordered_map<uint32_t, LocalizedCreatureName>* m_CreatureNames;
    std::unordered_map<uint32_t, LocalizedGameObjectName>* m_GameObjectNames;
    std::unordered_map<uint32_t, LocalizedItemPage>* m_ItemPages;
    std::unordered_map<uint32_t, LocalizedCreatureText>* m_CreatureText;
    std::unordered_map<uint32_t, LocalizedGossipMenuOption>* m_GossipMenuOption;
    std::unordered_map<uint32_t, LocalizedWorldStringTable>* m_WorldStrings;
    std::unordered_map<uint32_t, LocalizedWorldBroadCast>* m_WorldBroadCast;
    std::unordered_map<uint32_t, LocalizedWorldMapInfo>* m_WorldMapInfo;
    std::unordered_map<uint32_t, LocalizedMonstersay>* m_MonsterSay;

    std::vector<std::pair<uint32_t, uint32_t>> m_languages;
    bool m_disabled;
};

extern LocalizationMgr sLocalizationMgr;
