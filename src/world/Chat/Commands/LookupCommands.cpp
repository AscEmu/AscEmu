/*
Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "Chat/ChatHandler.hpp"
#include "Management/AchievementMgr.h"
#include "Management/QuestProperties.hpp"
#include "Objects/GameObjectProperties.hpp"
#include "Objects/Units/Players/Player.hpp"
#include "Server/WorldSession.h"
#include "Spell/SpellInfo.hpp"
#include "Spell/SpellMgr.hpp"
#include "Storage/MySQLDataStore.hpp"
#include "Storage/MySQLStructures.h"
#include "Storage/WDB/WDBStores.hpp"
#include "Storage/WDB/WDBStructures.hpp"
#include "Utilities/Strings.hpp"

#if VERSION_STRING < Cata
#include "Server/World.h"
#endif

//.lookup achievement
//////////////////////////////////////////////////////////////////////////////////////////
//Handles .lookup achievement
//\brief GM achievement lookup command usage:
//.lookup achievement string : searches for "string" in achievement name
//.lookup achievement desc string : searches for "string" in achievement description
//.lookup achievement reward string : searches for "string" in achievement reward name
//.lookup achievement criteria string : searches for "string" in achievement criteria name
//.lookup achievement all string : searches for "string" in achievement name, description, reward, and critiera
//////////////////////////////////////////////////////////////////////////////////////////
bool ChatHandler::HandleLookupAchievementCommand([[maybe_unused]]const char* args, [[maybe_unused]]WorldSession* m_session)
{
#if VERSION_STRING > TBC
    if (!*args)
        return false;

    std::string x;
    bool lookupname = true, lookupdesc = false, lookupcriteria = false, lookupreward = false;
    std::string name(args, 5);
    std::string desc(args, 5);
    std::string criteria(args, 9);
    std::string reward(args, 7);
    std::string all(args, 4);
    if (AscEmu::Util::Strings::isEqual(name, "name "))
    {
        x = std::string(args + 5);
    }
    else if (AscEmu::Util::Strings::isEqual(desc, "desc "))
    {
        lookupname = false;
        lookupdesc = true;
        x = std::string(args + 5);
    }
    else if (AscEmu::Util::Strings::isEqual(criteria, "criteria "))
    {
        lookupname = false;
        lookupcriteria = true;
        x = std::string(args + 9);
    }
    else if (AscEmu::Util::Strings::isEqual(reward, "reward "))
    {
        lookupname = false;
        lookupreward = true;
        x = std::string(args + 7);
    }
    else if (AscEmu::Util::Strings::isEqual(all, "all "))
    {
        lookupdesc = true;
        lookupcriteria = true;
        lookupreward = true;
        x = std::string(args + 4);
    }
    else
    {
        x = std::string(args);
    }

    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    AscEmu::Util::Strings::toLowerCase(x);
    GreenSystemMessage(m_session, "Starting search of achievement `%s`...", x.c_str());
    auto startTime = Util::TimeNow();
    uint32_t i, j, numFound = 0;
    std::string y, recout;
    char playerGUID[17];
    snprintf(playerGUID, 17, "%llu", m_session->GetPlayer()->getGuid());
    if (lookupname || lookupdesc || lookupreward)
    {
        std::set<uint32_t> foundList;
        j = sAchievementStore.getNumRows();
        bool foundmatch;
        for (i = 0; i < j && numFound < 25; ++i)
        {
            auto achievement = sAchievementStore.lookupEntry(i);
            if (achievement)
            {
                if (foundList.find(achievement->ID) != foundList.end())
                {
                    // already listed this achievement (some achievements have multiple entries in dbc)
                    continue;
                }
                foundmatch = false;
                if (lookupname)
                {
#if VERSION_STRING < Cata
                    y = std::string(achievement->name[sWorld.getDbcLocaleLanguageId()]);
#else
                    y = std::string(achievement->name[0]);
#endif
                    AscEmu::Util::Strings::toLowerCase(y);
                    foundmatch = AscEmu::Util::Strings::contains(x, y);
                }
                if (!foundmatch && lookupdesc)
                {
#if VERSION_STRING < Cata
                    y = std::string(achievement->description[sWorld.getDbcLocaleLanguageId()]);
#else
                    y = std::string(achievement->description[0]);
#endif
                    AscEmu::Util::Strings::toLowerCase(y);
                    foundmatch = AscEmu::Util::Strings::contains(x, y);
                }
                if (!foundmatch && lookupreward)
                {
#if VERSION_STRING < Cata
                    y = std::string(achievement->rewardName[sWorld.getDbcLocaleLanguageId()]);
#else
                    y = std::string(achievement->rewardName[0]);
#endif
                    AscEmu::Util::Strings::toLowerCase(y);
                    foundmatch = AscEmu::Util::Strings::contains(x, y);
                }
                if (!foundmatch)
                {
                    continue;
                }
                foundList.insert(achievement->ID);
                std::stringstream strm;
                strm << achievement->ID;
                // create achievement link
                recout = "|cffffffffAchievement ";
                recout += strm.str();
                recout += ": |cfffff000|Hachievement:";
                recout += strm.str();
                recout += ":";
                recout += playerGUID;
                time_t completetime = m_session->GetPlayer()->getAchievementMgr()->getCompletedTime(achievement);
                if (completetime)
                {
                    // achievement is completed
                    struct tm* ct;
                    ct = localtime(&completetime);
                    strm.str("");
                    strm << ":1:" << ct->tm_mon + 1 << ":" << ct->tm_mday << ":" << ct->tm_year - 100 << ":-1:-1:-1:-1|h[";
                    recout += strm.str();
                }
                else
                {
                    // achievement is not completed
                    recout += ":0:0:0:-1:0:0:0:0|h[";
                }
#if VERSION_STRING < Cata
                recout += achievement->name[sWorld.getDbcLocaleLanguageId()];
#else
                recout += achievement->name[0];
#endif
                if (!lookupreward)
                {
                    recout += "]|h|r";
                }
                else
                {
                    recout += "]|h |cffffffff";
#if VERSION_STRING < Cata
                    recout += achievement->rewardName[sWorld.getDbcLocaleLanguageId()];
#else
                    recout += achievement->rewardName[0];
#endif
                    recout += "|r";
                }
                strm.str("");
                SendMultilineMessage(m_session, recout.c_str());
                if (++numFound >= 25)
                {
                    RedSystemMessage(m_session, "More than 25 results found.");
                    break;
                }
            }
        } // for loop (number of rows, up to 25)
    } // lookup name or description
    if (lookupcriteria && numFound < 25)
    {
        std::set<uint32_t> foundList;
        j = sAchievementCriteriaStore.getNumRows();
        for (i = 0; i < j && numFound < 25; ++i)
        {
            auto criteria = sAchievementCriteriaStore.lookupEntry(i);
            if (criteria)
            {
                if (foundList.find(criteria->ID) != foundList.end())
                {
                    // already listed this achievement (some achievements have multiple entries in dbc)
                    continue;
                }
#if VERSION_STRING < Cata
                y = std::string(criteria->name[sWorld.getDbcLocaleLanguageId()]);
#else
                y = std::string(criteria->name[0]);
#endif
                AscEmu::Util::Strings::toLowerCase(y);
                if (AscEmu::Util::Strings::contains(x, y) == false)
                {
                    continue;
                }

                foundList.insert(criteria->ID);
                std::stringstream strm;
                strm << criteria->ID;
                recout = "|cffffffffCriteria ";
                recout += strm.str();
                recout += ": |cfffff000";
#if VERSION_STRING < Cata
                recout += criteria->name[sWorld.getDbcLocaleLanguageId()];
#else
                recout += criteria->name[0];
#endif
                strm.str("");
                auto achievement = sAchievementStore.lookupEntry(criteria->referredAchievement);
                if (achievement)
                {
                    // create achievement link
                    recout += " |cffffffffAchievement ";
                    strm << achievement->ID;
                    recout += strm.str();
                    recout += ": |cfffff000|Hachievement:";
                    recout += strm.str();
                    recout += ":";
                    recout += playerGUID;
                    time_t completetime = m_session->GetPlayer()->getAchievementMgr()->getCompletedTime(achievement);
                    if (completetime)
                    {
                        // achievement is completed
                        struct tm* ct;
                        ct = localtime(&completetime);
                        strm.str("");
                        strm << ":1:" << ct->tm_mon + 1 << ":" << ct->tm_mday << ":" << ct->tm_year - 100 << ":-1:-1:-1:-1|h[";
                        recout += strm.str();
                    }
                    else
                    {
                        // achievement is not completed
                        recout += ":0:0:0:-1:0:0:0:0|h[";
                    }
#if VERSION_STRING < Cata
                    recout += achievement->name[sWorld.getDbcLocaleLanguageId()];
#else
                    recout += achievement->name[0];
#endif
                    if (!lookupreward)
                    {
                        recout += "]|h|r";
                    }
                    else
                    {
                        recout += "]|h |cffffffff";
#if VERSION_STRING < Cata
                        recout += achievement->rewardName[sWorld.getDbcLocaleLanguageId()];
#else
                        recout += achievement->rewardName[0];
#endif
                        recout += "|r";
                    }
                    strm.str("");
                }
                SendMultilineMessage(m_session, recout.c_str());
                if (++numFound >= 25)
                {
                    RedSystemMessage(m_session, "More than 25 results found.");
                    break;
                }
            }
        } // for loop (number of rows, up to 25)
    } // lookup criteria
    if (numFound == 0)
    {
        recout = "|cff00ccffNo matches found.";
        SendMultilineMessage(m_session, recout.c_str());
    }
    BlueSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
#endif
    return true;
}

//.lookup creature
bool ChatHandler::HandleLookupCreatureCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    AscEmu::Util::Strings::toLowerCase(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of creature `%s`...", x.c_str());
    auto startTime = Util::TimeNow();

    uint32_t count = 0;

    MySQLDataStore::CreaturePropertiesContainer const* its = sMySQLStore.getCreaturePropertiesStore();
    for (MySQLDataStore::CreaturePropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        CreatureProperties const* it = sMySQLStore.getCreatureProperties(itr->second.Id);
        if (it != nullptr)
        {
            MySQLStructure::LocalesCreature const* lit = (m_session->language > 0) ? sMySQLStore.getLocalizedCreature(it->Id, m_session->language) : nullptr;

            std::string litName = std::string( lit ? lit->name : "");

            AscEmu::Util::Strings::toLowerCase(litName);

            bool localizedFound = false;
            if (AscEmu::Util::Strings::contains(x, litName))
                localizedFound = true;

            std::string names_lower = it->lowercase_name;
            if (AscEmu::Util::Strings::contains(x, names_lower) || localizedFound)
            {
                SystemMessage(m_session, "ID: %u |cfffff000%s", it->Id, it->Name.c_str());
                ++count;

                if (count == 25)
                {
                    RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                    break;
                }
            }
        }
    }

    if (count == 0)
        RedSystemMessage(m_session, "No results returned. aborting.");

    BlueSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.lookup faction
bool ChatHandler::HandleLookupFactionCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    AscEmu::Util::Strings::toLowerCase(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of faction `%s`...", x.c_str());
    auto startTime = Util::TimeNow();
    uint32_t count = 0;
    for (uint32_t index = 0; index < sFactionStore.getNumRows(); ++index)
    {
        WDB::Structures::FactionEntry const* faction = sFactionStore.lookupEntry(index);
        if (faction != nullptr)
        {
#if VERSION_STRING < Cata
            std::string y = std::string(faction->Name[sWorld.getDbcLocaleLanguageId()]);
#else
            std::string y = std::string(faction->Name[0]);
#endif
            AscEmu::Util::Strings::toLowerCase(y);
            if (AscEmu::Util::Strings::contains(x, y))
            {
#if VERSION_STRING < Cata
                SendHighlightedName(m_session, "Faction", faction->Name[sWorld.getDbcLocaleLanguageId()], y, x, faction->ID);
#else
                SendHighlightedName(m_session, "Faction", faction->Name[0], y, x, faction->ID);
#endif
                ++count;
                if (count == 25)
                {
                    RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                    break;
                }
            }
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.lookup item
bool ChatHandler::HandleLookupItemCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    AscEmu::Util::Strings::toLowerCase(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of item `%s`...", x.c_str());
    auto startTime = Util::TimeNow();

    uint32_t count = 0;

    MySQLDataStore::ItemPropertiesContainer const* its = sMySQLStore.getItemPropertiesStore();
    for (MySQLDataStore::ItemPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        ItemProperties const* it = sMySQLStore.getItemProperties(itr->second.ItemId);
        if (it == nullptr)
            continue;

        MySQLStructure::LocalesItem const* lit = (m_session->language > 0) ? sMySQLStore.getLocalizedItem(it->ItemId, m_session->language) : nullptr;

        std::string litName = std::string(lit ? lit->name : "");

        AscEmu::Util::Strings::toLowerCase(litName);

        bool localizedFound = false;
        if (AscEmu::Util::Strings::contains(x, litName))
            localizedFound = true;

        std::string proto_lower = it->lowercase_name;
        if (AscEmu::Util::Strings::contains(x, proto_lower) || localizedFound)
        {
            SendItemLinkToPlayer(it, m_session, false, 0, localizedFound ? m_session->language : 0);
            ++count;
            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    if (count == 0)
        RedSystemMessage(m_session, "No results returned. aborting.");

    BlueSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.lookup object
bool ChatHandler::HandleLookupObjectCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    AscEmu::Util::Strings::toLowerCase(x);

    GreenSystemMessage(m_session, "Starting search of object `%s`...", x.c_str());
    auto startTime = Util::TimeNow();
    GameObjectProperties const* gameobject_info;
    uint32_t count = 0;
    std::string y;
    std::string recout;

    MySQLDataStore::GameObjectPropertiesContainer const* its = sMySQLStore.getGameObjectPropertiesStore();
    for (MySQLDataStore::GameObjectPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        gameobject_info = sMySQLStore.getGameObjectProperties(itr->second.entry);
        y = std::string(gameobject_info->name);
        AscEmu::Util::Strings::toLowerCase(y);
        if (AscEmu::Util::Strings::contains(x, y))
        {
            std::string Name;
            std::stringstream strm;
            strm << gameobject_info->entry;
            strm << ", Display ";
            strm << gameobject_info->display_id;
            const char* objectName = gameobject_info->name.c_str();
            recout = "|cfffff000Object ";
            recout += strm.str();
            recout += "|cffFFFFFF: ";
            recout += objectName;
            recout = recout + Name;
            SendMultilineMessage(m_session, recout.c_str());

            ++count;
            if (count == 25 || count > 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    if (count == 0)
    {
        recout = "|cff00ccffNo matches found.";
        SendMultilineMessage(m_session, recout.c_str());
    }

    BlueSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.lookup quest
bool ChatHandler::HandleLookupQuestCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string search_string = std::string(args);
    AscEmu::Util::Strings::toLowerCase(search_string);
    if (search_string.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of quests `%s`...", search_string.c_str());
    auto startTime = Util::TimeNow();
    std::string recout;
    uint32_t count = 0;

    MySQLDataStore::QuestPropertiesContainer const* its = sMySQLStore.getQuestPropertiesStore();
    for (MySQLDataStore::QuestPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        QuestProperties const* quest = sMySQLStore.getQuestProperties(itr->second.id);
        if (quest == nullptr)
            continue;

        std::string lower_quest_title = quest->title;

        MySQLStructure::LocalesQuest const* li = (m_session->language > 0) ? sMySQLStore.getLocalizedQuest(quest->id, m_session->language) : nullptr;

        std::string liName = std::string(li ? li->title : "");

        AscEmu::Util::Strings::toLowerCase(liName);
        AscEmu::Util::Strings::toLowerCase(lower_quest_title);

        bool localizedFound = false;
        if (AscEmu::Util::Strings::contains(search_string, liName))
            localizedFound = true;

        if (AscEmu::Util::Strings::contains(search_string, lower_quest_title) || localizedFound)
        {
            std::string questid = MyConvertIntToString(quest->id);
            std::string questtitle = localizedFound ? (li ? li->title : "") : quest->title;
            // send quest link
            recout = questid;
            recout += ": |cff00ccff|Hquest:";
            recout += questid;
            recout += ":";
            recout += MyConvertIntToString(quest->min_level);
            recout += "|h[";
            recout += questtitle;
            recout += "]|h|r";
            SendMultilineMessage(m_session, recout.c_str());

            ++count;
            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    if (count == 0)
    {
        recout = "|cff00ccffNo matches found.\n\n";
        SendMultilineMessage(m_session, recout.c_str());
    }

    BlueSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));

    return true;
}

//.lookup spell
bool ChatHandler::HandleLookupSpellCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    AscEmu::Util::Strings::toLowerCase(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of spell `%s`...", x.c_str());
    auto startTime = Util::TimeNow();
    uint32_t count = 0;
    std::string recout;
    char itoabuf[12];
    for (auto it = sSpellMgr.getSpellInfoMap()->begin(); it != sSpellMgr.getSpellInfoMap()->end(); ++it)
    {
        SpellInfo const* spell = sSpellMgr.getSpellInfo(it->first);
        std::string y = std::string(spell->getName());
        AscEmu::Util::Strings::toLowerCase(y);
        if (AscEmu::Util::Strings::contains(x, y))
        {
            sprintf((char*)itoabuf, "%u", spell->getId());
            recout = itoabuf;
            recout += ": |cff71d5ff|Hspell:";
            recout += itoabuf;
            recout += "|h[";
            recout += spell->getName();
            recout += "]|h|r";

            std::string::size_type pos = recout.find('%');
            if (pos != std::string::npos)
            {
                recout.insert(pos + 1, "%");
            }

            SendMultilineMessage(m_session, recout.c_str());

            ++count;
            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}

//.lookup skill
bool ChatHandler::HandleLookupSkillCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    AscEmu::Util::Strings::toLowerCase(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of skill `%s`...", x.c_str());
    auto startTime = Util::TimeNow();
    uint32_t count = 0;
    for (uint32_t index = 0; index < sSkillLineStore.getNumRows(); ++index)
    {
        auto skill_line = sSkillLineStore.lookupEntry(index);
        if (skill_line == nullptr)
            continue;

#if VERSION_STRING < Cata
        std::string y = std::string(skill_line->Name[sWorld.getDbcLocaleLanguageId()]);
#else
        std::string y = std::string(skill_line->Name[0]);
#endif
        AscEmu::Util::Strings::toLowerCase(y);
        if (AscEmu::Util::Strings::contains(x, y))
        {
#if VERSION_STRING < Cata
            SendHighlightedName(m_session, "Skill", skill_line->Name[sWorld.getDbcLocaleLanguageId()], y, x, skill_line->id);
#else
            SendHighlightedName(m_session, "Skill", skill_line->Name[0], y, x, skill_line->id);
#endif
            ++count;
            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", static_cast<uint32_t>(Util::GetTimeDifferenceToNow(startTime)));
    return true;
}
