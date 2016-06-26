/*
Copyright (c) 2014-2016 AscEmu Team <http://www.ascemu.org/>
This file is released under the MIT license. See README-MIT for more information.
*/

#include "StdAfx.h"

//.lookup achievement
#ifdef ENABLE_ACHIEVEMENTS
//////////////////////////////////////////////////////////////////////////////////////////
//Handles .lookup achievement
//\brief GM achievement lookup command usage:
//.lookup achievement string : searches for "string" in achievement name
//.lookup achievement desc string : searches for "string" in achievement description
//.lookup achievement reward string : searches for "string" in achievement reward name
//.lookup achievement criteria string : searches for "string" in achievement criteria name
//.lookup achievement all string : searches for "string" in achievement name, description, reward, and critiera
//////////////////////////////////////////////////////////////////////////////////////////
bool ChatHandler::HandleLookupAchievementCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x;
    bool lookupname = true, lookupdesc = false, lookupcriteria = false, lookupreward = false;
    if (strnicmp(args, "name ", 5) == 0)
    {
        x = std::string(args + 5);
    }
    else if (strnicmp(args, "desc ", 5) == 0)
    {
        lookupname = false;
        lookupdesc = true;
        x = std::string(args + 5);
    }
    else if (strnicmp(args, "criteria ", 9) == 0)
    {
        lookupname = false;
        lookupcriteria = true;
        x = std::string(args + 9);
    }
    else if (strnicmp(args, "reward ", 7) == 0)
    {
        lookupname = false;
        lookupreward = true;
        x = std::string(args + 7);
    }
    else if (strnicmp(args, "all ", 4) == 0)
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

    arcemu_TOLOWER(x);
    GreenSystemMessage(m_session, "Starting search of achievement `%s`...", x.c_str());
    uint32 t = getMSTime();
    uint32 i, j, numFound = 0;
    std::string y, recout;
    char playerGUID[17];
    snprintf(playerGUID, 17, "%llu", m_session->GetPlayer()->GetGUID());
    if (lookupname || lookupdesc || lookupreward)
    {
        std::set<uint32> foundList;
        j = sAchievementStore.GetNumRows();
        bool foundmatch;
        for (i = 0; i < j && numFound < 25; ++i)
        {
            auto achievement = sAchievementStore.LookupEntry(i);
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
                    y = std::string(achievement->name[0]);
                    arcemu_TOLOWER(y);
                    foundmatch = FindXinYString(x, y);
                }
                if (!foundmatch && lookupdesc)
                {
                    y = std::string(achievement->description[0]);
                    arcemu_TOLOWER(y);
                    foundmatch = FindXinYString(x, y);
                }
                if (!foundmatch && lookupreward)
                {
                    y = std::string(achievement->rewardName[0]);
                    arcemu_TOLOWER(y);
                    foundmatch = FindXinYString(x, y);
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
                recout += (const char*)playerGUID;
                time_t completetime = m_session->GetPlayer()->GetAchievementMgr().GetCompletedTime(achievement);
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
                recout += achievement->name[0];
                if (!lookupreward)
                {
                    recout += "]|h|r";
                }
                else
                {
                    recout += "]|h |cffffffff";
                    recout += achievement->rewardName[0];
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
        std::set<uint32> foundList;
        j = sAchievementCriteriaStore.GetNumRows();
        for (i = 0; i < j && numFound < 25; ++i)
        {
            auto criteria = sAchievementCriteriaStore.LookupEntry(i);
            if (criteria)
            {
                if (foundList.find(criteria->ID) != foundList.end())
                {
                    // already listed this achievement (some achievements have multiple entries in dbc)
                    continue;
                }
                y = std::string(criteria->name[0]);
                arcemu_TOLOWER(y);
                if (!FindXinYString(x, y))
                {
                    continue;
                }
                foundList.insert(criteria->ID);
                std::stringstream strm;
                strm << criteria->ID;
                recout = "|cffffffffCriteria ";
                recout += strm.str();
                recout += ": |cfffff000";
                recout += criteria->name[0];
                strm.str("");
                auto achievement = sAchievementStore.LookupEntry(criteria->referredAchievement);
                if (achievement)
                {
                    // create achievement link
                    recout += " |cffffffffAchievement ";
                    strm << achievement->ID;
                    recout += strm.str();
                    recout += ": |cfffff000|Hachievement:";
                    recout += strm.str();
                    recout += ":";
                    recout += (const char*)playerGUID;
                    time_t completetime = m_session->GetPlayer()->GetAchievementMgr().GetCompletedTime(achievement);
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
                    recout += achievement->name[0];
                    if (!lookupreward)
                    {
                        recout += "]|h|r";
                    }
                    else
                    {
                        recout += "]|h |cffffffff";
                        recout += achievement->rewardName[0];
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
    BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}
#endif

//.lookup creature
bool ChatHandler::HandleLookupCreatureCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    arcemu_TOLOWER(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of creature `%s`...", x.c_str());
    uint32 t = getMSTime();

    uint32 count = 0;

    MySQLDataStore::CreaturePropertiesContainer const* its = sMySQLStore.GetCreaturePropertiesStore();
    for (MySQLDataStore::CreaturePropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        CreatureProperties const* it = sMySQLStore.GetCreatureProperties(itr->second.Id);
        if (it == nullptr)
            continue;

        LocalizedCreatureName* lit = (m_session->language > 0) ? sLocalizationMgr.GetLocalizedCreatureName(it->Id, m_session->language) : NULL;

        std::string litName = std::string(lit ? lit->Name : "");

        arcemu_TOLOWER(litName);

        bool localizedFound = false;
        if (FindXinYString(x, litName))
            localizedFound = true;

        std::string names_lower = it->lowercase_name;
        if (FindXinYString(x, names_lower) || localizedFound)
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

    if (count == 0)
        RedSystemMessage(m_session, "No results returned. aborting.");

    BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

//.lookup faction
bool ChatHandler::HandleLookupFactionCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    arcemu_TOLOWER(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of faction `%s`...", x.c_str());
    uint32 t = getMSTime();
    uint32 count = 0;
    for (uint32 index = 0; index < sFactionStore.GetNumRows(); ++index)
    {
        DBC::Structures::FactionEntry const* faction = sFactionStore.LookupEntry(index);
        std::string y = std::string(faction->Name[0]);
        arcemu_TOLOWER(y);
        if (FindXinYString(x, y))
        {
            SendHighlightedName(m_session, "Faction", faction->Name[0], y, x, faction->ID);
            ++count;
            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

//.lookup item
bool ChatHandler::HandleLookupItemCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    arcemu_TOLOWER(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of item `%s`...", x.c_str());
    uint32 t = getMSTime();

    uint32 count = 0;

    MySQLDataStore::ItemPropertiesContainer const* its = sMySQLStore.GetItemPropertiesStore();
    for (MySQLDataStore::ItemPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        ItemProperties const* it = sMySQLStore.GetItemProperties(itr->second.ItemId);
        if (it == nullptr)
            continue;

        LocalizedItem* lit = (m_session->language > 0) ? sLocalizationMgr.GetLocalizedItem(it->ItemId, m_session->language) : NULL;

        std::string litName = std::string(lit ? lit->Name : "");

        arcemu_TOLOWER(litName);

        bool localizedFound = false;
        if (FindXinYString(x, litName))
            localizedFound = true;

        std::string proto_lower = it->lowercase_name;
        if (FindXinYString(x, proto_lower) || localizedFound)
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

    BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

//.lookup object
bool ChatHandler::HandleLookupObjectCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    arcemu_TOLOWER(x);

    GreenSystemMessage(m_session, "Starting search of object `%s`...", x.c_str());
    uint32 t = getMSTime();
    GameObjectProperties const* gameobject_info;
    uint32 count = 0;
    std::string y;
    std::string recout;

    MySQLDataStore::GameObjectPropertiesContainer const* its = sMySQLStore.GetGameObjectPropertiesStore();
    for (MySQLDataStore::GameObjectPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        gameobject_info = sMySQLStore.GetGameObjectProperties(itr->second.entry);
        y = std::string(gameobject_info->name);
        arcemu_TOLOWER(y);
        if (FindXinYString(x, y))
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

    BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

//.lookup quest
bool ChatHandler::HandleLookupQuestCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string search_string = std::string(args);
    arcemu_TOLOWER(search_string);
    if (search_string.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    BlueSystemMessage(m_session, "Starting search of quests `%s`...", search_string.c_str());
    uint32 t = getMSTime();
    std::string recout;
    uint32 count = 0;

    MySQLDataStore::QuestPropertiesContainer const* its = sMySQLStore.GetQuestPropertiesStore();
    for (MySQLDataStore::QuestPropertiesContainer::const_iterator itr = its->begin(); itr != its->end(); ++itr)
    {
        QuestProperties const* quest = sMySQLStore.GetQuestProperties(itr->second.id);
        if (quest == nullptr)
            continue;

        std::string lower_quest_title = quest->title;

        LocalizedQuest* li = (m_session->language > 0) ? sLocalizationMgr.GetLocalizedQuest(quest->id, m_session->language) : NULL;

        std::string liName = std::string(li ? li->Title : "");

        arcemu_TOLOWER(liName);
        arcemu_TOLOWER(lower_quest_title);

        bool localizedFound = false;
        if (FindXinYString(search_string, liName))
            localizedFound = true;

        if (FindXinYString(search_string, lower_quest_title) || localizedFound)
        {
            std::string questid = MyConvertIntToString(quest->id);
            std::string questtitle = localizedFound ? (li ? li->Title : "") : quest->title;
            // send quest link
            recout = questid.c_str();
            recout += ": |cff00ccff|Hquest:";
            recout += questid.c_str();
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

    BlueSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);

    return true;
}

//.lookup spell
bool ChatHandler::HandleLookupSpellCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    arcemu_TOLOWER(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of spell `%s`...", x.c_str());
    uint32 t = getMSTime();
    uint32 count = 0;
    std::string recout;
    char itoabuf[12];
    for (uint32 index = 0; index < dbcSpell.GetNumRows(); ++index)
    {
        SpellEntry* spell = dbcSpell.LookupRow(index);
        std::string y = std::string(spell->Name);
        arcemu_TOLOWER(y);
        if (FindXinYString(x, y))
        {
            sprintf((char*)itoabuf, "%u", spell->Id);
            recout = (const char*)itoabuf;
            recout += ": |cff71d5ff|Hspell:";
            recout += (const char*)itoabuf;
            recout += "|h[";
            recout += spell->Name;
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

    GreenSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}

//.lookup skill
bool ChatHandler::HandleLookupSkillCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;

    std::string x = std::string(args);
    arcemu_TOLOWER(x);
    if (x.length() < 4)
    {
        RedSystemMessage(m_session, "Your search string must be at least 4 characters long.");
        return true;
    }

    GreenSystemMessage(m_session, "Starting search of skill `%s`...", x.c_str());
    uint32 t = getMSTime();
    uint32 count = 0;
    for (uint32 index = 0; index < sSkillLineStore.GetNumRows(); ++index)
    {
        auto skill_line = sSkillLineStore.LookupEntry(index);
        if (skill_line == nullptr)
            continue;

        std::string y = std::string(skill_line->Name[0]);
        arcemu_TOLOWER(y);
        if (FindXinYString(x, y))
        {
            SendHighlightedName(m_session, "Skill", skill_line->Name[0], y, x, skill_line->id);
            ++count;
            if (count == 25)
            {
                RedSystemMessage(m_session, "More than 25 results returned. aborting.");
                break;
            }
        }
    }

    GreenSystemMessage(m_session, "Search completed in %u ms.", getMSTime() - t);
    return true;
}
