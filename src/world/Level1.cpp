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


uint16 GetItemIDFromLink(const char* itemlink, uint32* itemid)
{
    if (itemlink == NULL)
    {
        *itemid = 0;
        return 0;
    }
    uint16 slen = (uint16)strlen(itemlink);
    const char* ptr = strstr(itemlink, "|Hitem:");
    if (ptr == NULL)
    {
        *itemid = 0;
        return slen;
    }
    ptr += 7;                       // item id is just past "|Hitem:" (7 bytes)
    *itemid = atoi(ptr);
    ptr = strstr(itemlink, "|r");   // the end of the item link
    if (ptr == NULL)                // item link was invalid
    {
        *itemid = 0;
        return slen;
    }
    ptr += 2;
    return (ptr - itemlink) & 0x0000ffff;
}

bool ChatHandler::HandleAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args || strlen(args) < 4 || strchr(args, '%'))
    {
        m_session->SystemMessage("Announces cannot contain the %% character and must be at least 4 characters.");
        return true;
    }
    char msg[1024];
    std::string input2;
    input2 = sWorld.ann_tagcolor;
    input2 += "[";
    input2 += sWorld.announce_tag;
    input2 += "]";
    input2 += sWorld.ann_gmtagcolor;
    if (sWorld.GMAdminTag)
    {
        if (m_session->CanUseCommand('z')) input2 += "<Admin>";
        else if (m_session->GetPermissionCount()) input2 += "<GM>";
    }
    if (sWorld.NameinAnnounce)
    {
        input2 += "|r" + sWorld.ann_namecolor + "|Hplayer:";
        input2 += m_session->GetPlayer()->GetName();
        input2 += "|h[";
        input2 += m_session->GetPlayer()->GetName();
        input2 += "]|h:|r " + sWorld.ann_msgcolor;
    }
    else if (!sWorld.NameinAnnounce) { input2 += ": "; input2 += sWorld.ann_msgcolor; }
    snprintf((char*)msg, 1024, "%s%s", input2.c_str(), args);
    sWorld.SendWorldText(msg);
    sGMLog.writefromsession(m_session, "used announce command, [%s]", args);
    return true;
}

bool ChatHandler::HandleWAnnounceCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    char pAnnounce[1024];
    std::string input3;
    input3 = sWorld.ann_tagcolor;
    input3 += "[";
    input3 += sWorld.announce_tag;
    input3 += "]";
    input3 += sWorld.ann_gmtagcolor;
    if (sWorld.GMAdminTag)
    {
        if (m_session->CanUseCommand('z')) input3 += "<Admin>";
        else if (m_session->GetPermissionCount()) input3 += "<GM>";
    }
    if (sWorld.NameinWAnnounce)
    {
        input3 += "|r" + sWorld.ann_namecolor + "[";
        input3 += m_session->GetPlayer()->GetName();
        input3 += "]:|r " + sWorld.ann_msgcolor;
    }
    else if (!sWorld.NameinWAnnounce) { input3 += ": "; input3 += sWorld.ann_msgcolor; }
    snprintf((char*)pAnnounce, 1024, "%s%s", input3.c_str(), args);
    sWorld.SendWorldWideScreenText(pAnnounce);
    sGMLog.writefromsession(m_session, "used wannounce command [%s]", args);
    return true;
}

bool ChatHandler::HandleGPSCommand(const char* args, WorldSession* m_session)
{
    Object* obj;
    uint64 guid = m_session->GetPlayer()->GetSelection();
    if (guid != 0)
    {
        if ((obj = m_session->GetPlayer()->GetMapMgr()->GetUnit(guid)) == 0)
        {
            SystemMessage(m_session, "You should select a character or a creature.");
            return true;
        }
    }
    else
        obj = m_session->GetPlayer();

    char buf[400];
    auto at = obj->GetArea();
    if (!at)
    {
        snprintf((char*)buf, 400, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%d |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f|r",
                 (unsigned int)obj->GetMapId(), obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation());
        SystemMessage(m_session, buf);
        return true;
    }
    auto out_map_id = obj->GetMapId();
    auto out_zone_id = at->zone; // uint32 at_old->ZoneId
    auto out_area_id = at->id; // uint32 at_old->AreaId
    auto out_phase = obj->GetPhase();
    auto out_x = obj->GetPositionX();
    auto out_y = obj->GetPositionY();
    auto out_z = obj->GetPositionZ();
    auto out_o = obj->GetOrientation();
    auto out_area_name = at->area_name[0]; // enUS, hardcoded until locale is implemented properly
    snprintf((char*)buf, 400, "|cff00ff00Current Position: |cffffffffMap: |cff00ff00%d |cffffffffZone: |cff00ff00%u |cffffffffArea: |cff00ff00%u |cffffffffPhase: |cff00ff00%u |cffffffffX: |cff00ff00%f |cffffffffY: |cff00ff00%f |cffffffffZ: |cff00ff00%f |cffffffffOrientation: |cff00ff00%f |cffffffffArea Name: |cff00ff00%s |r",
             out_map_id, out_zone_id, out_area_id, out_phase, out_x, out_y, out_z, out_o, out_area_name);
    SystemMessage(m_session, buf);
    if (obj->obj_movement_info.IsOnTransport())
    {
        SystemMessage(m_session, "Position on Transport:");
        SystemMessage(m_session, "  tX: %f  tY: %f  tZ: %f  tO: %f", obj->GetTransPositionX(), obj->GetTransPositionY(), obj->GetTransPositionZ(), obj->GetTransPositionO());
    }

    // ".gps 1" will save gps info to file logs/gps.log - This probably isn't very multithread safe so don't have many gms spamming it!
    if (args != NULL && *args == '1')
    {
        FILE* gpslog = fopen(FormatOutputString("logs", "gps", false).c_str(), "at");
        if (gpslog)
        {
            fprintf(gpslog, "%d, %u, %u, %f, %f, %f, %f, \'%s\'", out_map_id, out_zone_id, out_area_id, out_x, out_y, out_z, out_o, out_area_name);
            // ".gps 1 comment" will save comment after the gps data
            if (*(args + 1) == ' ')
                fprintf(gpslog, ",%s\n", args + 2);
            else
                fprintf(gpslog, "\n");
            fclose(gpslog);
        }
    }
    return true;
}

bool ChatHandler::HandleSummonCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    // Summon Blocking
    if (!stricmp(args, "on"))
    {
        if (m_session->GetPlayer()->IsSummonDisabled())
        {
            BlueSystemMessage(m_session, "Summon blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->DisableSummon(true);
            GreenSystemMessage(m_session, "Summon blocking is now enabled");
        }
        return true;
    }
    else if (!stricmp(args, "off"))
    {
        if (m_session->GetPlayer()->IsSummonDisabled())
        {
            m_session->GetPlayer()->DisableSummon(false);
            GreenSystemMessage(m_session, "Summon blocking is now disabled");
        }
        else
        {
            BlueSystemMessage(m_session, "Summon blocking is already disabled");
        }
        return true;
    }

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {
        // send message to user
        char buf[256];
        char buf0[256];
        if (!m_session->CanUseCommand('z') && chr->IsSummonDisabled())
        {
            snprintf((char*)buf, 256, "%s has blocked other GMs from summoning them.", chr->GetName());
            SystemMessage(m_session, buf);
            return true;
        }
        if (chr->GetMapMgr() == NULL)
        {
            snprintf((char*)buf, 256, "%s is already being teleported.", chr->GetName());
            SystemMessage(m_session, buf);
            return true;
        }
        snprintf((char*)buf, 256, "You are summoning %s.", chr->GetName());
        SystemMessage(m_session, buf);
        // Don't summon the dead, lol, I see dead people. :P
        // If you do, we better bring them back to life
        if (chr->getDeathState() == 1) // Just died
            chr->RemoteRevive();
        if (chr->getDeathState() != 0) // Not alive
            chr->ResurrectPlayer();
        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            SystemMessage(chr->GetSession(), "You are being summoned by %s.", m_session->GetPlayer()->GetName());
        }
        Player* plr = m_session->GetPlayer();
        if (plr->GetMapMgr() == chr->GetMapMgr())
            chr->_Relocate(plr->GetMapId(), plr->GetPosition(), false, false, plr->GetInstanceID());
        else
        {
            sEventMgr.AddEvent(chr, &Player::EventPortToGM, plr, 0, 1, 1, EVENT_FLAG_DO_NOT_EXECUTE_IN_WORLD_CONTEXT);
        }
    }
    else
    {
        PlayerInfo* pinfo = objmgr.GetPlayerInfoByName(args);
        if (!pinfo)
        {
            char buf[256];
            snprintf((char*)buf, 256, "Player (%s) does not exist.", args);
            SystemMessage(m_session, buf);
            return true;
        }
        else
        {
            Player* pPlayer = m_session->GetPlayer();
            char query[512];
            snprintf((char*)&query, 512, "UPDATE characters SET mapId = %u, positionX = %f, positionY = %f, positionZ = %f, zoneId = %u WHERE guid = %u;", pPlayer->GetMapId(), pPlayer->GetPositionX(), pPlayer->GetPositionY(), pPlayer->GetPositionZ(), pPlayer->GetZoneId(), pinfo->guid);
            CharacterDatabase.Execute(query);
            char buf[256];
            snprintf((char*)buf, 256, "(Offline) %s has been summoned.", pinfo->name);
            SystemMessage(m_session, buf);
            return true;
        }
    }
    sGMLog.writefromsession(m_session, "summoned %s on map %u, %f %f %f", args, m_session->GetPlayer()->GetMapId(), m_session->GetPlayer()->GetPositionX(), m_session->GetPlayer()->GetPositionY(), m_session->GetPlayer()->GetPositionZ());
    return true;
}

bool ChatHandler::HandleAppearCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    // Appear Blocking
    if (!stricmp(args, "on"))
    {
        if (m_session->GetPlayer()->IsAppearDisabled())
        {
            BlueSystemMessage(m_session, "Appear blocking is already enabled");
        }
        else
        {
            m_session->GetPlayer()->DisableAppear(true);
            GreenSystemMessage(m_session, "Appear blocking is now enabled");
        }
        return true;
    }
    else if (!stricmp(args, "off"))
    {
        if (m_session->GetPlayer()->IsAppearDisabled())
        {
            m_session->GetPlayer()->DisableAppear(false);
            GreenSystemMessage(m_session, "Appear blocking is now disabled");
        }
        else
        {
            BlueSystemMessage(m_session, "Appear blocking is already disabled");
        }
        return true;
    }

    Player* chr = objmgr.GetPlayer(args, false);
    if (chr)
    {
        if (!m_session->CanUseCommand('z') && chr->IsAppearDisabled())
        {
            SystemMessage(m_session, "%s has blocked other GMs from appearing to them.", chr->GetName());
            return true;
        }
        if (chr->GetMapMgr() == NULL)
        {
            SystemMessage(m_session, "%s is already being teleported.", chr->GetName());
            return true;
        }
        SystemMessage(m_session, "Appearing at %s's location.", chr->GetName());
        if (!m_session->GetPlayer()->m_isGmInvisible)
        {
            SystemMessage(chr->GetSession(), "%s is appearing to your location.", m_session->GetPlayer()->GetName());
        }
        //m_session->GetPlayer()->SafeTeleport(chr->GetMapId(), chr->GetInstanceID(), chr->GetPosition());
        //If the GM is on the same map as the player, use the normal safeteleport method
        if (m_session->GetPlayer()->GetMapId() == chr->GetMapId() && m_session->GetPlayer()->GetInstanceID() == chr->GetInstanceID())
            m_session->GetPlayer()->SafeTeleport(chr->GetMapId(), chr->GetInstanceID(), chr->GetPosition());
        else
            m_session->GetPlayer()->SafeTeleport(chr->GetMapMgr(), chr->GetPosition());
        //The player and GM are not on the same map. We use this method so we can port to BG's (Above method doesn't support them)
    }
    else
    {
        SystemMessage(m_session, "Player (%s) does not exist or is not logged in.", args);
    }
    return true;
}

bool ChatHandler::HandleLearnSkillCommand(const char* args, WorldSession* m_session)
{
    uint32 skill, min, max;
    min = max = 1;
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;
    else
        skill = atol(pSkill);
    BlueSystemMessage(m_session, "Adding skill line %d", skill);
    char* pMin = strtok(NULL, " ");
    if (pMin)
    {
        min = atol(pMin);
        char* pMax = strtok(NULL, "\n");
        if (pMax)
            max = atol(pMax);
    }
    else
    {
        return false;
    }
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr) return false;
    if (!plr->IsPlayer()) return false;
    sGMLog.writefromsession(m_session, "used add skill of %u %u %u on %s", skill, min, max, plr->GetName());
    plr->_AddSkillLine(skill, min, max);
    return true;
}

bool ChatHandler::HandleModifySkillCommand(const char* args, WorldSession* m_session)
{
    uint32 skill, min, max;
    min = max = 1;
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;
    else
        skill = atol(pSkill);
    char* pMin = strtok(NULL, " ");
    uint32 cnt = 0;
    if (!pMin)
        cnt = 1;
    else
        cnt = atol(pMin);
    skill = atol(pSkill);
    BlueSystemMessage(m_session, "Modifying skill line %d. Advancing %d times.", skill, cnt);
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr) plr = m_session->GetPlayer();
    if (!plr) return false;
    sGMLog.writefromsession(m_session, "used modify skill of %u %u on %s", skill, cnt, plr->GetName());
    if (!plr->_HasSkillLine(skill))
    {
        SystemMessage(m_session, "Does not have skill line, adding.");
        plr->_AddSkillLine(skill, 1, 300);
    }
    else
    {
        plr->_AdvanceSkillLine(skill, cnt);
    }
    return true;
}

/// DGM: Get skill level command for getting information about a skill
bool ChatHandler::HandleGetSkillLevelCommand(const char* args, WorldSession* m_session)
{
    uint32 skill = 0;
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;
    else
        skill = atol(pSkill);
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr) return false;
    if (skill > SkillNameManager->maxskill)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }
    char* SkillName = SkillNameManager->SkillNames[skill];
    if (SkillName == 0)
    {
        BlueSystemMessage(m_session, "Skill: %u does not exists", skill);
        return false;
    }
    if (!plr->_HasSkillLine(skill))
    {
        BlueSystemMessage(m_session, "Player does not have %s skill.", SkillName);
        return false;
    }
    uint32 nobonus = plr->_GetSkillLineCurrent(skill, false);
    uint32 bonus = plr->_GetSkillLineCurrent(skill, true) - nobonus;
    uint32 max = plr->_GetSkillLineMax(skill);
    BlueSystemMessage(m_session, "Player's %s skill has level: %u maxlevel: %u. (+ %u bonus)", SkillName, nobonus, max, bonus);
    return true;
}

bool ChatHandler::HandleRemoveSkillCommand(const char* args, WorldSession* m_session)
{
    uint32 skill = 0;
    char* pSkill = strtok((char*)args, " ");
    if (!pSkill)
        return false;
    else
        skill = atol(pSkill);
    BlueSystemMessage(m_session, "Removing skill line %d", skill);
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr && plr->_HasSkillLine(skill)) //fix bug; removing skill twice will mess up skills
    {
        plr->_RemoveSkillLine(skill);
        sGMLog.writefromsession(m_session, "used remove skill of %u on %s", skill, plr->GetName());
        SystemMessage(plr->GetSession(), "%s removed skill line %d from you. ", m_session->GetPlayer()->GetName(), skill);
    }
    else
    {
        BlueSystemMessage(m_session, "Player doesn't have skill line %d", skill);
    }
    return true;
}

bool ChatHandler::HandleUnlearnCommand(const char* args, WorldSession* m_session)
{
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (plr == 0)
        return true;
    uint32 SpellId = atol(args);
    if (SpellId == 0)
    {
        SpellId = GetSpellIDFromLink(args);
        if (SpellId == 0)
        {
            RedSystemMessage(m_session, "You must specify a spell id.");
            return true;
        }
    }
    sGMLog.writefromsession(m_session, "removed spell %u from %s", SpellId, plr->GetName());
    if (plr->HasSpell(SpellId))
    {
        GreenSystemMessage(plr->GetSession(), "Removed spell %u.", SpellId);
        GreenSystemMessage(m_session, "Removed spell %u from %s.", SpellId, plr->GetName());
        plr->removeSpell(SpellId, false, false, 0);
    }
    else
    {
        RedSystemMessage(m_session, "That player does not have spell %u learnt.", SpellId);
    }
    return true;
}

#ifdef ENABLE_ACHIEVEMENTS
/**
Handles .achieve complete
.achieve complete id : completes achievement "id" (can be an achievement link) for the selected player
*/
bool ChatHandler::HandleAchievementCompleteCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    uint32 achievement_id = atol(args);
    if (achievement_id == 0)
    {
        achievement_id = GetAchievementIDFromLink(args);
        if (achievement_id == 0)
        {
            if (stricmp(args, "all") == 0)
            {
                plr->GetAchievementMgr().GMCompleteAchievement(m_session, -1);
                SystemMessage(m_session, "All achievements have now been completed for that player.");
                sGMLog.writefromsession(m_session, "completed all achievements for player %s", plr->GetName());
                return true;
            }
            return false;
        }
    }
    if (plr->GetAchievementMgr().GMCompleteAchievement(m_session, achievement_id))
    {
        SystemMessage(m_session, "The achievement has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement %u for player %s", achievement_id, plr->GetName());
    }
    return true;
}

/**
Handles .achieve criteria
.achieve criteria id : completes achievement criteria "id" for the selected player
*/
bool ChatHandler::HandleAchievementCriteriaCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    uint32 criteria_id = atol(args);
    if (criteria_id == 0)
    {
        if (stricmp(args, "all") == 0)
        {
            plr->GetAchievementMgr().GMCompleteCriteria(m_session, -1);
            SystemMessage(m_session, "All achievement criteria have now been completed for that player.");
            sGMLog.writefromsession(m_session, "completed all achievement criteria for player %s", plr->GetName());
            return true;
        }
        return false;
    }
    if (plr->GetAchievementMgr().GMCompleteCriteria(m_session, criteria_id))
    {
        SystemMessage(m_session, "The achievement criteria has now been completed for that player.");
        sGMLog.writefromsession(m_session, "completed achievement criteria %u for player %s", criteria_id, plr->GetName());
    }
    return true;
}

/**
Handles .achieve reset
.achieve reset id : removes achievement "id" (can be an achievement link) from the selected player
.achieve reset criteria id : removes achievement criteria "id" from the selected player
.achieve reset all : removes all achievement and criteria data from the selected player
*/
bool ChatHandler::HandleAchievementResetCommand(const char* args, WorldSession* m_session)
{
    if (!*args)
        return false;
    Player* plr = GetSelectedPlayer(m_session, true, true);
    if (!plr)
    {
        plr = m_session->GetPlayer();
        SystemMessage(m_session, "Auto-targeting self.");
    }
    bool resetAch = true, resetCri = false;
    int32 achievement_id;
    if (strnicmp(args, "criteria ", 9) == 0)
    {
        achievement_id = atol(args + 9);
        if (achievement_id == 0)
        {
            if (stricmp(args + 9, "all") != 0)
            {
                return false;
            }
            achievement_id = -1;
        }
        resetCri = true;
        resetAch = false;
    }
    else if (stricmp(args, "all") == 0)
    {
        achievement_id = -1;
        resetCri = true;
    }
    else
    {
        achievement_id = atol(args);
        if (achievement_id == 0)
        {
            achievement_id = GetAchievementIDFromLink(args);
            if (achievement_id == 0)
                return false;
        }
    }
    if (resetAch)
        plr->GetAchievementMgr().GMResetAchievement(achievement_id);
    if (resetCri)
        plr->GetAchievementMgr().GMResetCriteria(achievement_id);
    return true;
}

/**
Handles .lookup achievement
GM achievement lookup command usage:
.lookup achievement string : searches for "string" in achievement name
.lookup achievement desc string : searches for "string" in achievement description
.lookup achievement reward string : searches for "string" in achievement reward name
.lookup achievement criteria string : searches for "string" in achievement criteria name
.lookup achievement all string : searches for "string" in achievement name, description, reward, and critiera
*/
bool ChatHandler::HandleLookupAchievementCmd(const char* args, WorldSession* m_session)
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

bool ChatHandler::HandleVehicleEjectPassengerCommand(const char *args, WorldSession* session)
{
    uint32 seat = 0;
    std::stringstream ss(args);
    ss >> seat;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify a seat number.");
        return false;
    }
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->EjectPassengerFromSeat(seat);
    return true;
}

bool ChatHandler::HandleVehicleEjectAllPassengersCommand(const char *args, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->EjectAllPassengers();
    return true;
}

bool ChatHandler::HandleVehicleInstallAccessoriesCommand(const char *args, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->InstallAccessories();
    return true;
}

bool ChatHandler::HandleVehicleRemoveAccessoriesCommand(const char *args, WorldSession* session)
{
    Player* p = session->GetPlayer();
    if (p->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = p->GetMapMgr()->GetUnit(p->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    u->GetVehicleComponent()->RemoveAccessories();
    return true;
}

bool ChatHandler::HandleVehicleAddPassengerCommand(const char *args, WorldSession* session)
{
    std::stringstream ss(args);
    uint32 creature_entry;
    ss >> creature_entry;
    if (ss.fail())
    {
        RedSystemMessage(session, "You need to specify a creature id.");
        return false;
    }
    if (session->GetPlayer()->GetTargetGUID() == 0)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    Unit* u = session->GetPlayer()->GetMapMgr()->GetUnit(session->GetPlayer()->GetTargetGUID());
    if (u == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (u->GetVehicleComponent() == NULL)
    {
        RedSystemMessage(session, "You need to select a vehicle.");
        return false;
    }
    if (!u->GetVehicleComponent()->HasEmptySeat())
    {
        RedSystemMessage(session, "That vehicle has no more empty seats.");
        return false;
    }
    CreatureInfo *ci = CreatureNameStorage.LookupEntry(creature_entry);
    CreatureProto *cp = CreatureProtoStorage.LookupEntry(creature_entry);
    if ((ci == NULL) || (cp == NULL))
    {
        RedSystemMessage(session, "Creature %u doesn't exist in the database", creature_entry);
        return false;
    }
    Creature* c = u->GetMapMgr()->CreateCreature(creature_entry);
    c->Load(cp, u->GetPositionX(), u->GetPositionY(), u->GetPositionZ(), u->GetOrientation());
    c->PushToWorld(u->GetMapMgr());
    c->EnterVehicle(u->GetGUID(), 1);
    return true;
}
